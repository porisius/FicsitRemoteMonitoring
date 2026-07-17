#!/usr/bin/env node
/**
 * ws-stress-harness.mjs
 *
 * Throwaway D-01 verification instrument for Phase 2 (thread-safe-websocket-request-handling).
 * Not shipped plugin code, not wired into build/package.sh — run manually per build/RUNBOOK.md.
 *
 * Drives N concurrent virtual WebSocket clients through a tight
 * connect -> subscribe -> unsubscribe -> disconnect churn loop against FRM's
 * live uWS endpoint, then performs an HTTP liveness probe. A server crash or
 * hang under the churn turns into a non-zero process exit code, so this can
 * be used as an automated pass/fail signal (see build/RUNBOOK.md "WebSocket
 * stress harness" section for the full pass criterion, including the
 * subscriber/client-count baseline check read from server logs).
 *
 * Zero new dependencies: uses Node's built-in global `WebSocket` (available
 * since Node 21+, confirmed present on this machine's Node v24.15.0 via
 * `node -e "console.log(typeof WebSocket)"`) and the built-in global `fetch`.
 * Do NOT `npm install ws` — the npm `ws` package was audited and REJECTED
 * for this harness (see .planning/phases/02-thread-safe-websocket-request-handling/02-RESEARCH.md,
 * "Package Legitimacy Audit").
 *
 * Usage:
 *   node build/tools/ws-stress-harness.mjs --help
 *   node build/tools/ws-stress-harness.mjs --clients 25 --duration 60
 *   node build/tools/ws-stress-harness.mjs --host 127.0.0.1 --port 8091 --clients 50 --duration 300
 */

'use strict';

const DEFAULTS = {
  host: '127.0.0.1',
  port: 8080,
  clients: 25,
  duration: 60, // seconds
  endpoint: 'getWorldInv',
  probePath: null, // derived from --endpoint if not set
  holdMs: 250, // time a client stays subscribed before unsubscribing
  reconnectDelayMs: 50, // brief pause between disconnect and reconnect
  openTimeoutMs: 5000,
  probeTimeoutMs: 5000,
  settleMs: 2000, // wait after closing all sockets before probing
};

function printUsage() {
  const lines = [
    'ws-stress-harness.mjs — Phase 2 WebSocket stress/churn verification harness',
    '',
    'Opens N concurrent WebSocket clients against FicsitRemoteMonitoring\'s uWS',
    'endpoint and loops each one through connect -> subscribe -> unsubscribe ->',
    'disconnect for --duration seconds. After the run, performs an HTTP liveness',
    'probe; exits non-zero if the server did not answer (crash/hang signal).',
    '',
    'Usage:',
    '  node build/tools/ws-stress-harness.mjs [options]',
    '',
    'Options:',
    `  --host <host>          Server host (default: ${DEFAULTS.host})`,
    `  --port <port>          uWS port (default: ${DEFAULTS.port}; dedicated server default is 8080, this box currently runs 8091)`,
    `  --clients <n>          Concurrent virtual clients (default: ${DEFAULTS.clients})`,
    `  --duration <seconds>   Total churn run duration in seconds (default: ${DEFAULTS.duration})`,
    `  --endpoint <name>      Endpoint name to subscribe/unsubscribe (default: ${DEFAULTS.endpoint})`,
    '  --probe-path <path>    HTTP liveness probe path (default: derived from --endpoint, e.g. /getWorldInv)',
    `  --hold-ms <ms>         Time a client stays subscribed before unsubscribing (default: ${DEFAULTS.holdMs})`,
    '  --help                 Print this usage and exit 0',
    '',
    'Exit codes:',
    '  0   All sockets closed cleanly AND the liveness probe returned HTTP 200.',
    '  1   Liveness probe failed or timed out (server crashed or hung).',
    '  2   Unexpected socket error pattern occurred during the churn run.',
    '',
    'Example:',
    '  node build/tools/ws-stress-harness.mjs --host 127.0.0.1 --port 8091 --clients 25 --duration 60',
  ];
  console.log(lines.join('\n'));
}

