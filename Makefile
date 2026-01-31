# EOS-LAN Emulator Build System
# PRIMARY TARGET: Windows DLL built on compute node
# Linux builds: Not a priority

VM_HOST = compute-local
VM_DIR = C:/Code/eos-lan

.PHONY: all build-vm deploy-vm clean help test test-single test-dual

# Default target: Windows build on VM
all: deploy-vm

help:
	@echo "EOS-LAN Emulator Makefile"
	@echo "PRIMARY: Windows DLL builds on compute node ($(VM_HOST))"
	@echo ""
	@echo "Targets:"
	@echo "  make build-vm   - Build DLL on compute node"
	@echo "  make deploy-vm  - Push + Build (DEFAULT)"
	@echo "  make test       - Run all tests"
	@echo "  make test-single - Run single instance test"
	@echo "  make test-dual  - Run dual instance test"
	@echo "  make test-windows - Run tests on compute node"
	@echo "  make clean      - Remove local build artifacts"

# Push code to compute node via git (CI runner clones from GitHub)
# For manual dev: scp changed files
push:
	git push origin main

# Build Windows DLL on compute node
build-vm:
	@echo "==> Building Windows DLL on compute node..."
	ssh $(VM_HOST) 'powershell -Command "Set-Location $(VM_DIR); cmake -B build -G \"Visual Studio 17 2022\" -A x64; cmake --build build --config Release"'
	@echo "==> Build complete: $(VM_DIR)/build/Release/EOSSDK-Win64-Shipping.dll"

# Full workflow: push + build
deploy-vm: push build-vm
	@echo "==> Windows DLL built successfully"
	@echo "==> Output: $(VM_DIR)/build/Release/EOSSDK-Win64-Shipping.dll"

# Test targets
test: test-single test-dual

test-single:
	@echo "==> Running single instance test..."
	@./test-bench/scripts/test-single.sh

test-dual:
	@echo "==> Running dual instance test..."
	@./test-bench/scripts/test-dual.sh

test-windows:
	@echo "==> Running tests on compute node..."
	@./test-bench/scripts/test-dual-windows.sh

clean:
	rm -rf build/
