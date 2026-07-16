# Build Runbook

Reproducible steps for compiling and packaging the FicsitRemoteMonitoring plugin
against the Satisfactory Mod Loader (SML) / Unreal Engine 5 toolchain on this
dev machine. This runbook explains each step; `build/package.sh` re-runs the
compile/package commands directly.

Status: this is a skeleton, filled in incrementally across this phase's plans.
Currently documents the **symlink → verify patch → compile** segment (plan
01-01). Packaging, deploy, and smoke-test steps are placeholders below,
completed by plans 01-02 and 01-03.

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

## Step 5 — Deploy (placeholder — added in plan 01-03)

TODO: unzip the packaged artifact(s) from `Saved/ArchivedPlugins/FicsitRemoteMonitoring/`
into a running `FactoryServer` (Linux) instance's `Mods/` directory (and/or
`FactoryGameEGS.exe` under wine for the client).

## Step 6 — Smoke test (placeholder — added in plan 01-03)

TODO: `curl -s http://localhost:8080/api/getWorldInv` against the launched
packaged instance — confirms the mod loaded with no errors and the monitoring
API is live end-to-end.

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
