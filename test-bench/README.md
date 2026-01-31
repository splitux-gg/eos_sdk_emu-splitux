# EOS-LAN Test Bench

The test bench provides a mock game client and test scripts for validating the EOS-LAN emulator implementation.

## Components

### Mock Game Client (`mock-game/main.c`)

A minimal EOS SDK client that exercises the core APIs:
- Authentication (Connect subsystem)
- Session creation (host mode)
- Session discovery and joining (client mode)

**Usage:**

```bash
# Test authentication only
./build/mock-game.exe --test-auth

# Host a session
./build/mock-game.exe --host --name "MySession" --max-players 4

# Join a session
./build/mock-game.exe --join

# Verbose logging
./build/mock-game.exe --host --name "Test" --verbose 2
```

### Test Scripts (`scripts/`)

- **test-single.sh** - Single instance tests (auth, session creation)
- **test-dual.sh** - Dual instance tests (LAN discovery, join)
- **test-compare.sh** - Compare behavior against real EOS SDK

## Building

The mock game is built automatically when you build the main project:

```bash
# Build on Windows VM (includes mock game)
make deploy-vm

# Or build with CMake manually
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The executable will be at: `build/Release/mock-game.exe`

## Running Tests

```bash
# Run all tests
make test

# Run individual tests
make test-single
make test-dual
```

## Logs

Test logs are saved to `test-bench/logs/`:
- `host.log` - Host instance logs
- `client.log` - Client instance logs
- `our-sdk.log` - Our SDK behavior
- `real-sdk.log` - Real SDK behavior (for comparison)

## Development Workflow

1. Make changes to SDK implementation in `src/`
2. Build: `make deploy-vm`
3. Test: `make test`
4. Check logs in `test-bench/logs/` if tests fail
5. Iterate

## Troubleshooting

**Mock game not found:**
- Ensure you've built the project: `make deploy-vm`
- Check that `build/Release/mock-game.exe` exists

**Tests fail:**
- Check logs in `test-bench/logs/`
- Run with verbose logging: `--verbose 2`
- Ensure Windows VM is accessible for builds

**Session discovery fails:**
- Verify LAN implementation is complete
- Check that multicast/broadcast is working
- Look for "Session created" in host logs
- Look for "Searching for sessions" in client logs
