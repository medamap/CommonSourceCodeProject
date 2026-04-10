#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../common.sh"

variant="${1:-}"
if [ -z "$variant" ]; then
	printf 'Usage: %s <variant>\n' "${0##*/}" >&2
	exit 1
fi

uninstall_app "$variant"
