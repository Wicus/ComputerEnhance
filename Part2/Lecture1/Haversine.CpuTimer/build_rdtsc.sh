#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"
g++ -shared -fPIC rdtsc.cpp -o librdtsc.so
echo "Built librdtsc.so"
