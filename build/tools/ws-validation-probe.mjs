#!/usr/bin/env node
/**
 * ws-validation-probe.mjs
 *
 * Wave-0 VALD-01 verification instrument for Phase 4 (request-validation-hardening).
 * Not shipped plugin code, not wired into build/package.sh — run manually per
 * build/RUNBOOK.md. This is the failing end-to-end test that exists BEFORE the
 * C++ validator (Plan 02): it defines the concrete acceptance target, and is
 * the instrument the live-verification checkpoint (Plan 03) runs against a
 * deployed dedicated server.
 *
 * Drives a battery of malformed and well-formed WebSocket subscribe/unsubscribe
 * payloads against FRM's live uWS endpoint and asserts each produces the
 * expected `{"error": ...}` frame (or, for the well-formed regression case,
 * the expected push data with NO error frame). See 04-VALIDATION.md's
 * "Per-Task Verification Map" (SC#1a/SC#1b/SC#2/SC#3) and 04-RESEARCH.md's
 * "Phase Requirements -> Test Map" for the exact payloads and expected
 * outcomes this script encodes.
 *
 * Zero new dependencies: uses Node's built-in global `WebSocket` (available
 * since Node 21+) and the built-in global `fetch`. Do NOT `npm install ws` —
 * the npm `ws` package was audited and REJECTED for this project's probes
 * (see .planning/phases/02-thread-safe-websocket-request-handling/02-RESEARCH.md,
 * "Package Legitimacy Audit"; reaffirmed for Phase 4 in 04-RESEARCH.md).
 *
 * Usage:
 *   node build/tools/ws-validation-probe.mjs --help
 *   node build/tools/ws-validation-probe.mjs
 *   node build/tools/ws-validation-probe.mjs --host 127.0.0.1 --port 8091
 *
 * Exit codes:
 *   0   All cases behaved as expected (malformed cases produced error frames,
 *       well-formed regression case succeeded).
 *   1   A validation case regressed (a malformed case produced no error frame,
 *       or the well-formed case failed).
 *   2   Connection/protocol error (could not reach the server, or an
 *       unexpected exception occurred during the run).
 */

'use strict';

const DEFAULTS = {
  host: '127.0.0.1',
  port: 8080,
  timeoutMs: 3000, // per-case receive timeout
  openTimeoutMs: 5000,
};

function printUsage() {
  const lines = [
    'ws-validation-probe.mjs — Phase 4 VALD-01 malformed-payload WS validation probe',
    '',
    'Opens WebSocket connections against FicsitRemoteMonitoring\'s uWS endpoint and',
    'sends a battery of malformed and well-formed subscribe/unsubscribe payloads,',
    'asserting each produces the expected {"error": ...} frame (or, for the',
    'well-formed regression case, the expected push data with no error frame).',
    '',
    'Usage:',
    '  node build/tools/ws-validation-probe.mjs [options]',
    '',
    'Options:',
    `  --host <host>          Server host (default: ${DEFAULTS.host})`,
    `  --port <port>          uWS port (default: ${DEFAULTS.port}; dedicated server default is 8080)`,
    `  --timeout-ms <ms>      Per-case receive timeout in ms (default: ${DEFAULTS.timeoutMs})`,
    '  --help                 Print this usage and exit 0',
    '',
    'Exit codes:',
    '  0   All cases behaved as expected (malformed -> error frame, well-formed -> success).',
    '  1   A validation case regressed.',
    '  2   Connection/protocol error.',
    '',
    'Example:',
    '  node build/tools/ws-validation-probe.mjs --host 127.0.0.1 --port 8091',
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
      case '--timeout-ms':
        opts.timeoutMs = parseInt(argv[++i], 10);
        break;
      default:
        console.error(`Unknown argument: ${arg} (see --help)`);
        process.exit(2);
    }
  }
  return opts;
}

// --- Socket helpers ------------------------------------------------------

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
 * Collects frames received on `ws` for `windowMs`, parsing each as JSON where
 * possible (raw string kept alongside on parse failure). Resolves with the
 * list of frames observed in that window; never rejects on timeout (an empty
 * list is a valid, meaningful result — "no frame arrived").
 */
function collectFrames(ws, windowMs) {
  return new Promise((resolve) => {
    const frames = [];
    let closed = false;

    const onMessage = (evt) => {
      let parsed = null;
      try {
        parsed = JSON.parse(evt.data);
      } catch (_) {
        parsed = null;
      }
      frames.push({ raw: evt.data, json: parsed });
    };
    const onClose = () => {
      closed = true;
    };

    ws.addEventListener('message', onMessage);
    ws.addEventListener('close', onClose);

    setTimeout(() => {
      ws.removeEventListener('message', onMessage);
      ws.removeEventListener('close', onClose);
      resolve({ frames, closed });
    }, windowMs);
  });
}

function isErrorFrame(frame) {
  return !!(frame && frame.json && typeof frame.json === 'object' && 'error' in frame.json);
}

