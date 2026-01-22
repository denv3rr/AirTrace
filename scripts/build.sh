#!/usr/bin/env sh
set -e

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"

case "$BUILD_TYPE" in
  Debug|Release|RelWithDebInfo|MinSizeRel) ;;
  *)
    echo "Invalid BUILD_TYPE '$BUILD_TYPE'. Allowed: Debug, Release, RelWithDebInfo, MinSizeRel." >&2
    exit 1
    ;;
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

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" $RESET_BUILD_TYPE
cmake --build "$BUILD_DIR"
