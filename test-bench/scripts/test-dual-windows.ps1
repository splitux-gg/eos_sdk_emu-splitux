# EOS-LAN Dual Instance Test for Windows
# Uses Windows Task Scheduler (schtasks) for background process management
# This script is designed to run ON the Windows VM

param(
    [string]$BuildDir = "C:\Code\eos-lan\build\Release",
    [string]$SessionName = "DualTestWin"
)

# Configuration
$MOCK_GAME = "$BuildDir\mock-game.exe"
$LOG_DIR = "C:\Code\eos-lan\test-bench\logs"
$HOST_LOG = "$LOG_DIR\host.log"
$CLIENT_LOG = "$LOG_DIR\client.log"
$TASK_NAME = "EOS-Test-Host-Dual"
$WRAPPER_BAT = "$env:TEMP\eos-host-wrapper.bat"

Write-Host "=== EOS-LAN Dual Instance Test (Windows) ===" -ForegroundColor Cyan
Write-Host "Build directory: $BuildDir"
Write-Host "Session name: $SessionName"
Write-Host ""

# Step 1: Verify mock-game.exe exists
if (-not (Test-Path $MOCK_GAME)) {
    Write-Host "ERROR: mock-game.exe not found at $MOCK_GAME" -ForegroundColor Red
    Write-Host "Please build the project first: make deploy-vm" -ForegroundColor Yellow
    exit 1
}

# Step 2: Create logs directory
Write-Host "--- Setup ---"
New-Item -ItemType Directory -Force -Path $LOG_DIR | Out-Null
Write-Host "Log directory: $LOG_DIR"

# Step 3: Cleanup any existing tasks and processes
Write-Host "Cleaning up existing tasks and processes..."
schtasks /Delete /TN $TASK_NAME /F 2>$null | Out-Null
Get-Process -Name "mock-game" -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

# Step 4: Create batch file wrapper for host (handles I/O redirection)
# schtasks doesn't support I/O redirection, so we need a batch file
Write-Host "Creating batch file wrapper..."
$wrapperContent = @"
@echo off
cd /d $BuildDir
"$MOCK_GAME" --host --name "$SessionName" --verbose 2 > "$HOST_LOG" 2>&1
"@
$wrapperContent | Out-File -FilePath $WRAPPER_BAT -Encoding ASCII -Force

# Step 5: Create scheduled task to run host
# Task runs once at a specific time (2 seconds from now)
Write-Host ""
Write-Host "--- Starting Host ---"
$startTime = (Get-Date).AddSeconds(2).ToString("HH:mm:ss")
Write-Host "Creating scheduled task (start time: $startTime)..."

$createResult = schtasks /Create /TN $TASK_NAME /TR "`"$WRAPPER_BAT`"" /SC ONCE /ST $startTime /F 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to create scheduled task" -ForegroundColor Red
    Write-Host $createResult
    exit 1
}

# Step 6: Run the scheduled task immediately
Write-Host "Starting scheduled task..."
$runResult = schtasks /Run /TN $TASK_NAME 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to run scheduled task" -ForegroundColor Red
    Write-Host $runResult
    schtasks /Delete /TN $TASK_NAME /F 2>$null | Out-Null
    exit 1
}

Write-Host "Host started via scheduled task" -ForegroundColor Green

# Step 7: Wait for host to initialize
Write-Host "Waiting 3 seconds for host initialization..."
Start-Sleep -Seconds 3

# Step 8: Verify host is running
$hostProcess = Get-Process -Name "mock-game" -ErrorAction SilentlyContinue
if (-not $hostProcess) {
    Write-Host "ERROR: Host process failed to start" -ForegroundColor Red
    Write-Host ""
    Write-Host "Host log:"
    if (Test-Path $HOST_LOG) {
        Get-Content $HOST_LOG
    } else {
        Write-Host "(No log file created)"
    }
    schtasks /Delete /TN $TASK_NAME /F 2>$null | Out-Null
    exit 1
}

Write-Host "Host process running (PID: $($hostProcess.Id))" -ForegroundColor Green

# Step 9: Run client in background
Write-Host ""
Write-Host "--- Starting Client ---"
Write-Host "Starting client in background..."

# Create batch file wrapper for client (same reason as host)
$clientWrapperPath = "$env:TEMP\eos-client-wrapper.bat"
$clientWrapperContent = @"
@echo off
cd /d $BuildDir
"$MOCK_GAME" --join --verbose 2 > "$CLIENT_LOG" 2>&1
"@
$clientWrapperContent | Out-File -FilePath $clientWrapperPath -Encoding ASCII -Force

# Start client using Start-Process
$clientProcess = Start-Process -FilePath $clientWrapperPath `
    -NoNewWindow `
    -PassThru

Write-Host "Client started (PID: $($clientProcess.Id))"

# Step 10: Wait for client to search and attempt to join (8 seconds total)
# Client does up to 5 search attempts with 2 second delays = ~10 seconds max
Write-Host "Waiting 8 seconds for client to search and join..."
Start-Sleep -Seconds 8

# Step 11: Cleanup
Write-Host ""
Write-Host "--- Cleanup ---"
Write-Host "Stopping mock-game processes..."
Get-Process -Name "mock-game" -ErrorAction SilentlyContinue | Stop-Process -Force

Write-Host "Deleting scheduled task..."
schtasks /Delete /TN $TASK_NAME /F 2>$null | Out-Null

Write-Host "Removing wrapper batch files..."
Remove-Item $WRAPPER_BAT -ErrorAction SilentlyContinue
Remove-Item "$env:TEMP\eos-client-wrapper.bat" -ErrorAction SilentlyContinue

# Wait for log files to be fully written
Start-Sleep -Milliseconds 500

# Step 12: Check results
Write-Host ""
Write-Host "=== Test Results ===" -ForegroundColor Cyan

# Check if client log exists
if (-not (Test-Path $CLIENT_LOG)) {
    Write-Host "FAIL: Client log not found" -ForegroundColor Red
    exit 1
}

# Read client log and check for session discovery
$clientContent = Get-Content $CLIENT_LOG -Raw

# The mock-game client prints "Found N sessions" when it successfully searches
# It also prints "Joined session successfully" when it joins
if ($clientContent -match "Found (\d+) sessions") {
    $sessionCount = $matches[1]
    if ($sessionCount -ge "1") {
        Write-Host "PASS: Client found $sessionCount session(s)" -ForegroundColor Green

        # Check if client also joined
        if ($clientContent -match "Joined session successfully") {
            Write-Host "PASS: Client successfully joined the session" -ForegroundColor Green
        }

        Write-Host ""
        Write-Host "Host log: $HOST_LOG"
        Write-Host "Client log: $CLIENT_LOG"
        exit 0
    } else {
        Write-Host "FAIL: Client found $sessionCount sessions (expected >= 1)" -ForegroundColor Red
    }
} else {
    Write-Host "FAIL: Client did not find any sessions" -ForegroundColor Red
}

# On failure, display logs for debugging
Write-Host ""
Write-Host "--- Host Log ---" -ForegroundColor Yellow
if (Test-Path $HOST_LOG) {
    Get-Content $HOST_LOG
} else {
    Write-Host "(No host log)"
}

Write-Host ""
Write-Host "--- Client Log ---" -ForegroundColor Yellow
Get-Content $CLIENT_LOG

exit 1
