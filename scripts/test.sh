#!/usr/bin/env sh
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

BUILD_DIR="${BUILD_DIR:-}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"

GENERATOR_ARGS=""
if [ -n "$CMAKE_GENERATOR" ]; then
  GENERATOR_ARGS="-G $CMAKE_GENERATOR"
elif command -v ninja >/dev/null 2>&1; then
  CMAKE_GENERATOR="Ninja"
  GENERATOR_ARGS="-G Ninja"
fi

if [ -z "$BUILD_DIR" ]; then
  if [ "$CMAKE_GENERATOR" = "Ninja" ]; then
    BUILD_DIR="$ROOT_DIR/build/tests-ninja"
  else
    BUILD_DIR="$ROOT_DIR/build/tests"
  fi
fi

case "$BUILD_TYPE" in
  Debug|Release|RelWithDebInfo|MinSizeRel) ;;
  *)
    echo "Invalid BUILD_TYPE '$BUILD_TYPE'. Allowed: Debug, Release, RelWithDebInfo, MinSizeRel." >&2
    exit 1
    ;;
esac

case "$BUILD_DIR" in
  /*) ;;
  *) BUILD_DIR="$ROOT_DIR/$BUILD_DIR" ;;
esac

RESET_BUILD_TYPE=""
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
  cached_line=$(grep -m 1 '^CMAKE_BUILD_TYPE:STRING=' "$BUILD_DIR/CMakeCache.txt" 2>/dev/null || true)
  if [ -z "$cached_line" ]; then
    cached_line=$(grep -m 1 '^CMAKE_BUILD_TYPE:UNINITIALIZED=' "$BUILD_DIR/CMakeCache.txt" 2>/dev/null || true)
  fi
  if [ -n "$cached_line" ]; then
    cached_value=${cached_line#*=}
    case "$cached_value" in
      Debug|Release|RelWithDebInfo|MinSizeRel) ;;
      *) RESET_BUILD_TYPE="-U CMAKE_BUILD_TYPE" ;;
    esac
  fi
fi

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" $GENERATOR_ARGS $RESET_BUILD_TYPE
cmake --build "$BUILD_DIR" --target AirTraceCoreTests AirTraceEdgeCaseTests AirTraceUiTests AirTraceHarnessTests AirTraceIntegrationTests

cd "$BUILD_DIR"
temp_output=$(mktemp 2>/dev/null || echo "$BUILD_DIR/ctest_output.log")
trap 'rm -f "$temp_output"' EXIT

set +e
ctest -Q --output-on-failure --output-log "$temp_output"
ctest_status=$?
set -e
sed 's/Total Test time (real)/Total Test time/' "$temp_output"
exit $ctest_status
