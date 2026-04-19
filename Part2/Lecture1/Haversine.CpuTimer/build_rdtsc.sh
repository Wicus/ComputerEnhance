#!/usr/bin/env bash
# Builds rdtsc.c into librdtsc.so for Linux (x86-64).
# -shared: produce a shared object instead of an executable
# -fPIC:   position-independent code, required for shared libraries on Linux
set -euo pipefail

cd "$(dirname "$0")"
gcc -shared -fPIC rdtsc.c -o librdtsc.so
echo "Built librdtsc.so"
