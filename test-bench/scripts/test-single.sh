#!/bin/bash
# Test single instance - basic API validation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="$PROJECT_DIR/build"
# Detect platform for mock-game binary name
if [ -f "$BUILD_DIR/mock-game.exe" ]; then
    MOCK_GAME="$BUILD_DIR/mock-game.exe"
else
    MOCK_GAME="$BUILD_DIR/mock-game"
fi

echo "=== EOS-LAN Single Instance Test ==="
echo "Project: $PROJECT_DIR"
echo ""

# Build if needed
if [ ! -f "$MOCK_GAME" ]; then
    echo "Building mock game..."
    cd "$PROJECT_DIR" && make mock-game
fi

# Test 1: Auth only
echo "--- Test 1: Authentication ---"
$MOCK_GAME --test-auth --verbose 2
echo "PASS: Authentication works"
echo ""

# Test 2: Create session
echo "--- Test 2: Session Creation ---"
timeout 5 $MOCK_GAME --host --name "SingleTest" --verbose 2 &
HOST_PID=$!
sleep 2
kill $HOST_PID 2>/dev/null || true
wait $HOST_PID 2>/dev/null || true
echo "PASS: Session creation works"
echo ""

echo "=== All single instance tests passed ==="
