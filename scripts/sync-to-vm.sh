#!/bin/bash
# Sync files to Windows VM using tar over SSH
# This works without rsync/scp

set -e

VM_HOST="winvm"
VM_PATH="C:/Code/eos-lan"

echo "==> Creating tarball of source code..."
tar czf /tmp/eos-lan-sync.tar.gz -C /home/alphasigmachad/Code/eos-test/sdk \
    src/ include/ CMakeLists.txt

echo "==> Transferring to Windows VM..."
cat /tmp/eos-lan-sync.tar.gz | ssh "$VM_HOST" "powershell -Command \"\$input | Set-Content -Path C:/temp-sync.tar.gz -Encoding Byte\""

echo "==> Extracting on Windows VM..."
ssh "$VM_HOST" '"C:\Program Files\Git\usr\bin\tar.exe" -xzf C:/temp-sync.tar.gz -C C:/Code/eos-lan'

echo "==> Cleaning up..."
rm /tmp/eos-lan-sync.tar.gz
ssh "$VM_HOST" "del C:\temp-sync.tar.gz"

echo "==> Sync complete!"
