#!/bin/bash
# Compare behavior against real EOS SDK

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
LOG_DIR="$PROJECT_DIR/test-bench/logs"
REAL_SDK="$PROJECT_DIR/SDK/Bin/EOSSDK-Win64-Shipping.dll"
OUR_SDK="$PROJECT_DIR/build/Release/EOSSDK-Win64-Shipping.dll"

echo "=== EOS-LAN vs Real SDK Comparison ==="

# Test with our SDK
echo "Testing with EOS-LAN..."
PATH="$PROJECT_DIR/build/Release:$PATH" $PROJECT_DIR/build/mock-game.exe --test-auth --verbose 2 > "$LOG_DIR/our-sdk.log" 2>&1

# Note: Real SDK will fail without real credentials, but we can compare API call patterns
echo "Testing with real SDK (will fail auth, but useful for call pattern comparison)..."
PATH="$PROJECT_DIR/SDK/Bin:$PATH" $PROJECT_DIR/build/mock-game.exe --test-auth --verbose 2 > "$LOG_DIR/real-sdk.log" 2>&1 || true

echo ""
echo "Comparison complete. Check logs in: $LOG_DIR"
echo "  - our-sdk.log: Our implementation"
echo "  - real-sdk.log: Real EOS SDK"
