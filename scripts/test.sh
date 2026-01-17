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

case "$BUILD_DIR" in
  /*) ;;
  *) BUILD_DIR="$ROOT_DIR/$BUILD_DIR" ;;
esac

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" $GENERATOR_ARGS
cmake --build "$BUILD_DIR" --target AirTraceCoreTests

cd "$BUILD_DIR"
ctest --output-on-failure
