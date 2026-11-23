#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$DIR/.."

echo "========================================"
echo "  Dune Strike Automated Test Suite"
echo "========================================"

# 1. Compile unit tests
echo "[BUILD] Compiling unit tests..."
clang++ -O2 -Wall "$DIR/test_bombsite.cpp" -o "$DIR/test_bombsite"
clang++ -O2 -Wall "$DIR/test_bomb_dual_site.cpp" -o "$DIR/test_bomb_dual_site"

# 2. Run unit tests
echo "[RUN] Running test_bombsite..."
"$DIR/test_bombsite"

echo "[RUN] Running test_bomb_dual_site..."
"$DIR/test_bomb_dual_site"

echo ""
echo "========================================"
echo "  ALL TESTS PASSED SUCCESSFULLY!"
echo "========================================"
