#!/bin/bash
# Test dual instance on Windows VM via SSH
# Mirrors the Linux test-dual.sh pattern

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
VM_HOST="winvm"
BUILD_DIR="C:/Code/eos-lan/build/Release"
LOG_DIR="C:/Code/eos-lan/test-bench/logs"
LOCAL_LOG_DIR="$PROJECT_DIR/test-bench/logs"

echo "=== EOS-LAN Dual Instance Test (Windows VM) ==="
echo "VM: $VM_HOST"
echo "Build: $BUILD_DIR"
echo ""

# Create local logs directory
mkdir -p "$LOCAL_LOG_DIR"

# Cleanup function
cleanup() {
    echo ""
    echo "--- Cleanup ---"
    echo "Stopping mock-game processes on Windows VM..."
    ssh "$VM_HOST" 'taskkill /F /IM mock-game.exe 2>nul || exit 0' || true

    # Wait a moment for logs to flush
    sleep 1

    # Retrieve logs from Windows
    echo "Retrieving logs..."
    ssh "$VM_HOST" "type \"$LOG_DIR\\host.log\"" > "$LOCAL_LOG_DIR/host-windows.log" 2>/dev/null || echo "(No host log)"
    ssh "$VM_HOST" "type \"$LOG_DIR\\client.log\"" > "$LOCAL_LOG_DIR/client-windows.log" 2>/dev/null || echo "(No client log)"
}
trap cleanup EXIT

# Create log directory on Windows
ssh "$VM_HOST" "powershell -Command \"New-Item -ItemType Directory -Force -Path '$LOG_DIR' | Out-Null\""

# Start host in background via SSH
# Note: We keep the SSH connection open to keep the process alive
echo "--- Starting Host ---"
ssh "$VM_HOST" "cd $BUILD_DIR && mock-game.exe --host --name DualTestWin --verbose 2 > \"$LOG_DIR\\host.log\" 2>&1" &
HOST_SSH_PID=$!
echo "Host SSH PID: $HOST_SSH_PID"

# Wait for host to initialize
echo "Waiting 3 seconds for host initialization..."
sleep 3

# Verify host is running on Windows
HOST_COUNT=$(ssh "$VM_HOST" 'tasklist /FI "IMAGENAME eq mock-game.exe" /FO CSV /NH | find /C "mock-game.exe"' || echo "0")
if [ "$HOST_COUNT" = "0" ]; then
    echo "ERROR: Host process not running on Windows VM"
    exit 1
fi
echo "Host is running on Windows (found $HOST_COUNT instance)"

# Start client in background via SSH
echo ""
echo "--- Starting Client ---"
ssh "$VM_HOST" "cd $BUILD_DIR && mock-game.exe --join --verbose 2 > \"$LOG_DIR\\client.log\" 2>&1" &
CLIENT_SSH_PID=$!
echo "Client SSH PID: $CLIENT_SSH_PID"

# Wait for client to search and join
echo "Waiting 8 seconds for client to search..."
sleep 8

# Check results
echo ""
echo "--- Checking Results ---"

# Kill both SSH sessions (which will kill the processes)
kill $HOST_SSH_PID $CLIENT_SSH_PID 2>/dev/null || true
wait $HOST_SSH_PID $CLIENT_SSH_PID 2>/dev/null || true

# Cleanup will retrieve logs (via trap)
# Wait a moment for cleanup to finish
sleep 2

# Check logs for LAN discovery evidence
echo "Checking logs for LAN discovery..."

# Check if host broadcasted sessions
HOST_BROADCAST_COUNT=$(grep -c "Broadcasted session:" "$LOCAL_LOG_DIR/host-windows.log" 2>/dev/null || echo "0")
# Check if client discovered sessions in cache
CLIENT_DISCOVERED_COUNT=$(grep -c "Found 1 discovered sessions in cache" "$LOCAL_LOG_DIR/client-windows.log" 2>/dev/null || echo "0")

if [ "$HOST_BROADCAST_COUNT" -gt "0" ] && [ "$CLIENT_DISCOVERED_COUNT" -gt "0" ]; then
    echo ""
    echo "=== PASS: LAN Discovery Working ==="
    echo "  Host broadcasted session $HOST_BROADCAST_COUNT times"
    echo "  Client discovered sessions $CLIENT_DISCOVERED_COUNT times"
    echo ""
    echo "This confirms:"
    echo "  - Host successfully creates and broadcasts sessions via UDP"
    echo "  - Client successfully receives broadcasts and caches discovered sessions"
    echo ""
    echo "Logs available at:"
    echo "  Host: $LOCAL_LOG_DIR/host-windows.log"
    echo "  Client: $LOCAL_LOG_DIR/client-windows.log"
    exit 0
fi

# Test failed
echo ""
echo "=== FAIL: LAN Discovery not working ==="
echo "  Host broadcasts: $HOST_BROADCAST_COUNT"
echo "  Client discoveries: $CLIENT_DISCOVERED_COUNT"
echo ""
echo "Host log (first 30 lines):"
head -30 "$LOCAL_LOG_DIR/host-windows.log" 2>/dev/null || echo "(No host log)"
echo ""
echo "Client log (first 30 lines):"
head -30 "$LOCAL_LOG_DIR/client-windows.log" 2>/dev/null || echo "(No client log)"
echo ""
echo "Full logs available at:"
echo "  Host: $LOCAL_LOG_DIR/host-windows.log"
echo "  Client: $LOCAL_LOG_DIR/client-windows.log"
exit 1
