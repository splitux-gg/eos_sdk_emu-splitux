# GitHub Actions Self-Hosted Runner Setup

## Overview

This project uses self-hosted GitHub Actions runners for CI/CD:

| Runner | Machine | Labels | Registered To |
|--------|---------|--------|---------------|
| splitux-linux-1 | lab (10.0.0.6) | `splitux`, `linux`, `x64` | splitux-gg org |
| splitux-windows-1 | compute (10.0.0.2) | `splitux`, `windows`, `x64` | splitux-gg org |
| gabrielgad-linux-1 | lab (10.0.0.6) | `gabrielgad`, `linux`, `x64` | gabrielgad/dsp-headless-server |
| gabrielgad-windows-1 | compute (10.0.0.2) | `gabrielgad`, `windows`, `x64` | gabrielgad/dsp-headless-server |

## Usage in Workflows

### For splitux-gg repos:
```yaml
runs-on: [self-hosted, splitux, linux]    # Linux builds
runs-on: [self-hosted, splitux, windows]  # Windows builds
```

### For gabrielgad personal repos:
```yaml
runs-on: [self-hosted, gabrielgad, linux]    # Linux builds
runs-on: [self-hosted, gabrielgad, windows]  # Windows builds
```

## Runner Locations

### Lab Server (10.0.0.6 / lab)
- **OS:** Arch Linux (CachyOS)
- **Runners:**
  - `~/actions-runner-splitux/` → splitux-linux-1
  - `~/actions-runner-gabrielgad/` → gabrielgad-linux-1
- **Services:**
  - `actions.runner.splitux-gg.splitux-linux-1.service`
  - `actions.runner.gabrielgad-dsp-headless-server.gabrielgad-linux-1.service`

### Compute Node (10.0.0.2 / compute)
- **OS:** Windows 11
- **Runners:**
  - `C:\actions-runner-splitux\` → splitux-windows-1
  - `C:\actions-runner-gabrielgad\` → gabrielgad-windows-1
- **Services (via NSSM):**
  - `actions-runner-splitux`
  - `actions-runner-gabrielgad`

## Management Commands

### Linux (lab)
```bash
# Check status
sudo systemctl status actions.runner.splitux-gg.splitux-linux-1
sudo systemctl status actions.runner.gabrielgad-dsp-headless-server.gabrielgad-linux-1

# Restart
sudo systemctl restart actions.runner.splitux-gg.splitux-linux-1

# View logs
journalctl -u actions.runner.splitux-gg.splitux-linux-1 -f
```

### Windows (compute)
```powershell
# Check status
nssm status actions-runner-splitux
nssm status actions-runner-gabrielgad

# Restart
nssm restart actions-runner-splitux

# View logs
Get-Content C:\actions-runner-splitux\_diag\Runner_*.log -Tail 50
```

## Adding a New Runner

### For splitux-gg org (any repo in org can use):
```bash
# Get token
TOKEN=$(gh api -X POST /orgs/splitux-gg/actions/runners/registration-token --jq '.token')

# Configure
./config.sh --url https://github.com/splitux-gg \
  --token $TOKEN \
  --name splitux-<platform>-<N> \
  --labels splitux,<platform>,x64 \
  --unattended
```

### For gabrielgad personal repos:
```bash
# Get token (per-repo, user-level not supported via API)
TOKEN=$(gh api -X POST /repos/gabrielgad/<repo>/actions/runners/registration-token --jq '.token')

# Configure
./config.sh --url https://github.com/gabrielgad/<repo> \
  --token $TOKEN \
  --name gabrielgad-<platform>-<N> \
  --labels gabrielgad,<platform>,x64 \
  --unattended
```

## Removing a Runner

### Via GitHub API:
```bash
# List runners
gh api /orgs/splitux-gg/actions/runners --jq '.runners[] | "\(.id) \(.name)"'

# Remove by ID
gh api -X DELETE /orgs/splitux-gg/actions/runners/<ID>
```

### On the machine:
```bash
# Linux
sudo ./svc.sh stop
sudo ./svc.sh uninstall
./config.sh remove --token <TOKEN>

# Windows
nssm stop actions-runner-<name>
nssm remove actions-runner-<name> confirm
config.cmd remove --token <TOKEN>
```

## Notes

- **Org-level runners** (splitux-*) can be used by any repo in the splitux-gg org
- **Repo-level runners** (gabrielgad-*) are registered to dsp-headless-server but can be reused if other personal repos use the same labels
- GitHub doesn't support user-level runners via API, so personal repo runners must be registered per-repo
- Runners auto-update when GitHub releases new versions
- Inactive runners are auto-deleted by GitHub after 14 days
