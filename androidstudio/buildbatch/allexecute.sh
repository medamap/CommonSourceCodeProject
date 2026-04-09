#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

csv_file="$BUILD_BATCH_DIR/models/models.csv"
build_script=""
direct_model_name=""
has_input_file=0
has_build_type=0

usage() {
	cat <<'EOF'
Usage:
  allexecute.sh [-i csvFile] [-m buildType] [-d modelName]

buildType:
  ReleaseBuild
  ReleaseInstall
  ReleaseBuildExecute
  DebugBuild
  DebugInstall
  DebugBuildExecute
  UnInstall
EOF
}

while [ "$#" -gt 0 ]; do
	case "$1" in
		-i)
			csv_file="${2:-}"
			has_input_file=1
			shift 2
			;;
		-m)
			case "${2:-}" in
				ReleaseBuild) build_script="$BUILD_BATCH_DIR/subBatch/rbuild.sh" ;;
				ReleaseInstall) build_script="$BUILD_BATCH_DIR/subBatch/rinstall.sh" ;;
				ReleaseBuildExecute) build_script="$BUILD_BATCH_DIR/subBatch/rbuildexec.sh" ;;
				DebugBuild) build_script="$BUILD_BATCH_DIR/subBatch/dbuild.sh" ;;
				DebugInstall) build_script="$BUILD_BATCH_DIR/subBatch/dinstall.sh" ;;
				DebugBuildExecute) build_script="$BUILD_BATCH_DIR/subBatch/dbuildexec.sh" ;;
				UnInstall) build_script="$BUILD_BATCH_DIR/subBatch/uninstall.sh" ;;
				*)
					printf 'Error: unsupported build type: %s\n' "${2:-}" >&2
					exit 1
					;;
			esac
			has_build_type=1
			shift 2
			;;
		-d)
			direct_model_name="${2:-}"
			has_input_file=1
			shift 2
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			shift
			;;
	esac
done

if [ "$has_input_file" -ne 1 ] || [ "$has_build_type" -ne 1 ]; then
	usage >&2
	exit 1
fi

if [ -n "$direct_model_name" ]; then
	logfile="$(log_file_path "$direct_model_name" "$(basename "$build_script")")"
	printf '*********** Direct Model [%s] [%s] ***********\n' "$direct_model_name" "$(basename "$build_script")"
	printf 'Log file: %s\n' "$logfile"
	run_logged "$logfile" "$build_script" "$direct_model_name"
	printf 'Build process completed.\n'
	exit 0
fi

if [ ! -f "$csv_file" ]; then
	printf 'Error: CSV file not found: %s\n' "$csv_file" >&2
	exit 1
fi

while IFS=, read -r model variant; do
	model="$(trim_csv_field "$model")"
	variant="$(trim_csv_field "$variant")"
	if [ -z "$model" ] || [ -z "$variant" ]; then
		continue
	fi
	logfile="$(log_file_path "$variant" "$(basename "$build_script")")"
	printf '***********  %s [%s] ***********\n' "$model" "$(basename "$build_script")"
	printf 'Log file: %s\n' "$logfile"
	run_logged "$logfile" "$build_script" "$variant"
done < "$csv_file"

printf 'Build process completed.\n'