function parseArgs(argv) {
  const opts = { ...DEFAULTS };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    switch (arg) {
      case '--help':
      case '-h':
        opts.help = true;
        break;
      case '--host':
        opts.host = argv[++i];
        break;
      case '--port':
        opts.port = parseInt(argv[++i], 10);
        break;
      case '--clients':
        opts.clients = parseInt(argv[++i], 10);
        break;
      case '--duration':
        opts.duration = parseInt(argv[++i], 10);
        break;
      case '--endpoint':
        opts.endpoint = argv[++i];
        break;
      case '--probe-path':
        opts.probePath = argv[++i];
        break;
      case '--hold-ms':
        opts.holdMs = parseInt(argv[++i], 10);
        break;
      default:
        console.error(`Unknown argument: ${arg} (see --help)`);
        process.exit(2);
    }
  }
  if (!opts.probePath) {
    opts.probePath = `/${opts.endpoint}`;
  }
  return opts;
}

// --- Shared run-wide counters -------------------------------------------------

function makeCounters() {
  return {
    opens: 0,
    cleanCloses: 0,
    unexpectedCloses: 0,
    subscribesSent: 0,
    unsubscribesSent: 0,
    messagesReceived: 0,
    connectErrors: 0,
    otherErrors: 0,
  };
}

/**
 * Runs a single virtual client's connect -> subscribe -> unsubscribe ->
 * disconnect churn loop repeatedly until `isRunning()` returns false.
 */
async function runClientLoop(clientId, opts, counters, isRunning, activeSockets) {
  const url = `ws://${opts.host}:${opts.port}/`;

  while (isRunning()) {
    let ws;
    try {
      ws = await openSocket(url, opts.openTimeoutMs);
    } catch (err) {
      counters.connectErrors++;
      await sleep(opts.reconnectDelayMs);
      continue;
    }

    activeSockets.add(ws);
    counters.opens++;

    ws.addEventListener('message', () => {
      counters.messagesReceived++;
    });

    let unexpectedClose = false;
    const closePromise = new Promise((resolve) => {
      ws.addEventListener('close', (evt) => {
        activeSockets.delete(ws);
        // 1000 (normal) and 1005 (no status, common on a deliberate close())
        // are both treated as clean for this harness's purposes.
        if (evt.code !== 1000 && evt.code !== 1005) {
          unexpectedClose = true;
        }
        resolve();
      });
      ws.addEventListener('error', () => {
        unexpectedClose = true;
      });
    });

    try {
      // Subscribe frame — matches ProcessClientRequest's expected shape:
      // { "action": "subscribe", "endpoints": [...] }
      ws.send(JSON.stringify({ action: 'subscribe', endpoints: [opts.endpoint] }));
      counters.subscribesSent++;

      await sleep(opts.holdMs);

      if (!isRunning()) {
        // Duration elapsed while holding subscribed — still unsubscribe/close
        // cleanly rather than abandoning the socket mid-loop.
      }

      ws.send(JSON.stringify({ action: 'unsubscribe', endpoints: [opts.endpoint] }));
      counters.unsubscribesSent++;
    } catch (err) {
      counters.otherErrors++;
    }

    try {
      ws.close(1000, 'churn-cycle-complete');
    } catch (err) {
      counters.otherErrors++;
    }

    await closePromise;
    if (unexpectedClose) {
      counters.unexpectedCloses++;
    } else {
      counters.cleanCloses++;
    }

    await sleep(opts.reconnectDelayMs);
  }
}

function openSocket(url, timeoutMs) {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket(url);
    const timer = setTimeout(() => {
      try {
        ws.close();
      } catch (_) {
        /* ignore */
      }
      reject(new Error(`open timed out after ${timeoutMs}ms`));
    }, timeoutMs);

    ws.addEventListener('open', () => {
      clearTimeout(timer);
      resolve(ws);
    });
    ws.addEventListener('error', (err) => {
      clearTimeout(timer);
      reject(err);
    });
  });
}

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

/**
 * HTTP liveness probe using Node's built-in fetch. Returns true only on a
 * 200 response within the timeout; false (never throws) otherwise so the
 * caller can turn a crashed/hung server into a clear non-zero exit.
 */
