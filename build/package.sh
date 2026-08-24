set -euo pipefail

# build/package.sh — FicsitRemoteMonitoring build/package pipeline
#
# Reproducible wrapper around this project's verified UBT/UAT invocations
# (see build/RUNBOOK.md and .planning/phases/01-build-environment-configuration/01-RESEARCH.md).
#
# IMPORTANT: run this script with an explicit bash interpreter, e.g.:
#   bash build/package.sh compile
# (there is intentionally no shebang line — invoking it directly via
# `./build/package.sh` on a system where /bin/sh is not bash, e.g. Ubuntu's
# dash, will fail because `set -o pipefail` is a bash-only option).
#
# Stages (first positional argument; defaults to "compile"):
#   compile   Compiles FactoryEditor (Linux, Development). Builds both plugin
#             modules (FicsitRemoteMonitoring + FicsitRemoteMonitoringServer)
#             and triggers the SML header patch auto-apply. This is the only
#             stage implemented so far — see build/RUNBOOK.md for status.
#   package   Not yet implemented (added in a later plan of this phase —
#             packages both the Win64 client and Linux server via RunUAT.sh
#             PackagePlugin).
#
# Environment variables (override the this-machine defaults below):
#   UE_DIR       Path to the Unreal Engine (Satisfactory CSS fork) checkout.
#                Default: /mnt/data/UE
#   SML_PROJECT  Path to the SatisfactoryModLoader host project's .uproject.
#                Default: /home/fabrice/dev/SatisfactoryModLoader/FactoryGame.uproject
#
# No secrets are embedded or logged by this script. It only invokes local,
# fixed-path engine tooling already installed on the build machine.

UE_DIR="${UE_DIR:-/mnt/data/UE}"
SML_PROJECT="${SML_PROJECT:-/home/fabrice/dev/SatisfactoryModLoader/FactoryGame.uproject}"

STAGE="${1:-compile}"

stage_compile() {
	echo "==> [compile] RunUBT.sh FactoryEditor Linux Development -project=${SML_PROJECT}"
	"${UE_DIR}/Engine/Build/BatchFiles/RunUBT.sh" FactoryEditor Linux Development -project="${SML_PROJECT}"
}

stage_package() {
	echo "==> [package] RunUAT.sh PackagePlugin -DLCName=FicsitRemoteMonitoring -platform=Win64 -server -serverplatform=Linux"
	# D-01/D-02 scope: Win64 client + Linux server only. Deliberately OMITS
	# -serverplatform=Win64+Linux (no WindowsServer target), -installed (this
	# is a source-built engine, not an Epic-installed one), and -merge (keep
	# separate per-platform zips). Shipping config for both — the only
	# supported/tested configuration per Alpakit's own default. Relies on
	# -nocompileeditor since the compile stage above already built the
	# editor with both plugin modules linked in.
	"${UE_DIR}/Engine/Build/BatchFiles/RunUAT.sh" \
		-ScriptsForProject="${SML_PROJECT}" \
		PackagePlugin \
		-project="${SML_PROJECT}" \
		-DLCName=FicsitRemoteMonitoring \
		-build \
		-clientconfig=Shipping -serverconfig=Shipping \
		-platform=Win64 \
		-server -serverplatform=Linux \
		-nocompileeditor \
		-utf8output
}

case "${STAGE}" in
	compile)
		stage_compile
		;;
	package)
		stage_package
		;;
	*)
		echo "Unknown stage: ${STAGE} (expected: compile|package)" >&2
		exit 1
		;;
esac
