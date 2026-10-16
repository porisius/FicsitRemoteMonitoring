{
  description = "Satisfactory modding environment for building Ficsit Remote Monitoring (LinuxServer and Win64 targets)";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      # Versions pinned to match SatisfactoryModLoader's own CI
      # (.github/workflows/build.yml) so we build the same way upstream does.
      engineTag = "5.6.1-css-83";
      wwiseSdkVersion = "2025.1.6.9117";
      # Intentionally different from the SDK version: Wwise's API uses the first
      # 3 version parts, skips the 4th, and appends a 5th "integration" part.
      wwiseIntegrationVersion = "2025.1.6.4248";

      # Everything else we download is pinned too, so everyone builds against
      # the same tree. SML's CI tracks these at latest; bump them together
      # with engineTag when upstream moves.
      smlRev = "8eebc9e659f0b2ff4392d2f8cf6e2cff60c8ea49"; # SatisfactoryModLoader dev, 2026-10-12
      arduinoKitRev = "483528eae5cb1526fd8ba97e7fc3dc210afb034f";
      msvcWineRev = "61b7cbc8cb9413fec8f0016ef433a90e06c37f54"; # mircearoata/msvc-wine ue-patches
      wwiseCliVersion = "v0.2.4";
      wwiseCliSha256 = "23ff2858e275ed33c02fad7ab96b0bab12c8a0c2015a90c6df820a55dca6ea9f";
      # Third-party mirror of Microsoft's Debugging Tools MSI (for pdbcopy),
      # so verify it rather than trusting whatever "latest" is today.
      debuggersMsiTag = "10.0.19041.0";
      debuggersMsiSha256 = "e9eda93064502a6998dc94c85484b2e46e7c0781041653c28407df9b6cf4a22d";

      # The engine and starter project must be writable (the Linux setup guide
      # has you edit engine source in place), so they live outside /nix/store.
      defaultWorkspace = "$HOME/satisfactory-modding";

      toolPkgs = with pkgs; [
        # Prerequisites from the Linux modding guide.
        #
        # Win64 cross-compilation via msvc-wine needs wine + msitools + gcab.
        # SML's CI builds wine 11.0 from source only to get an unmerged
        # oleaut32 typelib patch (winehq MR 9640), and its own comment says
        # that is for "opening blueprint nodes in VS" -- a Visual Studio IDE
        # feature we do not use from the CLI. nixpkgs already has wine 11.0.
        #
        # wineWow64Packages (not the deprecated wineWowPackages) is the modern
        # single-binary WoW64 build and, unlike wineWowPackages, is prebuilt in
        # the binary cache rather than a multi-hour source build.
        wineWow64Packages.stable
        msitools # msiextract, needs >=0.98; nixpkgs has 0.106
        gcab # needs >=1.2; nixpkgs has 1.6
        perl

        git
        gh # to download the engine release archives
        wget
        curl
        cacert
        openssh
        zstd
        gnutar
        dos2unix

        # REQUIRED, easy to miss: FicsitRemoteMonitoringServer.build.cs (and
        # frm-link) shell out to a bare `patch` to flip FGServerAPIManager.h:116
        # from `private:` to `public:`. If absent the build prints "Patch
        # failed" and keeps going, and the C++ then fails to compile for an
        # unrelated-looking reason.
        # Mods/WwisePatches/applyPatches.sh needs `patch` and dos2unix too.
        gnupatch

        gnumake
        coreutils
        which
        file
        unzip
        zip
        p7zip
        bashInteractive
        python3
        pciutils
        xdg-utils
      ];

      # UE ships prebuilt binaries (UnrealBuildTool, its own dotnet 8.0.300, the
      # clang toolchain, the editor) that all assume an FHS layout. nix-ld could
      # cover some of that, but an explicit FHS env is more predictable since
      # we need a GUI editor and not just one dynamic loader fixup.
      runtimeLibs = with pkgs; [
        stdenv.cc.cc.lib
        zlib
        openssl
        icu
        expat
        libxml2
        libunwind
        lttng-ust
        elfutils
        util-linux # libuuid
        systemdLibs # libudev

        # Graphics / Vulkan. buildFHSEnv wires up /run/opengl-driver, so the
        # host NVIDIA driver is picked up automatically.
        vulkan-loader
        vulkan-tools
        libglvnd
        libGL
        libGLU
        mesa
        libdrm
        libxshmfence

        libx11
        libxcursor
        libxrandr
        libxinerama
        libxi
        libxext
        libxfixes
        libxrender
        libxscrnsaver
        libxcomposite
        libxdamage
        libxtst
        libxcb
        libsm
        libice
        libxkbcommon

        # Audio. Needed to *link*, even though we generate no soundbanks.
        alsa-lib
        libpulseaudio

        SDL2
        fontconfig
        freetype

        # GTK stack: the editor uses it for native file dialogs.
        glib
        gtk3
        atk
        at-spi2-core
        at-spi2-atk
        pango
        cairo
        gdk-pixbuf
        dbus

        # CEF, embedded in the editor's web browser widget.
        nss
        nspr
      ];

      fhs = pkgs.buildFHSEnv {
        name = "frm-dev";
        targetPkgs = _: toolPkgs ++ runtimeLibs ++ scriptPkgs;

        profile = ''
          export FRM_ENGINE_TAG="${engineTag}"
          export FRM_WWISE_SDK_VERSION="${wwiseSdkVersion}"
          export FRM_WWISE_INTEGRATION_VERSION="${wwiseIntegrationVersion}"
          export FRM_SML_REV="${smlRev}"
          export FRM_ARDUINOKIT_REV="${arduinoKitRev}"
          export FRM_MSVC_WINE_REV="${msvcWineRev}"
          export FRM_WWISE_CLI_VERSION="${wwiseCliVersion}"
          export FRM_WWISE_CLI_SHA256="${wwiseCliSha256}"
          export FRM_DEBUGGERS_MSI_TAG="${debuggersMsiTag}"
          export FRM_DEBUGGERS_MSI_SHA256="${debuggersMsiSha256}"
          export FRM_WORKSPACE="''${FRM_WORKSPACE:-${defaultWorkspace}}"
          export UE_DIR="$FRM_WORKSPACE/ue"
          export SML_DIR="$FRM_WORKSPACE/SML"
          # Must be the live git checkout, NOT the /nix/store copy of the flake
          # source -- we symlink this into the project and edit it in place, so
          # `toString ./.` would give a read-only store path and break editing.
          export FRM_DIR="''${FRM_DIR:-$(${pkgs.git}/bin/git rev-parse --show-toplevel 2>/dev/null || pwd)}"

          # UE's zen storage server tries to raise RLIMIT_NOFILE to infinity
          # (2^30) while paking, which fails outright rather than clamping.
          # SML's CI works around it with `ulimit -n 1048576`. Raise the soft
          # limit to whatever the hard limit allows; if packaging still fails on
          # nofile, raise the hard limit in your NixOS config:
          #   systemd.extraConfig = "DefaultLimitNOFILE=1048576";
          ulimit -n "$(ulimit -Hn)" 2>/dev/null || true

          export SSL_CERT_FILE="${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt"

          # Win64 cross-compile. UE_WINE_MSVC is what UBT reads to find the
          # msvc-wine toolchain; the name is fixed by the engine, not by us.
          export UE_WINE_MSVC="''${UE_WINE_MSVC:-$FRM_WORKSPACE/msvc}"
          export WINEPREFIX="''${WINEPREFIX:-$FRM_WORKSPACE/.wine}"
          export WINEDEBUG="''${WINEDEBUG:--all}"

          cat <<'BANNER'
          Satisfactory modding env (LinuxServer target)

            $UE_DIR      engine       $SML_DIR   starter project
            $FRM_DIR     this mod     $FRM_WORKSPACE  (set before entering to move;
                                                       needs ~60GB free)

          Before you start you need:
            - A GitHub account linked to your Epic account, with the EpicGames
              org invite accepted. The engine repo is private; without this
              frm-fetch-engine fails with a 404. See
              https://www.unrealengine.com/en-US/ue-on-github
            - A free Audiokinetic account, exported as WWISE_EMAIL and
              WWISE_PASSWORD, for frm-fetch-wwise.

          Bootstrap (once, in order):
            1.  gh auth login
            2.  frm-fetch-engine     engine archives + register  (large)
            3.  frm-fetch-sml        clone starter project
            4.  frm-fetch-wwise      SDK only, no GUI, no soundbanks (needs WWISE_*)
            5.  frm-fetch-deps       ArduinoKit (required; must precede 7)
            6.  frm-link             symlink this mod + patch the engine header
            7.  frm-build-editor     compile FactoryEditor Linux Development

          Then, per target:
            LinuxServer:  frm-package
            Windows:      frm-fetch-msvc (once)  ->  frm-package-windows
                          frm-install-windows    drop into the Steam install

          Edit/rebuild loop:  frm-build-editor -> frm-package-windows
                                                -> frm-install-windows

          Run `frm-status` to see which steps are done.
          BANNER
        '';

        runScript = "bash";
      };

      # Bootstrap helpers. Kept as scripts rather than baked into the profile so
      # they are readable and individually re-runnable.
      scripts = {
        frm-status = ''
          check() { if [ -e "$2" ]; then echo "  ok      $1"; else echo "  missing $1"; fi; }
          echo "workspace: $FRM_WORKSPACE"
          check "engine"          "$UE_DIR/Engine/Build/BatchFiles/Linux/Build.sh"
          check "engine scripts"  "$UE_DIR/Engine/Build/BatchFiles/BuildUBT.sh"
          check "starter project" "$SML_DIR/FactoryGame.uproject"
          check "wwise plugin"    "$SML_DIR/Plugins/Wwise"
          check "arduinokit"      "$SML_DIR/Mods/ArduinoKit/ArduinoKit.uplugin"
          check "mod linked"      "$SML_DIR/Mods/GameFeatures/FicsitRemoteMonitoring"
          # Note the lib* prefix -- UE names these libUnrealEditor-*.so on Linux.
          check "editor built"    "$SML_DIR/Binaries/Linux/libUnrealEditor-FactoryGame.so"
          check "mod built"       "$SML_DIR/Mods/GameFeatures/FicsitRemoteMonitoring/Binaries/Linux/libUnrealEditor-FicsitRemoteMonitoringServer.so"
          check "msvc (win64)"    "$UE_WINE_MSVC/.installed"
          check "pkg linuxserver" "$SML_DIR/Saved/ArchivedPlugins/FicsitRemoteMonitoring/FicsitRemoteMonitoring-LinuxServer.zip"
          check "pkg windows"     "$SML_DIR/Saved/ArchivedPlugins/FicsitRemoteMonitoring/FicsitRemoteMonitoring-Windows.zip"
        '';

        # Prefers the prebuilt archive over a source build: the guide notes a
        # source build takes 2-3h and "a boatload of disk space".
        frm-fetch-engine = ''
          set -euo pipefail
          mkdir -p "$UE_DIR" && cd "$UE_DIR"

          # Pin the release explicitly. Bare `gh release download` takes the
          # latest release, which would silently drift from $FRM_ENGINE_TAG and
          # mismatch the BuildUBT.sh we fetch below.
          if [ ! -x ./Engine/Build/BatchFiles/Linux/Build.sh ]; then
            # Drop parts left over from an interrupted run; gh refuses to
            # overwrite them otherwise.
            rm -f UnrealEngine-CSS-Editor-Linux.tar.zst.*
            echo "Downloading prebuilt Linux editor archives (~29GB extracted, skipping -Debug)..."
            gh release download "$FRM_ENGINE_TAG" \
              --repo satisfactorymodding/UnrealEngine \
              -p "UnrealEngine-CSS-Editor-Linux.tar.zst.*"
            cat UnrealEngine-CSS-Editor-Linux.tar.zst.* | zstd -d | tar -xf -
            rm -f UnrealEngine-CSS-Editor-Linux.tar.zst.*
            ./Engine/Binaries/Linux/UnrealVersionSelector -register -unattended
          else
            echo "Engine already extracted, skipping download."
          fi

          # Not shipped in the archive; needed to avoid redundant recompiles on
          # every package. satisfactorymodding/UnrealEngine is a PRIVATE repo,
          # so raw.githubusercontent.com 404s -- must go through authenticated
          # `gh api` instead.
          for s in BuildUBT.sh DotnetDepends.sh; do
            gh api -H "Accept: application/vnd.github.raw" \
              "repos/satisfactorymodding/UnrealEngine/contents/Engine/Build/BatchFiles/$s?ref=$FRM_ENGINE_TAG" \
              > "Engine/Build/BatchFiles/$s.tmp"
            mv "Engine/Build/BatchFiles/$s.tmp" "Engine/Build/BatchFiles/$s"
            chmod +x "Engine/Build/BatchFiles/$s"
          done
          echo "Engine ready ($FRM_ENGINE_TAG)."
          echo "Note: commenting out the GlobalSingleInstanceMutex block in"
          echo "UEBuildTarget.cs and deleting Engine/Source/Programs/LiveLinkHub"
          echo "is only needed for Rider -- skipped."
        '';

        frm-fetch-sml = ''
          set -euo pipefail
          if [ -d "$SML_DIR/.git" ]; then
            echo "Starter project already cloned at $SML_DIR"
            echo "(pinned rev is $FRM_SML_REV; check out manually to move it)."
            exit 0
          fi
          git clone https://github.com/satisfactorymodding/SatisfactoryModLoader.git "$SML_DIR"
          git -C "$SML_DIR" checkout -q "$FRM_SML_REV"
          echo "Cloned starter project to $SML_DIR at $FRM_SML_REV"
        '';

        # This is the whole audio story. SML's CI omits `Packages=Authoring`,
        # so there is no Wwise.exe, no Wine, no project creation and no
        # soundbank generation. The SDK itself is NOT optional: FactoryGame's
        # build.cs hard-depends on AkAudio / WwiseSoundEngine /
        # WwiseResourceCooker / WwiseProjectDatabase / WwiseResourceLoader.
        # Needs a free Audiokinetic account via WWISE_EMAIL / WWISE_PASSWORD.
        frm-fetch-wwise = ''
          set -euo pipefail
          : "''${WWISE_EMAIL:?set WWISE_EMAIL (free Audiokinetic account)}"
          : "''${WWISE_PASSWORD:?set WWISE_PASSWORD}"
          cd "$FRM_WORKSPACE"
          if [ ! -x ./wwise-cli ]; then
            gh release download "$FRM_WWISE_CLI_VERSION" --repo mircearoata/wwise-cli \
              -p wwise-cli_linux_amd64 -O wwise-cli.tmp
            echo "$FRM_WWISE_CLI_SHA256  wwise-cli.tmp" | sha256sum -c --quiet
            mv wwise-cli.tmp wwise-cli
            chmod +x ./wwise-cli
          fi
          ./wwise-cli download --sdk-version "$FRM_WWISE_SDK_VERSION" \
            --filter Packages=SDK \
            --filter DeploymentPlatforms=Windows_vc160 \
            --filter DeploymentPlatforms=Windows_vc170 \
            --filter DeploymentPlatforms=Linux \
            --filter DeploymentPlatforms= >/dev/null
          ./wwise-cli integrate-ue \
            --integration-version "$FRM_WWISE_INTEGRATION_VERSION" \
            --project "$SML_DIR/FactoryGame.uproject" >/dev/null
          bash "$SML_DIR/Mods/WwisePatches/applyPatches.sh" "$SML_DIR"
        '';

        # ArduinoKit is a REAL requirement, not stale documentation.
        # FicsitRemoteMonitoring_BP.uasset calls serial-port nodes (Open/Close/
        # IsOpen/WriteText/ReadBytes/StringFromByteArray) that live in it.
        # Without it the cook fails with a wall of "Could not find a function
        # named ..." LogBlueprint errors and Error_UnknownCookFailure (25).
        # Grepping Content/ for the string "ArduinoKit" finds nothing -- the
        # blueprint references the functions, not the plugin name -- so absence
        # of that string is not evidence it is unused.
        #
        # It goes in Mods/, NOT Mods/GameFeatures/. It must be cloned BEFORE
        # frm-build-editor, otherwise its module is missing at cook time and
        # you get "Plugin 'ArduinoKit' failed to load because module
        # 'ArduinoKit' could not be found."
        #
        # CONTRIBUTING.md also lists DiscIt, but no such repo exists publicly
        # under porisius any more, and the LinuxServer package cooks fine
        # without it.
        frm-fetch-deps = ''
          set -euo pipefail
          mkdir -p "$SML_DIR/Mods"
          if [ -d "$SML_DIR/Mods/ArduinoKit/.git" ]; then
            echo "ArduinoKit already present."
          else
            git clone https://github.com/porisius/ArduinoKit.git \
              "$SML_DIR/Mods/ArduinoKit"
            git -C "$SML_DIR/Mods/ArduinoKit" checkout -q "$FRM_ARDUINOKIT_REV"
          fi
        '';

        # Win64 toolchain. Mirrors SML CI's "Setup MSVC" step.
        #
        # NOTE: this downloads Microsoft's Visual Studio build tools and
        # accepts their license on your behalf via --accept-license. The
        # resulting toolchain is NOT redistributable (msvc-wine's own README:
        # "As Visual Studio isn't redistributable, the installed toolchain
        # isn't either"), which is why it lives outside the nix store as a
        # user-supplied dependency, same as the engine.
        frm-fetch-msvc = ''
          set -euo pipefail
          cache="$FRM_WORKSPACE/msvc-download"
          mkdir -p "$cache"

          if [ ! -d "$FRM_WORKSPACE/msvc-wine/.git" ]; then
            git clone -b ue-patches https://github.com/mircearoata/msvc-wine.git \
              "$FRM_WORKSPACE/msvc-wine"
            git -C "$FRM_WORKSPACE/msvc-wine" checkout -q "$FRM_MSVC_WINE_REV"
          fi

          # pdbcopy ships in the Debuggers-and-Tools MSI, not in the VS
          # packages vsdownload pulls, so it has to be fetched separately.
          msi="$cache/X64.Debuggers.And.Tools-x64_en-us.msi"
          if [ ! -f "$msi" ]; then
            wget -O "$msi.tmp" "https://github.com/kbandla/installers/releases/download/$FRM_DEBUGGERS_MSI_TAG/X64.Debuggers.And.Tools-x64_en-us.msi"
            mv "$msi.tmp" "$msi"
          fi
          echo "$FRM_DEBUGGERS_MSI_SHA256  $msi" | sha256sum -c --quiet

          if [ ! -f "$UE_WINE_MSVC/.installed" ]; then
            "$FRM_WORKSPACE/msvc-wine/vsdownload.py" \
              --dest "$UE_WINE_MSVC" --cache "$cache" \
              --accept-license \
              --channel release.ltsc.17.8 \
              --msvc-version 17.8 \
              --sdk-version 10.0.22621 \
              Microsoft.Net.4.8.SDK Microsoft.VisualStudio.MinShell

            msiextract -C "$UE_WINE_MSVC" "$msi" > /dev/null

            "$FRM_WORKSPACE/msvc-wine/install.sh" "$UE_WINE_MSVC"

            # UBT shells out to `pdbcopy` to strip private symbols. msvc-wine
            # does not wrap it, so hand-roll the same shim CI inlines.
            cat > "$UE_WINE_MSVC/bin/x64/pdbcopy" <<'PDBEOF'
          #!/usr/bin/env bash
          . "$(dirname "$0")"/msvcenv.sh
          SDKDEBUGBINDIR="$SDKBASE/Debuggers/x64"
          "$(dirname "$0")"/wine-msvc.sh "$SDKDEBUGBINDIR/pdbcopy.exe" "$@"
          PDBEOF
            ln -sf ./pdbcopy "$UE_WINE_MSVC/bin/x64/pdbcopy.exe"
            chmod +x "$UE_WINE_MSVC"/bin/x64/pdbcopy*

            touch "$UE_WINE_MSVC/.installed"
          else
            echo "MSVC already installed at $UE_WINE_MSVC"
          fi

          WINEDLLOVERRIDES="mscoree,mshtml=" wineboot -u
          wineserver -w
          echo "MSVC ready: $UE_WINE_MSVC"
        '';

        frm-link = ''
          set -euo pipefail
          if [ ! -f "$FRM_DIR/FicsitRemoteMonitoring.uplugin" ]; then
            echo "FRM_DIR=$FRM_DIR does not look like the mod repo" >&2
            echo "(no FicsitRemoteMonitoring.uplugin). Set FRM_DIR explicitly." >&2
            exit 1
          fi
          mkdir -p "$SML_DIR/Mods/GameFeatures"
          ln -sfn "$FRM_DIR" "$SML_DIR/Mods/GameFeatures/FicsitRemoteMonitoring"
          echo "Linked $FRM_DIR -> $SML_DIR/Mods/GameFeatures/FicsitRemoteMonitoring"

          # Apply the FGServerAPIManager patch up front. The build.cs applies it
          # too, but doing it here means a fresh checkout compiles first try and
          # a failure shows up now instead of buried in UBT output.
          #
          # Without the patch, FGServerAPIManager.h:116 stays `private:` and
          # FRM_Controller.cpp:30 fails to compile on RequestHandlerContext->
          # RequestHeaders. Idempotent: --dry-run tells us if it is still needed.
          patchFile="$FRM_DIR/Patches/FGServerAPIManager-FRM-04162025.patch"
          if patch -N -p1 -d "$SML_DIR" --dry-run -s -i "$patchFile" >/dev/null 2>&1; then
            patch -N -p1 -d "$SML_DIR" -s -i "$patchFile"
            echo "Applied FGServerAPIManager patch."
          else
            echo "FGServerAPIManager patch already applied (or no longer applies)."
          fi
        '';

        frm-build-editor = ''
          set -euo pipefail
          exec "$UE_DIR/Engine/Build/BatchFiles/Linux/Build.sh" \
            FactoryEditor Linux Development -project="$SML_DIR/FactoryGame.uproject"
        '';

        # Server-only variant of SML's CI packaging step.
        #
        # `-target=FactoryServer` is load-bearing and NOT just a hint. Alpakit's
        # PackagePlugin.GetClientTargetsToCook() otherwise returns a hardcoded
        # ["FactoryGameEGS", "FactoryGameSteam"], so it tries to build a *client*
        # target; with no -platform it defaults to the host (Linux) and UBT dies
        # with "Game targets for the Linux platform are not currently supported
        # from this engine distribution." `-noclient` does not prevent this --
        # it only guards the deploy/archive steps.
        #
        # The escape hatch is the `if (ParseParam("target")) return null;` branch
        # at the top of that method, which hands target selection to
        # BuildCookRun. UAT's ParseParam matches a `-target=` prefix, not just a
        # bare `-target`, so `-target=FactoryServer` trips it.
        frm-package = ''
          set -euo pipefail
          exec "$UE_DIR/Engine/Build/BatchFiles/RunUAT.sh" \
            -ScriptsForProject="$SML_DIR/FactoryGame.uproject" PackagePlugin \
            -Project="$SML_DIR/FactoryGame.uproject" \
            -dlcname=FicsitRemoteMonitoring \
            -build -server -noclient -target=FactoryServer \
            -serverconfig=Shipping -serverplatform=Linux \
            -nocompileeditor -installed -merge -utf8output
        '';

        # Win64 client package -- the one you can actually load in the Steam /
        # Proton install. Same -target= trick as frm-package (see the long note
        # there); here it also lets us build ONLY the Steam client variant
        # instead of Alpakit's default FactoryGameEGS+FactoryGameSteam pair.
        # Swap to -target=FactoryGameEGS for an Epic install, or drop -target
        # entirely to build both for an upstream-parity release.
        # No -server, so the server targets are skipped.
        frm-package-windows = ''
          set -euo pipefail
          : "''${UE_WINE_MSVC:?run frm-fetch-msvc first}"
          [ -f "$UE_WINE_MSVC/.installed" ] || { echo "MSVC not installed; run frm-fetch-msvc" >&2; exit 1; }
          exec "$UE_DIR/Engine/Build/BatchFiles/RunUAT.sh" \
            -ScriptsForProject="$SML_DIR/FactoryGame.uproject" PackagePlugin \
            -Project="$SML_DIR/FactoryGame.uproject" \
            -dlcname=FicsitRemoteMonitoring \
            -build -target=FactoryGameSteam \
            -clientconfig=Shipping -platform=Win64 \
            -nocompileeditor -installed -merge -utf8output
        '';

        # Drop the packaged Windows mod straight into the Steam install.
        #
        # Destination is Mods/GameFeatures/, NOT Mods/ -- Alpakit stamps
        # "GameFeature": true into the packaged uplugin, which per the SML docs
        # means "install to the GameFeatures subdirectory of the Mods
        # directory". Installing to Mods/ directly would not be loaded.
        #
        # That directory is normally managed by Satisfactory Mod Manager
        # (see Mods/default-lock.json), so we stash whatever is there first.
        # To go back to the released build, restore the backup or just hit
        # reinstall in SMM.
        frm-install-windows = ''
          set -euo pipefail
          gameDir="''${SATISFACTORY_DIR:-$HOME/.local/share/Steam/steamapps/common/Satisfactory}"
          zip="$SML_DIR/Saved/ArchivedPlugins/FicsitRemoteMonitoring/FicsitRemoteMonitoring-Windows.zip"
          [ -d "$gameDir" ] || { echo "Game dir not found: $gameDir" >&2; exit 1; }
          [ -f "$zip" ] || { echo "No Windows package; run frm-package-windows" >&2; exit 1; }

          dest="$gameDir/FactoryGame/Mods/GameFeatures/FicsitRemoteMonitoring"
          if [ -d "$dest" ] && [ ! -d "$dest.smm-backup" ]; then
            cp -a "$dest" "$dest.smm-backup"
            echo "Backed up existing install -> $dest.smm-backup"
          fi
          rm -rf "$dest"; mkdir -p "$dest"
          unzip -q -o "$zip" -d "$dest"
          echo "Installed to $dest"
          ls "$dest/Binaries/Win64"
        '';

        frm-editor = ''
          exec "$UE_DIR/Engine/Binaries/Linux/UnrealEditor" "$SML_DIR/FactoryGame.uproject"
        '';
      };

      scriptPkgs =
        nixpkgs.lib.mapAttrsToList (n: v: pkgs.writeShellScriptBin n v) scripts;
    in
    {
      packages.${system}.default = fhs;

      # `nix develop` and `nix run` both drop you into the same FHS sandbox.
      devShells.${system}.default = fhs.env;

      apps.${system}.default = {
        type = "app";
        program = "${fhs}/bin/frm-dev";
      };
    };
}
