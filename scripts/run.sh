#!/usr/bin/env sh
set -e

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR"

if [ -x "$BUILD_DIR/AirTrace" ]; then
  "$BUILD_DIR/AirTrace"
elif [ -x "$BUILD_DIR/$BUILD_TYPE/AirTrace" ]; then
  "$BUILD_DIR/$BUILD_TYPE/AirTrace"
elif [ -x "$BUILD_DIR/$BUILD_TYPE/AirTrace.exe" ]; then
  "$BUILD_DIR/$BUILD_TYPE/AirTrace.exe"
elif [ -x "$BUILD_DIR/AirTrace.exe" ]; then
  "$BUILD_DIR/AirTrace.exe"
else
  echo "AirTrace executable not found in $BUILD_DIR" >&2
  exit 1
fi