function isPushFrameFor(frame, endpointName) {
  return !!(
    frame &&
    frame.json &&
    typeof frame.json === 'object' &&
    frame.json.endpoint === endpointName &&
    !('error' in frame.json)
  );
}

// --- Case runner -----------------------------------------------------------

/**
 * Runs a single probe case: opens a fresh connection, sends `payload` (a
 * pre-serialized string, so unparseable-JSON cases can be exercised too),
 * collects frames for `timeoutMs`, and hands them to `assert(frames, closed)`
 * which must return `{ pass: boolean, detail: string }`.
 */
async function runCase(name, opts, payload, assertFn) {
  const url = `ws://${opts.host}:${opts.port}/`;
  let ws;
  try {
    ws = await openSocket(url, opts.openTimeoutMs);
  } catch (err) {
    return { name, pass: false, detail: `connect failed: ${err.message || err}`, connectError: true };
  }

  try {
    ws.send(payload);
    const { frames, closed } = await collectFrames(ws, opts.timeoutMs);
    const result = assertFn(frames, closed);
    return { name, pass: result.pass, detail: result.detail };
  } catch (err) {
    return { name, pass: false, detail: `unexpected error: ${err.message || err}`, connectError: true };
  } finally {
    try {
      ws.close(1000, 'probe-case-complete');
    } catch (_) {
      /* ignore */
    }
  }
}

// --- Probe cases (one per VALD-01 test-map row) -----------------------------

async function caseUnknownEndpoint(opts) {
  return runCase(
    'unknown-endpoint-name',
    opts,
    JSON.stringify({ action: 'subscribe', endpoints: ['definitelyNotARealEndpoint'] }),
    (frames) => {
      const errorFrame = frames.find(isErrorFrame);
      if (!errorFrame) {
        return { pass: false, detail: 'expected an error frame, got none' };
      }
      const msg = JSON.stringify(errorFrame.json.error);
      if (!msg.includes('definitelyNotARealEndpoint')) {
        return { pass: false, detail: `error frame did not mention the offending name: ${msg}` };
      }
      return { pass: true, detail: `got error frame mentioning offending name: ${msg}` };
    }
  );
}

async function caseGetOnlyRejection(opts) {
  return runCase(
    'get-only-rejection-setSwitches',
    opts,
    JSON.stringify({ action: 'subscribe', endpoints: ['setSwitches'] }),
    (frames) => {
      const errorFrame = frames.find(isErrorFrame);
      if (!errorFrame) {
        return { pass: false, detail: 'expected an immediate error frame for POST-only endpoint, got none' };
      }
      const pushFrame = frames.find((f) => isPushFrameFor(f, 'setSwitches'));
      if (pushFrame) {
        return { pass: false, detail: 'unexpected push frame for setSwitches arrived (should be rejected at subscribe time)' };
      }
      return { pass: true, detail: 'got error frame, no setSwitches push frame arrived' };
    }
  );
}

async function caseMissingAction(opts) {
  return runCase(
    'missing-action',
    opts,
    JSON.stringify({ endpoints: ['getWorldInv'] }),
    (frames) => {
      const errorFrame = frames.find(isErrorFrame);
      if (!errorFrame) {
        return { pass: false, detail: 'expected an error frame for missing action, got none' };
      }
      return { pass: true, detail: `got error frame: ${JSON.stringify(errorFrame.json.error)}` };
    }
  );
}

async function caseUnknownAction(opts) {
  return runCase(
    'unknown-action',
    opts,
    JSON.stringify({ action: 'frobnicate', endpoints: ['getWorldInv'] }),
    (frames) => {
      const errorFrame = frames.find(isErrorFrame);
      if (!errorFrame) {
        return { pass: false, detail: 'expected an error frame for unknown action, got none' };
      }
      return { pass: true, detail: `got error frame: ${JSON.stringify(errorFrame.json.error)}` };
    }
  );
}

async function caseWrongTypedEndpoints(opts) {
  return runCase(
    'wrong-typed-endpoints',
    opts,
    JSON.stringify({ action: 'subscribe', endpoints: 5 }),
    (frames) => {
      const errorFrame = frames.find(isErrorFrame);
      if (!errorFrame) {
        return { pass: false, detail: 'expected an error frame for wrong-typed endpoints field, got none' };
      }
      return { pass: true, detail: `got error frame: ${JSON.stringify(errorFrame.json.error)}` };
    }
  );
}

async function caseMixedValidInvalid(opts) {
  return runCase(
    'mixed-valid-invalid-partial-success',
    opts,
    JSON.stringify({ action: 'subscribe', endpoints: [123, 'getWorldInv'] }),
    (frames) => {
      const errorFrame = frames.find(isErrorFrame);
      if (!errorFrame) {
        return { pass: false, detail: 'expected an error frame listing the invalid entry, got none' };
      }
      const msg = JSON.stringify(errorFrame.json.error);
      if (msg.includes('getWorldInv')) {
        return { pass: false, detail: `error frame should list only the invalid entry, but mentions getWorldInv: ${msg}` };
      }
      const pushFrame = frames.find((f) => isPushFrameFor(f, 'getWorldInv'));
      if (!pushFrame) {
        return { pass: false, detail: `got error frame (${msg}) but no getWorldInv push data arrived (partial success failed)` };
      }
      return { pass: true, detail: `got error frame for invalid entry (${msg}) AND getWorldInv push data (partial success)` };
    }
  );
}

