#!/usr/bin/env sh
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
TARGET="${AIRTRACE_TARGET:-AirTrace}"
CONFIG_PATH="${AIRTRACE_CONFIG:-configs/sim_default.cfg}"

case "$BUILD_DIR" in
  /*) ;;
  *) BUILD_DIR="$ROOT_DIR/$BUILD_DIR" ;;
esac

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
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
