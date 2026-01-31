#!/bin/bash
# EOS-LAN Environment Variable Integration Tests
# Tests all 5 environment variables with valid/invalid values, defaults, and isolation

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
LOG_DIR="$PROJECT_DIR/test-bench/logs/env-tests"

# Track test results
TESTS_PASSED=0
TESTS_FAILED=0

echo "=========================================="
echo "EOS-LAN Environment Variable Tests"
echo "=========================================="
echo "Project: $PROJECT_DIR"
echo ""

# Create log directory
mkdir -p "$LOG_DIR"

# Build if needed
if [ ! -f "$MOCK_GAME" ]; then
    echo "Building mock game..."
    cd "$PROJECT_DIR" && make mock-game
    echo ""
fi

# Cleanup function
cleanup() {
    # Kill any lingering mock-game processes
    pkill -f mock-game 2>/dev/null || true
}
trap cleanup EXIT

# ==============================================================================
# VALID VALUE TESTS
# ==============================================================================

echo "===== VALID VALUE TESTS ====="
echo ""

# Test 1: Valid EOSLAN_DISCOVERY_PORT
echo "--- Test 1: Valid EOSLAN_DISCOVERY_PORT ---"
EOSLAN_DISCOVERY_PORT=12345 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test1.log" 2>&1

if grep -q "Using discovery port from env: 12345" "$LOG_DIR/test1.log" && \
   grep -q "port=12345" "$LOG_DIR/test1.log"; then
    echo "PASS: Valid EOSLAN_DISCOVERY_PORT"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Valid EOSLAN_DISCOVERY_PORT"
    cat "$LOG_DIR/test1.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 2: Valid EOSLAN_BROADCAST_ADDR
echo "--- Test 2: Valid EOSLAN_BROADCAST_ADDR ---"
EOSLAN_BROADCAST_ADDR=192.168.1.255 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test2.log" 2>&1

if grep -q "Using broadcast address from env: 192.168.1.255" "$LOG_DIR/test2.log" && \
   grep -q "broadcast=192.168.1.255" "$LOG_DIR/test2.log"; then
    echo "PASS: Valid EOSLAN_BROADCAST_ADDR"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Valid EOSLAN_BROADCAST_ADDR"
    cat "$LOG_DIR/test2.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 3: Valid EOSLAN_ANNOUNCE_INTERVAL
echo "--- Test 3: Valid EOSLAN_ANNOUNCE_INTERVAL ---"
EOSLAN_ANNOUNCE_INTERVAL=1000 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test3.log" 2>&1

if grep -q "Using announcement interval from env: 1000ms" "$LOG_DIR/test3.log" && \
   grep -q "interval=1000ms" "$LOG_DIR/test3.log"; then
    echo "PASS: Valid EOSLAN_ANNOUNCE_INTERVAL"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Valid EOSLAN_ANNOUNCE_INTERVAL"
    cat "$LOG_DIR/test3.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 4: Valid EOSLAN_PREFERRED_IP
echo "--- Test 4: Valid EOSLAN_PREFERRED_IP ---"
EOSLAN_PREFERRED_IP=192.168.1.100 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test4.log" 2>&1

if grep -q "Using preferred IP from env: 192.168.1.100" "$LOG_DIR/test4.log"; then
    echo "PASS: Valid EOSLAN_PREFERRED_IP"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Valid EOSLAN_PREFERRED_IP"
    cat "$LOG_DIR/test4.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 5: Valid EOSLAN_DEBUG
echo "--- Test 5: Valid EOSLAN_DEBUG ---"
EOSLAN_DEBUG=1 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test5.log" 2>&1

if grep -q "LAN debug logging enabled via env" "$LOG_DIR/test5.log"; then
    echo "PASS: Valid EOSLAN_DEBUG"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Valid EOSLAN_DEBUG"
    cat "$LOG_DIR/test5.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo ""

# ==============================================================================
# INVALID VALUE TESTS
# ==============================================================================

echo "===== INVALID VALUE TESTS ====="
echo ""

# Test 6: Invalid port (below range)
echo "--- Test 6: Invalid EOSLAN_DISCOVERY_PORT (below 1024) ---"
EOSLAN_DISCOVERY_PORT=999 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test6.log" 2>&1

if grep -q "Invalid EOSLAN_DISCOVERY_PORT: 999" "$LOG_DIR/test6.log" && \
   grep -q "port=23456" "$LOG_DIR/test6.log"; then
    echo "PASS: Invalid EOSLAN_DISCOVERY_PORT (below 1024)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Invalid EOSLAN_DISCOVERY_PORT (below 1024)"
    cat "$LOG_DIR/test6.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 7: Invalid port (above range)
echo "--- Test 7: Invalid EOSLAN_DISCOVERY_PORT (above 65535) ---"
EOSLAN_DISCOVERY_PORT=70000 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test7.log" 2>&1

if grep -q "Invalid EOSLAN_DISCOVERY_PORT: 70000" "$LOG_DIR/test7.log" && \
   grep -q "port=23456" "$LOG_DIR/test7.log"; then
    echo "PASS: Invalid EOSLAN_DISCOVERY_PORT (above 65535)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Invalid EOSLAN_DISCOVERY_PORT (above 65535)"
    cat "$LOG_DIR/test7.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 8: Invalid broadcast address (too long)
echo "--- Test 8: Invalid EOSLAN_BROADCAST_ADDR (too long) ---"
EOSLAN_BROADCAST_ADDR=192.168.255.255.255 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test8.log" 2>&1