async function livenessProbe(opts) {
  const url = `http://${opts.host}:${opts.port}${opts.probePath}`;
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), opts.probeTimeoutMs);
  try {
    const res = await fetch(url, { signal: controller.signal });
    clearTimeout(timer);
    return { ok: res.status === 200, status: res.status, url };
  } catch (err) {
    clearTimeout(timer);
    return { ok: false, status: null, url, error: err.message || String(err) };
  }
}

async function main() {
  const opts = parseArgs(process.argv.slice(2));

  if (opts.help) {
    printUsage();
    process.exit(0);
  }

  if (!Number.isInteger(opts.clients) || opts.clients <= 0) {
    console.error('--clients must be a positive integer');
    process.exit(2);
  }
  if (!Number.isInteger(opts.duration) || opts.duration <= 0) {
    console.error('--duration must be a positive integer (seconds)');
    process.exit(2);
  }
  if (typeof WebSocket === 'undefined') {
    console.error(
      'Global WebSocket is not available in this Node runtime. This harness ' +
        'requires Node 21+ (built-in WebSocket global). Detected: ' + process.version
    );
    process.exit(2);
  }

  const counters = makeCounters();
  const activeSockets = new Set();
  const startTime = Date.now();
  const endTime = startTime + opts.duration * 1000;
  const isRunning = () => Date.now() < endTime;

  console.log(
    `[ws-stress-harness] starting: host=${opts.host} port=${opts.port} ` +
      `clients=${opts.clients} duration=${opts.duration}s endpoint=${opts.endpoint} ` +
      `probe=${opts.probePath}`
  );

  const clientPromises = [];
  for (let i = 0; i < opts.clients; i++) {
    clientPromises.push(runClientLoop(i, opts, counters, isRunning, activeSockets));
  }

  await Promise.all(clientPromises);

  // Force-close any sockets still open past the duration window (defensive;
  // the per-client loop already closes cleanly on each cycle boundary).
  for (const ws of activeSockets) {
    try {
      ws.close(1000, 'harness-shutdown');
    } catch (_) {
      /* ignore */
    }
  }

  await sleep(opts.settleMs);

  const probeResult = await livenessProbe(opts);

  const elapsedSec = ((Date.now() - startTime) / 1000).toFixed(1);
  console.log('[ws-stress-harness] run summary:');
  console.log(`  elapsed:              ${elapsedSec}s`);
  console.log(`  opens:                ${counters.opens}`);
  console.log(`  clean closes:         ${counters.cleanCloses}`);
  console.log(`  unexpected closes:    ${counters.unexpectedCloses}`);
  console.log(`  subscribes sent:      ${counters.subscribesSent}`);
  console.log(`  unsubscribes sent:    ${counters.unsubscribesSent}`);
  console.log(`  messages received:    ${counters.messagesReceived}`);
  console.log(`  connect errors:       ${counters.connectErrors}`);
  console.log(`  other errors:         ${counters.otherErrors}`);
  console.log(
    `  liveness probe:       ${probeResult.url} -> ${
      probeResult.status !== null ? `HTTP ${probeResult.status}` : `FAILED (${probeResult.error})`
    }`
  );

  if (!probeResult.ok) {
    console.error(
      '[ws-stress-harness] FAIL: liveness probe did not return HTTP 200 — server ' +
        'may have crashed or hung under load.'
    );
    process.exit(1);
  }

  const hadUnexpectedSocketErrors =
    counters.unexpectedCloses > 0 || counters.otherErrors > 0 || counters.connectErrors > 0;

  if (hadUnexpectedSocketErrors) {
    console.error(
      '[ws-stress-harness] FAIL: unexpected socket error pattern occurred during the churn run ' +
        `(unexpectedCloses=${counters.unexpectedCloses}, otherErrors=${counters.otherErrors}, ` +
        `connectErrors=${counters.connectErrors}).`
    );
    process.exit(2);
  }

  console.log('[ws-stress-harness] PASS: no crash, liveness probe returned HTTP 200, all sockets closed cleanly.');
  process.exit(0);
}

main().catch((err) => {
  console.error('[ws-stress-harness] fatal error:', err);
  process.exit(2);
});
