# Changelog

All notable changes to FicsitRemoteMonitoring are documented here.
Format is loosely based on [Keep a Changelog](https://keepachangelog.com/).

---

## [Unreleased] — Reliability Retrofit

> **Baseline:** v1.5.2 · **Focus:** reliability of the monitoring channel, not new features.
> A targeted retrofit hardening the HTTP/WebSocket API so it returns accurate, real-time
> game state without crashing or corrupting the dedicated server.
>
> **No new external dependencies.** SML `^3.12.0` compatibility preserved. All public API
> response shapes are unchanged or additive-only (see *Compatibility* below).

### TL;DR for reviewers

| Area | What changed | Why it matters |
|------|--------------|----------------|
| **WebSocket threading** | All inbound callbacks and outbound sends are now marshaled through the game thread. | Removes a whole class of race-condition crashes / shared-state corruption. |
| **`getPlayer`** | Crash-safe reads, new `location.lookPitch`, persistent player names across disconnect. | Endpoint no longer crashes the server; richer, more reliable data. |
| **WS request validation** | Malformed/malicious subscribe envelopes are rejected with a clear error *before* dispatch. | Bad input can no longer reach endpoint logic on the now-guaranteed game thread. |
| **Dedicated-server packaging** | Branded 503 fallback when the web UI is missing; icon staging verified against a real packaged build. | Servers without the web UI bundle degrade gracefully instead of breaking silently. |
| **Dev tooling** | Reproducible build/package pipeline, WS stress + validation harnesses, deploy/verify runbook. | Makes the plugin buildable and verifiable on Linux (no test framework exists). |

Because the project has **no automated test framework**, every runtime change was verified
**live against a deployed Satisfactory dedicated server** (see *Verification* below), not just
by compiling.

---

## Shipping changes (plugin runtime)

### 1. Thread-safe WebSocket request handling

**Files:** `Source/FicsitRemoteMonitoring/Private/FicsitRemoteMonitoring.cpp`,
`Source/FicsitRemoteMonitoring/Public/FicsitRemoteMonitoring.h`

**Problem.** Inbound WebSocket callbacks (`open`/`close`/message) and the outbound push loop
touched shared subsystem state — `ConnectedClients`, `EndpointSubscribers`, subscription maps —
directly from the uWS background thread. Concurrent access with the game thread caused
race-condition crashes and state corruption.

**Change.**
- Introduced generation-tagged client identity: a game-thread-owned `ClientGenerations` map,
  loop-thread-only `NextClientIDCounter` / `LoopLiveSockets` bookkeeping, and an
  `IsClientCurrent(ws, ClientID)` validity helper — **no new `FWebSocketUserData` field and no
  `FCriticalSection`** (correctness comes from thread confinement + generation checks).
- `wsBehavior.open` mints the `ClientID` on the loop thread (where `ws` is provably fresh),
  then marshals `ConnectedClients` / `ClientGenerations` registration to the game thread via a
  `bShouldStop`-guarded `AsyncTask(GameThread, …)`. Also fixed a pre-existing copy-paste log bug
  ("Client Disconnected" was logged on connect).
- `wsBehavior.close` removes `ws` from `LoopLiveSockets` on the loop thread, then a marshaled,
  `IsClientCurrent`-guarded game-thread task removes it from `ConnectedClients`,
  `ClientGenerations`, and every `EndpointSubscribers` entry. The old `OnClientDisconnected`
  path was **retired** and folded in here.
- `ProcessClientRequest` now extracts the action / endpoint list / `(ws, ClientID)` on the loop
  thread only, then performs **all** subscribe/unsubscribe mutation inside one
  `AsyncTask(GameThread, …)` block, dropping stale/not-yet-registered messages instead of erroring.
- `PushUpdatedData` no longer reads `EndpointSubscribers` or calls `send()` on the push-pacing
  thread. It hops to the game thread to build JSON and snapshot recipients (re-validated against
  `ClientGenerations`), then **defers each `send()` onto the captured uWS loop** (`CapturedLoop`),
  where the callback re-checks `LoopLiveSockets` + the generation tag before sending. Payloads are
  copied into owning `std::string`s (no dangling stack buffers).
- `StopWebSocketServer` reworked identically: snapshot recipients on the game thread, defer the
  `close()` calls onto `CapturedLoop` with the same liveness guard. `CapturedLoop` is captured
  once on the loop thread before `app.run()` and reset to `nullptr` on teardown.
- `bHasRunningPushDataLoop` discipline moved fully onto the game thread.

**Client impact:** none functionally — message shapes and push behavior are unchanged; the fix is
purely about *where* the work runs.

---

### 2. `getPlayer` endpoint reliability

**Files:** `Source/FicsitRemoteMonitoring/Private/Endpoints/World/PlayerLibrary.cpp`,
`Source/FicsitRemoteMonitoring/Public/FicsitRemoteMonitoring.h` (name cache),
`Source/FicsitRemoteMonitoring/FicsitRemoteMonitoring.build.cs` (`CoreOnline` module)

- **Crash-safety:** the two known crash sites — unchecked `GetInventory()->GetInventoryStacks(…)`
  and `GetHealthComponent()->GetCurrentHealth()/IsDead()` — are now `IsValid()`-guarded with
  fallbacks (`Inventory:[]`, `PlayerHP:0`, `Dead:false`). The `Cast<AFGCharacterPlayer>` result is
  guarded too (defense in depth).
- **New field:** `location.lookPitch` on every live player entry — signed camera/look pitch from
  `AFGPlayerController::GetControlRotation().Pitch`, clamped to `[-90, 90]` (never actor body
  rotation). Named `lookPitch` (not `pitch`) so it coexists with upstream's generic actor-rotation
  `pitch` field (porisius/FicsitRemoteMonitoring#297) instead of shadowing it: `pitch` = body/actor
  pitch (meaningful for vehicles), `lookPitch` = the player's camera/aim pitch (the only pitch
  meaningful for an upright player body).
- **Persistent player names:** a session-persistent `PlayerNameCache` (`net-id → name`, keyed by
  the stable `APlayerState::GetUniqueId().ToString()`) lets `getPlayer` emit offline players from
  cache via a hand-built JSON path that never dereferences a despawned actor.
- **Dedicated-server-safe identity:** names use `APlayerState::GetUniqueId()`, **deliberately not**
  `AFGPlayerState::GetUserID()` (which SIGSEGVs on dedicated servers). `CoreOnline` was linked so
  `FUniqueNetIdWrapper::ToString()` resolves.

---

### 3. WebSocket request validation (VALD-01)

**Files:** `Source/FicsitRemoteMonitoring/Private/FicsitRemoteMonitoring.cpp`,
`Source/FicsitRemoteMonitoring/Public/FicsitRemoteMonitoring.h`

- New private `ValidateWSSubscriptionEnvelope(...)` performs strict envelope validation on
  subscribe/unsubscribe payloads **before** any endpoint logic runs: `action` present and known;
  `endpoints` must be a string or an array of strings (rejects missing/`null`/object/mixed-typed);
  names matched **GET-only** against the endpoint registry
  (`APIName == Name && Method == "GET"`). Unknown/invalid names are collected and reported with a
  **partial-success** contract; the reported-problems list is capped (`MaxReportedProblems = 20`
  + `"…and N more"`) so an adversarial all-invalid array can't drive an oversized reflected frame.
- Wired into `ProcessClientRequest` (subscribe path) and `OnMessageReceived` (JSON parse-failure).
  Rejected input returns an `{"error": …}` frame and **never reaches dispatch or shared-state
  mutation**; the socket stays open and usable.
- The HTTP `CallEndpoint` path is untouched (zero diff) — no regression for existing REST clients.

---

### 4. Dedicated-server asset packaging & fallback page

**Files:** `Source/FicsitRemoteMonitoring/Private/FicsitRemoteMonitoring.cpp` (catch-all +
`/Icons/*` serving), `Source/FicsitRemoteMonitoring/Private/FRM_Request.cpp` +
`Source/FicsitRemoteMonitoring/Public/FRM_Request.h` (`SendFallbackPage`),
`Config/FilterPlugin.ini` (inert marker)

- **Branded 503 fallback (PKG-02):** when the web UI bundle (`www/`) isn't deployed — common on
  dedicated servers — a page navigation now returns a branded `503` HTML page
  (`SendFallbackPage`) pointing at `/api/` and the docs, instead of a bare error. The body is a
  fixed compile-time string (no request data echoed → no injection surface).
- **Carve-outs:** the fallback fires **only** for extensionless / `.html` / `.htm` page requests
  **and** only when the path does not name a registered endpoint. Sub-resources (`.js/.css/.avif/
  .woff2`), `/Icons/*`, and `/api/*` keep their normal `404`/JSON content-type contract.
- **Bare-form API guard (CR-01):** the catch-all consults the endpoint registry first, so
  extensionless API calls (e.g. `/getWorldInv`, `/getModList`) keep returning **JSON**, not the
  fallback page, even while `www/` is missing.
- **Icon staging (PKG-01):** confirmed that `Config/PluginSettings.ini`
  (`[StageSettings] +AdditionalNonUSFDirectories=Icons`) is the real mechanism that stages the
  `Icons/` directory into a packaged build — **verified against a real LinuxServer zip**, not just
  in-editor. `Config/FilterPlugin.ini` was marked *inert* with a comment so no maintainer mistakes
  it for the packaging lever. `PluginSettings.ini` itself was **not modified** (already correct).
- Fixed an MSVC-only unity-build compile break (`C4459`: a `SendFallbackPage` parameter shadowed a
  file-scope `DocsURL`), which only surfaced on the Windows client target during full packaging.

---

## Developer tooling & repo scaffolding (non-shipping)

None of these ship in the packaged plugin; they make it buildable and verifiable.

- **`build/package.sh`** — reproducible `compile` / `package` wrapper around the verified
  RunUBT / RunUAT (`PackagePlugin`) invocations (Win64 client + Linux server, Shipping).
- **`build/RUNBOOK.md`** — deploy/verify runbook: WS stress procedure, PKG-02 fallback smoke
  battery, and the icon release workflow (generate on a client → copy into `Icons/` → package →
  verify via `unzip -l`).
- **`build/tools/ws-stress-harness.mjs`** — zero-dependency Node script driving N concurrent WS
  clients through connect→subscribe→unsubscribe→disconnect churn; exits non-zero on crash/leak.
- **`build/tools/ws-validation-probe.mjs`** — zero-dependency Node probe encoding the VALD-01
  malformed-payload cases; exits non-zero on any validation regression.
- **`.gitignore`** — ignore the ephemeral planning research cache.

---

## Compatibility

**Deliberately unchanged:**
- `Config/PluginSettings.ini` — already staged `Icons`/`www` correctly.
- HTTP `CallEndpoint` dispatch path — no behavior change for REST clients.
- SML `^3.12.0` dependency; **no new external C++/JS libraries** (uses already-linked Unreal `Json`
  and the vendored uWebSockets).

**Additive-only API changes** (no breaking changes to existing consumers):
- `getPlayer` entries gain `location.lookPitch` (camera/aim pitch; distinct from the generic actor
  `pitch` field added upstream in porisius/FicsitRemoteMonitoring#297).
- `getPlayer` may now include offline players (from the persistent name cache).
- WebSocket subscribe/unsubscribe now returns an `{"error": …}` frame for malformed envelopes
  (previously undefined behavior); well-formed requests are unaffected.

---

## Verification

No automated test framework exists in this project (by design — see `.claude/CLAUDE.md`).
Each phase was verified **live on a deployed Satisfactory Linux dedicated server**:

- **Threading:** `ws-stress-harness.mjs` under connection churn — clean closes, `Remaining
  connections: 0` after each cycle, no crash/leak.
- **Validation:** `ws-validation-probe.mjs` — 8/8 malformed-payload cases rejected correctly;
  25-client churn showed no regression for valid clients.
- **Packaging:** real `LinuxServer.zip` inspected via `unzip -l` (Icons/ staged); PKG-02 six-case
  curl battery (503 fallback + carve-outs + bare-form JSON guard) all green.
- Per-phase `*-VERIFICATION.md` and `*-SECURITY.md` (threats_open: 0) accompany the work in
  `.planning/phases/`.

---

## Reviewer file map

| File | Area | Nature |
|------|------|--------|
| `Source/…/FicsitRemoteMonitoring.cpp` | threading + validation + fallback/Icons serving | core logic (+506/−92) |
| `Source/…/FicsitRemoteMonitoring.h` | declarations, `ClientGenerations`, `PlayerNameCache`, validator | header |
| `Source/…/Endpoints/World/PlayerLibrary.cpp` | `getPlayer` hardening + `location.lookPitch` + offline names | endpoint (+191) |
| `Source/…/FRM_Request.cpp` / `.h` | `SendFallbackPage` (503 page) | helper |
| `Source/…/FicsitRemoteMonitoring.build.cs` | `+CoreOnline` module | build |
| `Config/FilterPlugin.ini` | inert marker comment | config (no functional effect) |
| `build/package.sh`, `build/RUNBOOK.md`, `build/tools/*.mjs` | build + verification tooling | non-shipping |
| `.gitignore` | repo scaffolding | non-shipping |