async function caseUnparseableJson(opts) {
  const url = `ws://${opts.host}:${opts.port}/`;
  let ws;
  try {
    ws = await openSocket(url, opts.openTimeoutMs);
  } catch (err) {
    return { name: 'unparseable-json', pass: false, detail: `connect failed: ${err.message || err}`, connectError: true };
  }

  try {
    ws.send('{not json');
    const { frames, closed } = await collectFrames(ws, opts.timeoutMs);
    if (closed) {
      return { name: 'unparseable-json', pass: false, detail: 'socket closed on unparseable JSON (D-02 requires it stay open)' };
    }
    const errorFrame = frames.find(isErrorFrame);
    if (!errorFrame) {
      return { name: 'unparseable-json', pass: false, detail: 'expected an error frame for unparseable JSON, got none' };
    }
    // Confirm the socket is genuinely still usable, not just un-closed-yet.
    let stillOpen = true;
    try {
      ws.send(JSON.stringify({ action: 'unsubscribe', endpoints: ['getWorldInv'] }));
    } catch (_) {
      stillOpen = false;
    }
    if (!stillOpen || ws.readyState !== WebSocket.OPEN) {
      return { name: 'unparseable-json', pass: false, detail: 'socket not usable/open after unparseable JSON error frame' };
    }
    return {
      name: 'unparseable-json',
      pass: true,
      detail: `got error frame (${JSON.stringify(errorFrame.json.error)}), socket stayed open`,
    };
  } catch (err) {
    return { name: 'unparseable-json', pass: false, detail: `unexpected error: ${err.message || err}`, connectError: true };
  } finally {
    try {
      ws.close(1000, 'probe-case-complete');
    } catch (_) {
      /* ignore */
    }
  }
}

async function caseWellFormedRegression(opts) {
  return runCase(
    'well-formed-regression',
    opts,
    JSON.stringify({ action: 'subscribe', endpoints: ['getWorldInv'] }),
    (frames) => {
      const errorFrame = frames.find(isErrorFrame);
      if (errorFrame) {
        return { pass: false, detail: `well-formed request produced an unexpected error frame: ${JSON.stringify(errorFrame.json.error)}` };
      }
      const pushFrame = frames.find((f) => isPushFrameFor(f, 'getWorldInv'));
      if (!pushFrame) {
        return { pass: false, detail: 'expected getWorldInv push data, got none (and no error frame either)' };
      }
      return { pass: true, detail: 'got getWorldInv push data, no error frame (regression check passed)' };
    }
  );
}

// --- Main --------------------------------------------------------------

async function main() {
  const opts = parseArgs(process.argv.slice(2));

  if (opts.help) {
    printUsage();
    process.exit(0);
  }

  if (typeof WebSocket === 'undefined') {
    console.error(
      'Global WebSocket is not available in this Node runtime. This probe ' +
        'requires Node 21+ (built-in WebSocket global). Detected: ' + process.version
    );
    process.exit(2);
  }

  console.log(`[ws-validation-probe] starting: host=${opts.host} port=${opts.port} timeoutMs=${opts.timeoutMs}`);

  const cases = [
    caseUnknownEndpoint,
    caseGetOnlyRejection,
    caseMissingAction,
    caseUnknownAction,
    caseWrongTypedEndpoints,
    caseMixedValidInvalid,
    caseUnparseableJson,
    caseWellFormedRegression,
  ];

  const results = [];
  let hadConnectError = false;

  for (const caseFn of cases) {
    let result;
    try {
      result = await caseFn(opts);
    } catch (err) {
      result = { name: caseFn.name, pass: false, detail: `fatal error: ${err.message || err}`, connectError: true };
    }
    results.push(result);
    if (result.connectError) hadConnectError = true;
    // Brief pause between cases to avoid overlapping push-loop timing artifacts.
    await sleep(150);
  }

  console.error('[ws-validation-probe] case results:');
  for (const r of results) {
    const status = r.pass ? 'PASS' : 'FAIL';
    console.error(`  [${status}] ${r.name}: ${r.detail}`);
  }

  const failCount = results.filter((r) => !r.pass).length;
  const passCount = results.length - failCount;
  console.error(`[ws-validation-probe] summary: ${passCount}/${results.length} passed`);

  if (hadConnectError) {
    console.error('[ws-validation-probe] FAIL: connection/protocol error occurred during the run.');
    process.exit(2);
  }

  if (failCount > 0) {
    console.error('[ws-validation-probe] FAIL: one or more validation cases regressed.');
    process.exit(1);
  }

  console.error('[ws-validation-probe] PASS: all validation cases behaved as expected.');
  process.exit(0);
}

main().catch((err) => {
  console.error('[ws-validation-probe] fatal error:', err);
  process.exit(2);
});
