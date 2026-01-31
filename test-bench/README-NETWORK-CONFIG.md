# EOS-LAN Network Configuration

The EOS-LAN emulator supports configuring network settings via **environment variables**. This allows you to customize discovery ports, broadcast addresses, and other network parameters **without modifying game code**.

## Environment Variables

| Variable | Default | Valid Range | Description |
|----------|---------|-------------|-------------|
| `EOSLAN_DISCOVERY_PORT` | 23456 | 1024-65535 | UDP port for LAN discovery |
| `EOSLAN_BROADCAST_ADDR` | 255.255.255.255 | IPv4 address | Broadcast address for session announcements |
| `EOSLAN_ANNOUNCE_INTERVAL` | 2000 | 500-10000 | Announcement interval in milliseconds |
| `EOSLAN_PREFERRED_IP` | (auto-detect) | IPv4 address | Preferred local network interface IP |
| `EOSLAN_DEBUG` | 0 | 0 or 1 | Enable debug logging for LAN networking |

## Usage Examples

### Basic Custom Port

**Linux/Mac:**
```bash
EOSLAN_DISCOVERY_PORT=12345 ./game.exe --host
```

**Windows (PowerShell):**
```powershell
$env:EOSLAN_DISCOVERY_PORT=12345
.\game.exe --host
```

**Windows (CMD):**
```cmd
set EOSLAN_DISCOVERY_PORT=12345
game.exe --host
```

### Subnet-Specific Broadcast

```bash
# Use subnet broadcast instead of global broadcast
EOSLAN_BROADCAST_ADDR=192.168.1.255 ./game.exe --host
```

### Faster Announcements for Testing

```bash
# Broadcast every 1 second instead of 2 seconds
EOSLAN_ANNOUNCE_INTERVAL=1000 ./game.exe --host
```

### Run Multiple "LAN Clusters" on Same Machine

```bash
# Terminal 1: Cluster A on port 12345
EOSLAN_DISCOVERY_PORT=12345 ./game.exe --host --name "Cluster A" &

# Terminal 2: Cluster B on port 23456
EOSLAN_DISCOVERY_PORT=23456 ./game.exe --host --name "Cluster B" &
```

These two hosts won't discover each other because they use different ports!

### Docker/Container Environment

**Dockerfile:**
```dockerfile
FROM ubuntu:latest

# Set network config for container environment
ENV EOSLAN_BROADCAST_ADDR=172.17.0.255
ENV EOSLAN_DISCOVERY_PORT=23456
ENV EOSLAN_DEBUG=1

COPY game.exe /app/
CMD ["/app/game.exe", "--host"]
```

### Multi-Interface Systems

If your system has multiple network interfaces and autodiscovery picks the wrong one:

```bash
# Force emulator to use specific interface
EOSLAN_PREFERRED_IP=192.168.1.100 ./game.exe --host
```

## Testing

### Test Two Instances with Custom Port

```bash
# Terminal 1: Host on port 12345
EOSLAN_DISCOVERY_PORT=12345 ./mock-game --host --name "CustomPortTest"

# Terminal 2: Client on same port (should discover)
EOSLAN_DISCOVERY_PORT=12345 ./mock-game --join

# Terminal 3: Client on different port (should NOT discover)
./mock-game --join  # Uses default port 23456
```

### Test Isolation

```bash
# These won't see each other (different ports)
EOSLAN_DISCOVERY_PORT=11111 ./game1 --host &
EOSLAN_DISCOVERY_PORT=22222 ./game2 --host &

# These WILL see each other (same port)
EOSLAN_DISCOVERY_PORT=33333 ./game3 --host &
EOSLAN_DISCOVERY_PORT=33333 ./game4 --join  # Finds game3
```

## Troubleshooting

### "Invalid EOSLAN_DISCOVERY_PORT" Error

```
[ERROR] Invalid EOSLAN_DISCOVERY_PORT: 999 (must be 1024-65535)
```

**Cause:** Port number out of valid range (must be ≥ 1024)
**Fix:** Use a port between 1024 and 65535

### "Invalid EOSLAN_ANNOUNCE_INTERVAL" Error

```
[ERROR] Invalid EOSLAN_ANNOUNCE_INTERVAL: 100 (must be 500-10000)
```

**Cause:** Interval too short or too long
**Fix:** Use a value between 500ms and 10000ms

### Client Can't Find Host

**Check 1:** Both using same discovery port?
```bash
# Host
EOSLAN_DISCOVERY_PORT=12345 ./game --host

# Client MUST use same port
EOSLAN_DISCOVERY_PORT=12345 ./game --join
```

**Check 2:** Firewall blocking UDP traffic?
```bash
# Linux: Allow UDP port
sudo ufw allow 12345/udp

# Windows: Add firewall rule
netsh advfirewall firewall add rule name="EOS-LAN" dir=in action=allow protocol=UDP localport=12345
```

**Check 3:** Check broadcast address for your network
```bash
# Linux: Find your broadcast address
ip addr show | grep brd

# Example output:
# inet 192.168.1.100/24 brd 192.168.1.255 scope global eth0

# Use the broadcast address:
EOSLAN_BROADCAST_ADDR=192.168.1.255 ./game --host
```

## How It Works

1. When `EOS_Platform_Create()` is called, the emulator reads environment variables
2. If found and valid, they override the default hardcoded values
3. Invalid values are rejected with an error log and fall back to defaults
4. The configuration is logged at INFO level for verification

**Example log output:**
```
[INFO] Using discovery port from env: 12345
[INFO] Using broadcast address from env: 192.168.1.255
[INFO] SessionsState created with LAN discovery: port=12345, broadcast=192.168.1.255, interval=2000ms
```

## Advanced: Programmatic Configuration

While environment variables are the recommended approach, you can also set them programmatically in your launcher:

**C++:**
```cpp
#ifdef _WIN32
_putenv("EOSLAN_DISCOVERY_PORT=12345");
#else
setenv("EOSLAN_DISCOVERY_PORT", "12345", 1);
#endif

// Now initialize EOS SDK
EOS_Platform_Create(&options);
```

**Python (launcher script):**
```python
import os
import subprocess

os.environ["EOSLAN_DISCOVERY_PORT"] = "12345"
os.environ["EOSLAN_BROADCAST_ADDR"] = "192.168.1.255"

subprocess.run(["./game.exe", "--host"])
```

## Platform-Specific Notes

### Windows

Environment variables persist only for the current session:

```powershell
# PowerShell: Set for current session
$env:EOSLAN_DISCOVERY_PORT=12345

# PowerShell: Set permanently (user level)
[Environment]::SetEnvironmentVariable("EOSLAN_DISCOVERY_PORT", "12345", "User")

# CMD: Set for current session
set EOSLAN_DISCOVERY_PORT=12345
```

### Linux/Mac

```bash
# Bash: Set for single command
EOSLAN_DISCOVERY_PORT=12345 ./game

# Bash: Set for session
export EOSLAN_DISCOVERY_PORT=12345

# Bash: Set permanently (add to ~/.bashrc)
echo 'export EOSLAN_DISCOVERY_PORT=12345' >> ~/.bashrc
```

## Summary

- ✅ **Zero code changes** - Works with any game using the emulator
- ✅ **Per-process config** - Each instance can have different settings
- ✅ **Standard pattern** - Familiar to developers
- ✅ **Backward compatible** - Defaults work if env vars not set
- ✅ **Validation** - Invalid values rejected with clear error messages

For questions or issues, see the main README or project documentation.
