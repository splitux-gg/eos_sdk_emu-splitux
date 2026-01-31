#!/bin/bash
# Test bidirectional session discovery - true local co-op simulation
# Both instances host AND discover each other's sessions

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="$PROJECT_DIR/build"
MOCK_GAME="$BUILD_DIR/mock-game"
LOG_DIR="$PROJECT_DIR/test-bench/logs"

mkdir -p "$LOG_DIR"

echo "=== EOS-LAN Bidirectional Discovery Test ==="
echo "Project: $PROJECT_DIR"
echo ""

# Build if needed
if [ ! -f "$MOCK_GAME" ]; then
    echo "Building mock game..."
    cd "$PROJECT_DIR" && cmake -B build && cmake --build build
fi

cleanup() {
    echo "Cleaning up..."
    kill $PLAYER1_PID 2>/dev/null || true
    kill $PLAYER2_PID 2>/dev/null || true
}
trap cleanup EXIT

# Enable localhost mode for Wine/Proton compatibility
export EOSLAN_LOCALHOST_MODE=1

# Start Player 1 (hosts "Player1Game", discovers others)
echo "--- Starting Player 1 (hosting 'Player1Game') ---"
stdbuf -oL -eL $MOCK_GAME --both --name "Player1Game" --verbose 2 > "$LOG_DIR/player1.log" 2>&1 &
PLAYER1_PID=$!
echo "Player 1 PID: $PLAYER1_PID"

# Wait for Player 1 to initialize
sleep 2

# Start Player 2 (hosts "Player2Game", discovers others)
echo "--- Starting Player 2 (hosting 'Player2Game') ---"
stdbuf -oL -eL $MOCK_GAME --both --name "Player2Game" --verbose 2 > "$LOG_DIR/player2.log" 2>&1 &
PLAYER2_PID=$!
echo "Player 2 PID: $PLAYER2_PID"

# Wait for discovery to happen
echo "--- Waiting for mutual discovery (8 seconds) ---"
sleep 8

# Check results
echo ""
echo "--- Checking results ---"

PASS=true

# Check if Player 1 discovered Player 2's session
if grep -q "DISCOVERED other session" "$LOG_DIR/player1.log"; then
    echo "PASS: Player 1 discovered other session(s)"
else
    echo "FAIL: Player 1 did not discover any sessions"
    PASS=false
fi

# Check if Player 2 discovered Player 1's session
if grep -q "DISCOVERED other session" "$LOG_DIR/player2.log"; then
    echo "PASS: Player 2 discovered other session(s)"
else
    echo "FAIL: Player 2 did not discover any sessions"
    PASS=false
fi

# Show discovery details
echo ""
echo "--- Discovery Details ---"
echo "Player 1 discoveries:"
grep "DISCOVERED\|Status:" "$LOG_DIR/player1.log" 2>/dev/null | head -5 || echo "  (none)"
echo ""
echo "Player 2 discoveries:"
grep "DISCOVERED\|Status:" "$LOG_DIR/player2.log" 2>/dev/null | head -5 || echo "  (none)"

echo ""
if [ "$PASS" = true ]; then
    echo "=== BIDIRECTIONAL DISCOVERY TEST PASSED ==="
    echo "Both players can host AND discover each other's sessions!"
else
    echo "=== BIDIRECTIONAL DISCOVERY TEST FAILED ==="
    echo ""
    echo "Player 1 log:"
    cat "$LOG_DIR/player1.log"
    echo ""
    echo "Player 2 log:"
    cat "$LOG_DIR/player2.log"
    exit 1
fi

echo "Logs available in: $LOG_DIR"
