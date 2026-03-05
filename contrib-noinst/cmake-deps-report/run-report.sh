#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PYTHON_SCRIPT="${SCRIPT_DIR}/cmake-deps-report.py"

ROOT="."
BUILD_DIR="build"
OUTPUT="cmake-deep-deps-report.md"
VERBOSE=0

print_help() {
  cat <<'EOF'
Usage: run-report.sh [options]

Options:
  -r, --root <path>        CMake project root (default: .)
  -b, --build-dir <path>   Build directory for configured-only mode (default: build)
  -o, --output <path|->    Output Markdown path, or - for stdout (default: cmake-deep-deps-report.md)
  -v, --verbose            Full report (all discovered targets)
  -h, --help               Show this help message

Behavior:
  Default mode generates a configured-only report using --build-dir.
  Verbose mode (-v) generates a full report (no configured-only filtering).
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -r|--root)
      ROOT="$2"
      shift 2
      ;;
    -b|--build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    -o|--output)
      OUTPUT="$2"
      shift 2
      ;;
    -v|--verbose)
      VERBOSE=1
      shift
      ;;
    -h|--help)
      print_help
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      print_help >&2
      exit 2
      ;;
  esac
done

if [[ ! -f "$PYTHON_SCRIPT" ]]; then
  echo "Scanner script not found: $PYTHON_SCRIPT" >&2
  exit 1
fi

if [[ "$VERBOSE" -eq 1 ]]; then
  exec python "$PYTHON_SCRIPT" \
    --root "$ROOT" \
    --output "$OUTPUT"
fi

exec python "$PYTHON_SCRIPT" \
  --root "$ROOT" \
  --build-dir "$BUILD_DIR" \
  --configured-only \
  --output "$OUTPUT"