if grep -q "Invalid EOSLAN_BROADCAST_ADDR: too long" "$LOG_DIR/test8.log" && \
   grep -q "broadcast=255.255.255.255" "$LOG_DIR/test8.log"; then
    echo "PASS: Invalid EOSLAN_BROADCAST_ADDR (too long)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Invalid EOSLAN_BROADCAST_ADDR (too long)"
    cat "$LOG_DIR/test8.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 9: Invalid interval (below range)
echo "--- Test 9: Invalid EOSLAN_ANNOUNCE_INTERVAL (below 500) ---"
EOSLAN_ANNOUNCE_INTERVAL=100 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test9.log" 2>&1

if grep -q "Invalid EOSLAN_ANNOUNCE_INTERVAL: 100" "$LOG_DIR/test9.log" && \
   grep -q "interval=2000ms" "$LOG_DIR/test9.log"; then
    echo "PASS: Invalid EOSLAN_ANNOUNCE_INTERVAL (below 500)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Invalid EOSLAN_ANNOUNCE_INTERVAL (below 500)"
    cat "$LOG_DIR/test9.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

# Test 10: Invalid interval (above range)
echo "--- Test 10: Invalid EOSLAN_ANNOUNCE_INTERVAL (above 10000) ---"
EOSLAN_ANNOUNCE_INTERVAL=15000 $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test10.log" 2>&1

if grep -q "Invalid EOSLAN_ANNOUNCE_INTERVAL: 15000" "$LOG_DIR/test10.log" && \
   grep -q "interval=2000ms" "$LOG_DIR/test10.log"; then
    echo "PASS: Invalid EOSLAN_ANNOUNCE_INTERVAL (above 10000)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Invalid EOSLAN_ANNOUNCE_INTERVAL (above 10000)"
    cat "$LOG_DIR/test10.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo ""

# ==============================================================================
# DEFAULT VALUE TEST
# ==============================================================================

echo "===== DEFAULT VALUE TEST ====="
echo ""

# Test 11: Default values
echo "--- Test 11: Default values (no env vars) ---"
env -u EOSLAN_DISCOVERY_PORT \
    -u EOSLAN_BROADCAST_ADDR \
    -u EOSLAN_ANNOUNCE_INTERVAL \
    -u EOSLAN_PREFERRED_IP \
    -u EOSLAN_DEBUG \
    $MOCK_GAME --test-auth --verbose 2 > "$LOG_DIR/test11.log" 2>&1

if grep -q "port=23456" "$LOG_DIR/test11.log" && \
   grep -q "broadcast=255.255.255.255" "$LOG_DIR/test11.log" && \
   grep -q "interval=2000ms" "$LOG_DIR/test11.log" && \
   ! grep -q "Using discovery port from env" "$LOG_DIR/test11.log"; then
    echo "PASS: Default values (no env vars)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Default values (no env vars)"
    cat "$LOG_DIR/test11.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo ""

# ==============================================================================
# ISOLATION TEST
# ==============================================================================

echo "===== ISOLATION TEST ====="
echo ""

echo "--- Test 12: Isolation (different ports) ---"

# Start host on port 24000
EOSLAN_DISCOVERY_PORT=24000 $MOCK_GAME --host --name "IsolationTest" --verbose 2 > "$LOG_DIR/host-iso.log" 2>&1 &
HOST_PID=$!
echo "  Started host on port 24000 (PID: $HOST_PID)"
sleep 3

# Start client on port 25000 (different port)
EOSLAN_DISCOVERY_PORT=25000 timeout 8 $MOCK_GAME --join --verbose 2 > "$LOG_DIR/client-iso.log" 2>&1 &
CLIENT_PID=$!
echo "  Started client on port 25000 (PID: $CLIENT_PID)"
sleep 6

# Kill both
kill $HOST_PID $CLIENT_PID 2>/dev/null || true
wait $HOST_PID $CLIENT_PID 2>/dev/null || true

# Verify
ISO_PASS=true

if grep -q "port=24000" "$LOG_DIR/host-iso.log"; then
    echo "  Host port 24000: OK"
else
    echo "  Host port 24000: FAIL"
    ISO_PASS=false
fi

if grep -q "port=25000" "$LOG_DIR/client-iso.log"; then
    echo "  Client port 25000: OK"
else
    echo "  Client port 25000: FAIL"
    ISO_PASS=false
fi

if ! grep -q "Found 1 sessions" "$LOG_DIR/client-iso.log"; then
    echo "  Client isolation: OK (no discovery)"
else
    echo "  Client isolation: FAIL (should not discover)"
    ISO_PASS=false
fi

if [ "$ISO_PASS" = true ]; then
    echo "PASS: Isolation (different ports)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo "FAIL: Isolation (different ports)"
    echo "Host log:"
    cat "$LOG_DIR/host-iso.log"
    echo ""
    echo "Client log:"
    cat "$LOG_DIR/client-iso.log"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo ""

# ==============================================================================
# SUMMARY
# ==============================================================================

echo "=========================================="
echo "TEST SUMMARY"
echo "=========================================="
echo "Total: $((TESTS_PASSED + TESTS_FAILED))"
echo "Passed: $TESTS_PASSED"
echo "Failed: $TESTS_FAILED"
echo ""
echo "Logs: $LOG_DIR"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo "All tests PASSED!"
    exit 0
else
    echo "Some tests FAILED!"
    exit 1
fi
