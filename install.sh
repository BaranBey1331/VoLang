#!/usr/bin/env bash

# VoLang Professional Installer Script
# Supports: curl and wget

set -e

# Color codes for terminal UI
CYAN='\033[0;36m'
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${CYAN}"
echo "========================================"
echo "      Installing VoLang Compiler        "
echo "========================================"
echo -e "${NC}"

# Detect download tool (curl or wget)
if command -v curl >/dev/null 2>&1; then
    DOWNLOAD_CMD="curl -sSL"
elif command -v wget >/dev/null 2>&1; then
    DOWNLOAD_CMD="wget -qO-"
else
    echo -e "${RED}Error: You need curl or wget installed to run this script.${NC}"
    exit 1
fi

# Define installation directory (Local user space to avoid sudo requirements)
INSTALL_DIR="$HOME/.local/bin"
mkdir -p "$INSTALL_DIR"

# 1. Fetch latest release info from GitHub API
# NOTE: Replace 'YOUR_GITHUB_NAME' with your actual github username before pushing!
REPO="YOUR_GITHUB_NAME/VoLang"
echo "-> Fetching latest version info..."
LATEST_TAG=$($DOWNLOAD_CMD "https://api.github.com/repos/$REPO/releases/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')

if [ -z "$LATEST_TAG" ]; then
    echo -e "${RED}Error: Could not find latest release. Are you sure you published one?${NC}"
    # Fallback for development testing
    LATEST_TAG="v0.1.0"
fi

echo -e "-> Latest version found: ${GREEN}${LATEST_TAG}${NC}"

# 2. Download the Tar archive
TAR_URL="https://github.com/$REPO/releases/download/${LATEST_TAG}/volang-${LATEST_TAG}-linux-amd64.tar.gz"
TMP_DIR=$(mktemp -d)

echo "-> Downloading VoLang archive..."
if command -v curl >/dev/null 2>&1; then
    curl -# -L "$TAR_URL" -o "$TMP_DIR/volang.tar.gz"
else
    wget --show-progress -q "$TAR_URL" -O "$TMP_DIR/volang.tar.gz"
fi

# 3. Extract and Install
echo "-> Extracting files..."
tar -xzf "$TMP_DIR/volang.tar.gz" -C "$TMP_DIR"

echo "-> Installing binary to $INSTALL_DIR..."
mv "$TMP_DIR/bin/volang" "$INSTALL_DIR/volang"
chmod +x "$INSTALL_DIR/volang"

# Cleanup
rm -rf "$TMP_DIR"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  VoLang was successfully installed!    ${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "You can now run VoLang by typing: volang"
echo ""

# Check if ~/.local/bin is in PATH
if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
    echo -e "${CYAN}Note: ${INSTALL_DIR} is not in your PATH.${NC}"
    echo "To fix this, add the following line to your ~/.bashrc or ~/.zshrc:"
    echo "export PATH=\"\$HOME/.local/bin:\$PATH\""
fi
