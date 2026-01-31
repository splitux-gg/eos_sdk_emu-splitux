# EOS-LAN Emulator Development Guide

⚠️ **PRIMARY BUILD TARGET: Windows DLL Only**

This document describes the development workflow for building the EOS-LAN emulator **natively on a Windows VM**. The emulator DLL is used by Proton/Gamescope for running Windows games on Linux.

---

## Windows VM Development Environment

### Quick Access

```bash
# SSH to Windows VM (alias configured in ~/.ssh/config)
ssh winvm

# Or direct access
ssh WINDOWSISBALLS@192.168.122.100
```

### VM Details

| Property | Value |
|----------|-------|
| **IP Address** | 192.168.122.100 |
| **Username** | WINDOWSISBALLS |
| **Authentication** | SSH key (~/.ssh/windows-vm) |
| **OS** | Windows 11 25H2 |
| **RAM** | 16 GB |
| **CPUs** | 10 cores |

### SSH Configuration

Already configured in `~/.ssh/config` as multiple aliases:
- `winvm`
- `windows11-dev`
- `vm`

All three aliases work identically. See `/home/alphasigmachad/ssh-workspace/WINDOWS-VM.md` for detailed VM setup and troubleshooting.

---

## File Synchronization Workflow

### Development Workflow

Development happens on **Linux**, but builds happen on **Windows VM**:

1. **Edit code on Linux**: `/home/alphasigmachad/Code/eos-test/sdk/`
2. **Sync to Windows VM**: `make sync-vm`
3. **Build on Windows**: `make build-vm`
4. **Or combined**: `make deploy-vm`

### Manual Sync Commands

```bash
# Sync source code
rsync -avz --delete ./src/ winvm:'C:/Code/eos-lan/src/'

# Sync headers
rsync -avz --delete ./include/ winvm:'C:/Code/eos-lan/include/'

# Sync CMake configuration
rsync -avz ./CMakeLists.txt winvm:'C:/Code/eos-lan/'
```

### Makefile Targets

```bash
make sync-vm      # Just sync code to Windows VM
make build-vm     # Just build on Windows VM (no sync)
make deploy-vm    # Sync + Build (DEFAULT)
make help         # Show all targets
```

---

## Windows Build System

⚠️ **All builds produce Windows DLL** - this is intentional for Proton/Gamescope compatibility.

### Required Software on Windows VM

The following must be installed on the Windows VM:

| Software | Version | Purpose |
|----------|---------|---------|
| **Visual Studio Build Tools 2022** | Latest | C++ compiler (MSVC) |
| **CMake** | 3.20+ | Build system generator |
| **Git for Windows** | Latest | Source control |

### Build Commands (on Windows VM)

If you need to build manually on the Windows VM:

```cmd
cd C:\Code\eos-lan

# Configure with CMake (generates Visual Studio solution)
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build Release configuration
cmake --build build --config Release
```

### Output Location

**C:\Code\eos-lan\build\Release\EOSSDK-Win64-Shipping.dll**

### Build from Linux Host

The recommended workflow is to build from the Linux host using the Makefile:

```bash
cd /home/alphasigmachad/Code/eos-test/sdk

# Default: Sync + Build
make deploy-vm

# Or step by step:
make sync-vm    # 1. Sync code
make build-vm   # 2. Build on VM
```

### Why Windows-Only?

- **Target**: Native Windows games running via Proton/Gamescope on Linux
- **Requirement**: DLL must be Windows-native for game compatibility
- **Reality**: Linux builds are irrelevant for this use case

Windows games running under Proton/Wine need a Windows DLL, not a Linux .so file.

---

## Git Worktree Strategy for Parallel Development

Use git worktrees to enable multiple developers (or sub-agents) to work on different components simultaneously without conflicts.

### Creating Worktrees

```bash
cd /home/alphasigmachad/Code/eos-test/sdk

# Create worktree for callbacks implementation
git worktree add ../feature-callbacks feature/callbacks

# Create worktree for sessions implementation
git worktree add ../feature-sessions feature/sessions

# Create worktree for P2P implementation
git worktree add ../feature-p2p feature/p2p

# Create worktree for network layer
git worktree add ../feature-network feature/network
```

### Branch Naming Convention

| Pattern | Purpose | Example |
|---------|---------|---------|
| `feature/<component>` | New component implementation | `feature/callbacks`, `feature/sessions` |
| `fix/<issue>` | Bug fixes | `fix/session-discovery` |
| `experiment/<name>` | Experimental work | `experiment/udp-relay` |

### Workflow

1. **Create worktree** with feature branch
2. **Implement component** in isolation
3. **Test independently** using dual-instance tests
4. **Merge back to main** when complete
5. **Remove worktree**: `git worktree remove ../feature-sessions`

### Sub-Agent Assignment Example

Perfect for parallel development with Claude Code sub-agents:

```bash
# Main SDK remains at: /home/alphasigmachad/Code/eos-test/sdk

# Create 4 worktrees for parallel work:
git worktree add ../feature-callbacks feature/callbacks
git worktree add ../feature-sessions feature/sessions
git worktree add ../feature-p2p feature/p2p
git worktree add ../feature-network feature/network
```

Each agent works in their own worktree:

- **Agent 1** (feature/callbacks): Implement callback queue system
- **Agent 2** (feature/sessions): Implement session management
- **Agent 3** (feature/p2p): Implement P2P packet handling
- **Agent 4** (feature/network): Implement LAN discovery

**Benefits**:
- No merge conflicts during development
- Independent testing and verification
- Clean integration when merging back to main
- Each agent can build and test independently

### Listing Worktrees

```bash
git worktree list
```

Output example:
```
/home/alphasigmachad/Code/eos-test/sdk              5143f87 [main]
/home/alphasigmachad/Code/eos-test/feature-callbacks 1a2b3c4 [feature/callbacks]
/home/alphasigmachad/Code/eos-test/feature-sessions  5d6e7f8 [feature/sessions]
```

---

## Testing with Palworld

### DLL Deployment Path

**Palworld Install Location**:
```
C:\Program Files (x86)\Steam\steamapps\common\Palworld\Pal\Binaries\Win64\
```

### Deployment Steps

1. **Backup original DLL** (on Windows VM):
   ```cmd
   cd "C:\Program Files (x86)\Steam\steamapps\common\Palworld\Pal\Binaries\Win64"
   copy EOSSDK-Win64-Shipping.dll EOSSDK-Win64-Shipping.dll.original
   ```

2. **Deploy custom DLL** (on Windows VM):
   ```cmd
   copy C:\Code\eos-lan\build\Release\EOSSDK-Win64-Shipping.dll "C:\Program Files (x86)\Steam\steamapps\common\Palworld\Pal\Binaries\Win64\"
   ```

3. **Launch Palworld** and test multiplayer

### Dual-Instance Testing

To test LAN session discovery and P2P communication:

1. **Start Palworld instance 1 (Host)**:
   - Create a multiplayer session
   - Set session name (e.g., "Test Session")
   - Wait for other players

2. **Start Palworld instance 2 (Client)**:
   - Search for multiplayer sessions
   - Should see "Test Session" in the list
   - Join the session

3. **Verify functionality**:
   - Client successfully finds host's session
   - Client can join the session
   - P2P connection established
   - Game traffic flows correctly

### Log Locations

| Log Type | Path | Purpose |
|----------|------|---------|
| **Palworld logs** | `%LOCALAPPDATA%\Pal\Saved\Logs\` | Game engine logs |
| **EOS-LAN logs** | Same directory as DLL | Emulator debug logs |
| **Log config** | `eos-lan.ini` | Log level configuration |

Example paths:
- Palworld: `C:\Users\WINDOWSISBALLS\AppData\Local\Pal\Saved\Logs\Palworld.log`
- EOS-LAN: `C:\Program Files (x86)\Steam\steamapps\common\Palworld\Pal\Binaries\Win64\eos-lan.log`

### Log Configuration

Create `eos-lan.ini` in the same directory as the DLL:

```ini
[logging]
log_file = eos-lan.log
log_level = 5  # 0=none, 1=errors, 2=warnings, 3=info, 4=debug, 5=trace
```

### What to Look For in Logs

When testing, check for:

- **Session announcements** (should broadcast every 2 seconds):
  ```
  [Sessions] Broadcasting session announcement: Test Session
  ```

- **Session discovery queries**:
  ```
  [Sessions] Received session query from client
  [Sessions] Found 1 session(s)
  ```

- **P2P connection establishment**:
  ```
  [P2P] Connection request from ProductUserID: EOSLAN_...
  [P2P] Connection established with peer
  ```

- **Packet transmission**:
  ```
  [P2P] Sent 1024 bytes to peer
  [P2P] Received 512 bytes from peer
  ```

---

## Quick Reference

### Most Common Commands

```bash
# On Linux: Build Windows DLL
cd /home/alphasigmachad/Code/eos-test/sdk
make deploy-vm

# On Windows VM: Deploy to Palworld
copy C:\Code\eos-lan\build\Release\EOSSDK-Win64-Shipping.dll "C:\Program Files (x86)\Steam\steamapps\common\Palworld\Pal\Binaries\Win64\"

# On Windows VM: Check logs
type "C:\Program Files (x86)\Steam\steamapps\common\Palworld\Pal\Binaries\Win64\eos-lan.log"
```

### Troubleshooting

**Build fails on Windows VM**:
- Verify Visual Studio Build Tools 2022 is installed
- Verify CMake is in PATH: `cmake --version`
- Check for compilation errors in the output

**DLL doesn't load in Palworld**:
- Ensure DLL is in correct directory
- Check Windows Event Viewer for DLL load errors
- Verify DLL architecture is x64 (not x86)

**Can't SSH to Windows VM**:
- Check VM is running: `virsh list --all`
- Verify SSH service on Windows: `ssh winvm "sc query sshd"`
- See `/home/alphasigmachad/ssh-workspace/WINDOWS-VM.md` for detailed troubleshooting

**Session discovery doesn't work**:
- Check firewall allows UDP port 23456
- Verify both instances are on same network
- Check eos-lan.log for broadcast/discovery messages
- Ensure Windows network is set to "Private" (not Public)

---

## See Also

- [DESIGN.md](DESIGN.md) - Complete architecture and API specifications
- `/home/alphasigmachad/ssh-workspace/WINDOWS-VM.md` - VM setup and troubleshooting
- `/home/alphasigmachad/Code/eos-test/VM-SETUP.md` - Detailed VM configuration
