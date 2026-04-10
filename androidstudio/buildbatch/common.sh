#!/usr/bin/env bash
set -euo pipefail

BUILD_BATCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_STUDIO_DIR="$(cd "$BUILD_BATCH_DIR/.." && pwd)"

sanitize_name() {
	printf '%s' "$1" | tr '[:upper:]' '[:lower:]' | tr ' /:' '___' | tr -cd '[:alnum:]_.-'
}

trim_csv_field() {
	local value="$1"
	value="${value%$'\r'}"
	value="${value#"${value%%[![:space:]]*}"}"
	value="${value%"${value##*[![:space:]]}"}"
	printf '%s' "$value"
}

build_type_name() {
	local variant="$1"
	local mode="$2"
	local first rest
	first="$(printf '%s' "$variant" | cut -c1 | tr '[:lower:]' '[:upper:]')"
	rest="${variant#?}"
	printf '%s%s%s' "$first" "$rest" "$mode"
}

apk_output_dir() {
	local variant="$1"
	local kind="$2"
	printf '%s/app/build/outputs/apk/%s/%s' "$ANDROID_STUDIO_DIR" "$variant" "$kind"
}

archive_dir_name() {
	local kind="$1"
	local date_str
	date_str="$(date +%Y%m%d)"
	printf 'v%s_%s_apk' "$date_str" "$kind"
}

build_log_dir() {
	printf '%s/logs' "$BUILD_BATCH_DIR"
}

log_file_path() {
	local model_name="$1"
	local build_name="$2"
	local stamp
	stamp="$(date +%Y%m%d_%H%M%S)"
	printf '%s/%s_%s_%s.log' "$(build_log_dir)" "$stamp" "$(sanitize_name "$model_name")" "$(sanitize_name "$build_name")"
}

run_logged() {
	local logfile="$1"
	shift
	mkdir -p "$(dirname "$logfile")"
	"$@" </dev/null 2>&1 | tee -a "$logfile"
}

archive_apks() {
	local variant="$1"
	local kind="$2"
	local src_dir dest_dir
	local files=()

	src_dir="$(apk_output_dir "$variant" "$kind")"
	dest_dir="$BUILD_BATCH_DIR/$(archive_dir_name "$kind")"
	mkdir -p "$dest_dir"

	shopt -s nullglob
	files=("$src_dir"/*.apk)
	shopt -u nullglob

	if [ "${#files[@]}" -eq 0 ]; then
		printf 'No APK files found: %s\n' "$src_dir" >&2
		return 1
	fi

	cp -f "${files[@]}" "$dest_dir"/
}

run_gradle() {
	local task="$1"
	(
		cd "$ANDROID_STUDIO_DIR"
		bash ./gradlew "$task"
	)
}

start_app() {
	local variant="$1"
	adb shell am start -n "jp.matrix.shikarunochi.emulator.${variant}/jp.matrix.shikarunochi.emulator.EmulatorActivity"
}

uninstall_app() {
	local variant="$1"
	adb uninstall "jp.matrix.shikarunochi.emulator.${variant}"
}
