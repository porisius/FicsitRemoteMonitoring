# Build Runbook

Reproducible steps for compiling and packaging the FicsitRemoteMonitoring plugin
against the Satisfactory Mod Loader (SML) / Unreal Engine 5 toolchain on this
dev machine. This runbook explains each step; `build/package.sh` re-runs the
compile/package commands directly.

This runbook documents the full pipeline end-to-end: **symlink → verify patch →
compile → package → deploy → smoke-test**, plus a Known pitfalls section. Every
step below has been run and verified live on this machine.

## Local environment facts (this machine)

- Unreal Engine (Satisfactory CSS fork): `/mnt/data/UE`
- SML host project (Starter Project): `/home/fabrice/dev/SatisfactoryModLoader`
- `wwise-cli`: on `PATH`; Wwise SDK already downloaded and integrated into the
  host project's Wwise project (`SatisfactoryModLoader_WwiseProject`). This is
  a one-time, already-completed provisioning step — it is **not** part of the
  repeatable compile/package sequence below and `build/package.sh` never
  invokes it.
- `wine` + `msvc-wine`: already configured (`UE_WINE_MSVC` exported in
  `~/.bashrc`, pointing at `/opt/msvc-wine`). Provides the MSVC toolchain UBT
  needs to produce real Win64 binaries when cross-compiling the client from
  Linux.

## Step 1 — Symlink the plugin into the host project

