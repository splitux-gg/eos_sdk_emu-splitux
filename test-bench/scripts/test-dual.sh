#!/bin/bash
# Test dual instance - LAN discovery and join

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
LOG_DIR="$PROJECT_DIR/test-bench/logs"

mkdir -p "$LOG_DIR"

echo "=== EOS-LAN Dual Instance Test ==="
echo "Project: $PROJECT_DIR"
echo ""

# Build if needed
if [ ! -f "$MOCK_GAME" ]; then
    echo "Building mock game..."
    cd "$PROJECT_DIR" && make mock-game
fi

cleanup() {
    echo "Cleaning up..."
    kill $HOST_PID 2>/dev/null || true
    kill $CLIENT_PID 2>/dev/null || true
}
trap cleanup EXIT

# Start host (use stdbuf for unbuffered output)
echo "--- Starting host ---"
stdbuf -oL -eL $MOCK_GAME --host --name "DualTest" --verbose 2 > "$LOG_DIR/host.log" 2>&1 &
HOST_PID=$!
echo "Host PID: $HOST_PID"

# Wait for host to initialize
sleep 3

# Start client (use stdbuf for unbuffered output)
echo "--- Starting client ---"
stdbuf -oL -eL $MOCK_GAME --join --verbose 2 > "$LOG_DIR/client.log" 2>&1 &
CLIENT_PID=$!
echo "Client PID: $CLIENT_PID"

# Wait for client to find session
sleep 5

# Check results
echo ""
echo "--- Checking results ---"

if grep -q "Found 1 sessions" "$LOG_DIR/client.log"; then
    echo "PASS: Client found host's session"
else
    echo "FAIL: Client did not find session"
    echo "Host log:"
    cat "$LOG_DIR/host.log"
    echo ""
    echo "Client log:"
    cat "$LOG_DIR/client.log"
    exit 1
fi

echo ""
echo "=== Dual instance test passed ==="
echo "Logs available in: $LOG_DIR"
