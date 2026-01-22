#!/usr/bin/env sh
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
TARGET="${AIRTRACE_TARGET:-AirTrace}"
CONFIG_PATH="${AIRTRACE_CONFIG:-configs/sim_default.cfg}"

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

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" $RESET_BUILD_TYPE
cmake --build "$BUILD_DIR"

if [ -x "$BUILD_DIR/$TARGET" ]; then
  EXE="$BUILD_DIR/$TARGET"
elif [ -x "$BUILD_DIR/$BUILD_TYPE/$TARGET" ]; then
  EXE="$BUILD_DIR/$BUILD_TYPE/$TARGET"
elif [ -x "$BUILD_DIR/$BUILD_TYPE/$TARGET.exe" ]; then
  EXE="$BUILD_DIR/$BUILD_TYPE/$TARGET.exe"
elif [ -x "$BUILD_DIR/$TARGET.exe" ]; then
  EXE="$BUILD_DIR/$TARGET.exe"
else
  echo "$TARGET executable not found in $BUILD_DIR" >&2
  exit 1
fi

cd "$ROOT_DIR"
if [ "$TARGET" = "AirTraceSimExample" ]; then
  "$EXE" "$CONFIG_PATH"
else
  "$EXE"
fi