SML mods are developed as standalone git repos symlinked (never copied — this
must stay a live dev loop against this repo's working tree) into the host
Starter Project's `Mods/GameFeatures/<PluginName>/` directory. It **must** be
directly under `Mods/GameFeatures/` (not plain `Mods/`, not
`Mods/GameFeatures/GameFeatures/`) — Alpakit's packaging automation
(`IsGameFeatureDLC()`) keys off this exact path to decide how to stage the mod
and mark it as a Game Feature; getting the location wrong produces a mod that
either isn't recognized or mounts its content at the wrong runtime path.

```bash
ln -s /home/fabrice/dev/FicsitRemoteMonitoring \
      /home/fabrice/dev/SatisfactoryModLoader/Mods/GameFeatures/FicsitRemoteMonitoring
```

Verify it resolves correctly:

```bash
readlink -f /home/fabrice/dev/SatisfactoryModLoader/Mods/GameFeatures/FicsitRemoteMonitoring
# expected: /home/fabrice/dev/FicsitRemoteMonitoring
```

## Step 2 — Verify the SML header patch is applicable

`Source/FicsitRemoteMonitoringServer/FicsitRemoteMonitoringServer.build.cs`
auto-applies `Patches/FGServerAPIManager-FRM-04162025.patch` (flips a
`private:` to `public:` before `friend class UFGServerAPIManager;` at line
~116 of the host project's `FGServerAPIManager.h`, so the server module can
access `FFGRequestData`). It does this by shelling out to `patch -N -s` at UBT
module-rules construction time, **before** the first compile.

**Caveat — silent-fail path:** the patch-apply logic catches failures and only
logs to stdout; it does not fail the UBT invocation. A missing/broken `patch`
binary, or an already-diverged header, would otherwise surface later as a
confusing C++ compile error instead of a clear "patch failed" message.
De-risk this before the first compile with a dry-run:

```bash
patch -N -s --dry-run \
  /home/fabrice/dev/SatisfactoryModLoader/Source/FactoryDedicatedServer/Public/Networking/FGServerAPIManager.h \
  /home/fabrice/dev/FicsitRemoteMonitoring/Patches/FGServerAPIManager-FRM-04162025.patch
```

Exit code `0` means the patch can apply cleanly (or is already applied). Do
not apply it manually — the build.cs applies it automatically on the first
`RunUBT.sh` invocation in Step 3. After that first compile, sanity-check it
actually took:

```bash
sed -n '116p' /home/fabrice/dev/SatisfactoryModLoader/Source/FactoryDedicatedServer/Public/Networking/FGServerAPIManager.h
# expected: public:   (was private: before the patch applied)
```

## Step 3 — Compile

Compiles the `FactoryEditor` target for Linux, which builds both plugin
modules (`FicsitRemoteMonitoring` runtime + `FicsitRemoteMonitoringServer`
server-only) and triggers the patch from Step 2.

```bash
bash build/package.sh compile
```

Equivalent to running directly:

```bash
UE_DIR=/mnt/data/UE
SML_PROJECT=/home/fabrice/dev/SatisfactoryModLoader/FactoryGame.uproject

"$UE_DIR/Engine/Build/BatchFiles/RunUBT.sh" FactoryEditor Linux Development -project="$SML_PROJECT"
```

Override `UE_DIR` / `SML_PROJECT` environment variables if your paths differ
from this machine's defaults.

Confirm both modules were reported as built in the RunUBT.sh output, and
re-run the Step 2 sanity check to confirm the patch applied.

## Step 4 — Package

```bash
bash build/package.sh package
```

Equivalent to running directly:

```bash
"$UE_DIR/Engine/Build/BatchFiles/RunUAT.sh" \
  -ScriptsForProject="$SML_PROJECT" \
  PackagePlugin \
  -project="$SML_PROJECT" \
  -DLCName=FicsitRemoteMonitoring \
  -build \
  -clientconfig=Shipping -serverconfig=Shipping \
  -platform=Win64 \
  -server -serverplatform=Linux \
  -nocompileeditor \
  -utf8output
```

Produces both required artifacts under
`<SML_PROJECT>/Saved/ArchivedPlugins/FicsitRemoteMonitoring/`:
`FicsitRemoteMonitoring-Windows.zip` (client) and
`FicsitRemoteMonitoring-LinuxServer.zip` (server). The Win64 zip's
`Binaries/Win64/` already contains `uv.dll` and `zlib1.dll` via the
`RuntimeDependencies` staging declared in the build.cs files — no manual DLL
copy step was needed on this machine.

**Required external dependency — ArduinoKit:** this repo's
`Content/Subsystems/FicsitRemoteMonitoring_BP.uasset` hard-references
Blueprint nodes (serial/RS232 I/O) from the `ArduinoKit` plugin. Without it,
the cook stage fails outright (`ExitCode=25`, `Error_UnknownCookFailure`,
log mentions `/Script/ArduinoKit`). Per `CONTRIBUTING.md`, clone it into the
host project's `Mods/` folder (note: `Mods/`, not `Mods/GameFeatures/`):

```bash
git clone https://github.com/porisius/ArduinoKit \
  /home/fabrice/dev/SatisfactoryModLoader/Mods/ArduinoKit
```

Then **re-run Step 3 (compile)** before packaging again — `ArduinoKit`'s own
C++ module is only source after cloning; `-nocompileeditor` in the package
stage means the newly-added module must already be built, or the cook fails
a second time with `Plugin 'ArduinoKit' failed to load because module
'ArduinoKit' could not be found`. This machine did not need `DiscIt` (it did
not surface in the cook failure, unlike ArduinoKit); do not pre-emptively
clone it — only add it if a future cook run demonstrably references it.

ArduinoKit's clone lives in the sibling `SatisfactoryModLoader` checkout, not
in this repo — it is a build-environment dependency of the *host project*,
not a code change to this plugin, so it is not (and should not be) tracked by
this repo's git.

## Step 5 — Deploy

Unzip the packaged `-LinuxServer.zip` artifact from
`Saved/ArchivedPlugins/FicsitRemoteMonitoring/` into the dedicated server's mod
tree. **Critical:** FRM must land under `Mods/GameFeatures/`, not plain `Mods/`:

```bash
SRV=/mnt/data/satisfactory-server/SatisfactoryDedicatedServer
unzip -o \
  /home/fabrice/dev/SatisfactoryModLoader/Saved/ArchivedPlugins/FicsitRemoteMonitoring/FicsitRemoteMonitoring-LinuxServer.zip \
  -d "$SRV/FactoryGame/Mods/GameFeatures/FicsitRemoteMonitoring"
```

Resulting layout (the `.uplugin` and `Content/Paks/` land directly under this dir):

```
FactoryGame/Mods/GameFeatures/FicsitRemoteMonitoring/FicsitRemoteMonitoring.uplugin
FactoryGame/Mods/GameFeatures/FicsitRemoteMonitoring/Content/Paks/LinuxServer/FicsitRemoteMonitoringFactoryGame-LinuxServer.{pak,utoc,ucas}
```

**Why `Mods/GameFeatures/` and not `Mods/`** (this is a real, confirmed failure
mode — see `.planning/debug/resolved/frm-worldmodule-not-cooked.md`): FRM is a
Game Feature mod. Its pak is cooked with a baked-in mount point of
`../../../FactoryGame/Mods/GameFeatures/FicsitRemoteMonitoring/` (Step 1's symlink
location drives this — Alpakit bakes the plugin's `Mods/GameFeatures/<Name>` path
into the pak). If you deploy the plugin one level too shallow at
`Mods/FicsitRemoteMonitoring/`, the runtime plugin content root no longer matches
the pak's baked mount point, so the cooked packages — including
`Content/InitGameWorld` (FRM's `GameWorldModule` blueprint) — never resolve under
the `FicsitRemoteMonitoring` plugin. SML's root-module asset-registry scan then
misses FRM entirely: the log reads `Discovered 2 world modules` (SML + ArduinoKit
only), **no** `AFicsitRemoteMonitoring*` subsystem spawns, `Registered API
Endpoint` stays `0`, and `/frm` reports "Unknown command" — even though the pak
mounts cleanly and SML still logs `FicsitRemoteMonitoring: 1.5.2` (that version
line comes from the C++ Binaries, not the content pak, which is what masks the
failure). Contrast: ArduinoKit is a plain mod cooked with mount point
`Mods/ArduinoKit/`, so it deploys to `Mods/ArduinoKit/` and matches.

Verify the deploy path before launching:

```bash
test -f "$SRV/FactoryGame/Mods/GameFeatures/FicsitRemoteMonitoring/FicsitRemoteMonitoring.uplugin" \
  && echo "FRM deployed under GameFeatures (correct)" \
  || { echo "WRONG PATH — FRM not under Mods/GameFeatures/"; exit 1; }
```

Launch the server (output captured for the smoke test in Step 6):

```bash
cd "$SRV"
./FactoryServer.sh -log -unattended > /mnt/data/satisfactory-server/server-run.log 2>&1 &
```

Confirm discovery succeeded once a session loads:

```bash
grep 'Discovered .* world modules' /mnt/data/satisfactory-server/server-run.log | tail -1
# expected: "Discovered 3 world modules of class GameWorldModule" (SML + ArduinoKit + FRM)
grep -c 'Registered API Endpoint' /mnt/data/satisfactory-server/server-run.log
# expected: > 0  (93 on this build)
```

## Step 6 — Smoke test

FRM's uWS HTTP server is **off by default** on a dedicated server (`uWS.Autostart`
defaults to `false`) and binds `uWS.Port` (default `8080`). `uWS.Autostart=1` (or
running `/frm http start` in-game) is required before the server opens the port —
without it the port never binds and the curl below will fail to connect regardless
of whether the mod loaded correctly.

The canonical smoke test, matching this phase's stated verification bar, is:

```bash
curl -sf http://localhost:8080/api/getWorldInv
```

Expect HTTP 200 with a JSON body. Both `/getWorldInv` and `/api/getWorldInv` are
served by the uWS router; either path form works against the same port.

**If port `8080` is already occupied** (e.g. a colocated Satisfactory game client
also running FRM on this machine, as on this dev box), set `uWS.Port` to a free
port before launching and curl that port instead. Settings live under the
`FicsitRemoteMonitoring.Server.` prefix in the server's `GameUserSettings.ini`;
booleans are stored in `mIntValues` as `0/1`:

```ini
[/Script/FactoryGame.FGGameUserSettings]
mIntValues=(("FicsitRemoteMonitoring.Server.uWS.Port", 8091),("FicsitRemoteMonitoring.Server.uWS.Autostart", 1))
```

After launch + session load, confirm the listener and query live data:

```bash
ss -tlnp | grep ':8091'                         # expect a FactoryServer LISTEN
curl -s http://localhost:8091/getModList         # expect JSON incl. "Ficsit Remote Monitoring" 1.5.2
curl -s -o /dev/null -w 'HTTP %{http_code}\n' \
     http://localhost:8091/getWorldInv           # expect HTTP 200
```

A `200` with well-formed JSON confirms the monitoring API is live end-to-end on the
dedicated server.

### Validating the Windows client package (`-Windows.zip`)

The Linux server package is the primary target, but the Windows client build is
validated the same way — deploy it into a game **client** install and load a
**single-player** world (no dedicated server needed; the world is ready immediately).
On the client, `uWS.Autostart` is typically already enabled and the options live
under the `FicsitRemoteMonitoring.` prefix (no `Server.` segment). Deploy path is the
same `Mods/GameFeatures/FicsitRemoteMonitoring/`:

```bash
# with the game client CLOSED, back up any existing install first, then:
unzip -o FicsitRemoteMonitoring-Windows.zip \
  -d "<client>/FactoryGame/Mods/GameFeatures/FicsitRemoteMonitoring"
```

Launch the client, load a single-player world, then:

```bash
curl -s -o /dev/null -w 'HTTP %{http_code}\n' http://localhost:8080/getWorldInv
```

Expect HTTP 200 (the body may be `[]` on a brand-new save with no inventory
containers — an empty JSON array is still a valid 200). Confirm in the client's
`FactoryGame.log` that the mod loaded with **no** module-load error (the vendored
`uv.dll`/`zlib1.dll` in the package's `Binaries/Win64/` mean the CONTRIBUTING.md
DLL-copy fallback is normally unnecessary) and that SML logged
`Discovered N world modules of class GameWorldModule` counting your FRM install.

> Note: the alternate dedicated-server route — the game Server API on `:7777` via a
> custom `Frm` function (`UFRM_Controller::Handler_Frm`) — currently returns
> `bad_function` even after the FRM server subsystem spawns; the mod's
> `RegisterRequestHandler` does not surface in the `:7777` dispatch table. Use the
> uWS route above for smoke testing until that separate issue is resolved.

## Known pitfalls

- **Plugin not yet linked into the host project** — the very first build
  attempt fails with "plugin not found" if Step 1 was skipped.
- **SML header patch silent-fail** — see Step 2's caveat; always dry-run
  before trusting a green compile as proof the patch applied.
- **Missing `ArduinoKit` Blueprint dependency** — confirmed on this machine:
  the cook stage fails outright without it (see Step 4). `DiscIt` did not
  surface as a failure in this run; do not pre-emptively clone it.
- **`-nocompileeditor` before the editor has ever built this plugin (or a
  newly-cloned dependency plugin like `ArduinoKit`)** — always run the
  explicit compile (Step 3) after cloning `ArduinoKit` and before packaging
  with `-nocompileeditor`, or the package step fails with `Plugin
  'ArduinoKit' failed to load because module 'ArduinoKit' could not be
  found` — confirmed on this machine.
- **Disk space** — the root filesystem (`/`) on this machine is at ~95%
  capacity with a limited margin free; UE's cook → stage → archive pipeline
  creates multiple intermediate copies per platform. Monitor free space
  during packaging; if it becomes a blocker, this is a pre-existing
  environment constraint outside this phase's scope to fix.
- **DLL copy for the Windows client** — per `CONTRIBUTING.md`'s troubleshooting
  section, if the packaged Win64 client fails to load with a module-load
  error, copy `zlib.dll` and `uv.dll` from `Source/ThirdParty/uWebSockets/lib`
  to the packaged mod's `Binaries/Win64` directory. Try without this manual
  copy first — `RuntimeDependencies` staging declared in the build.cs files
  should already handle it.
- **Deployed one level too shallow (`Mods/FicsitRemoteMonitoring/` instead of
  `Mods/GameFeatures/FicsitRemoteMonitoring/`)** — confirmed failure mode on
  this machine, see Step 5. FRM's pak is cooked with a baked-in mount point
  under `Mods/GameFeatures/`; deploying to plain `Mods/` deceptively looks
  fine (SML still logs `FicsitRemoteMonitoring: 1.5.2` from the C++ Binaries,
  and the pak still mounts) but the content pak's `GameWorldModule` never
  resolves, so SML's log reads `Discovered 2 world modules` instead of `3`,
  `Registered API Endpoint` stays at `0`, and `/frm` reports "Unknown
  command". Always verify with the Step 5 `test -f ... .uplugin` deploy-path
  guard and the `Discovered 3 world modules` / endpoint-count check before
  moving on to the smoke test.
