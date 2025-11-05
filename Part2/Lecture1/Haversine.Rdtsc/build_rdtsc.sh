#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

SRC=rdtsc.cpp
OUT_SO=librdtsc.so

echo "[build_rdtsc] compiling $SRC -> $OUT_SO"

if command -v g++ >/dev/null 2>&1; then
  CXX=g++
elif command -v clang++ >/dev/null 2>&1; then
  CXX=clang++
else
  echo "C++ compiler not found (need g++ or clang++)" >&2
  exit 1
fi

"$CXX" -O3 -fPIC -shared -o "$OUT_SO" "$SRC"

echo "[build_rdtsc] done: $(pwd)/$OUT_SO"

