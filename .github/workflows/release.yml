name: Release VoLang

# Triggered ONLY manually by you
on:
  workflow_dispatch:
    inputs:
      version:
        description: 'Release Version (e.g., v0.1.0)'
        required: true
        default: 'v0.1.0'

permissions:
  contents: write

jobs:
  # 1. Compile the source code using the Unity Build Makefile
  build:
    name: Compile VoLang
    runs-on: ubuntu-latest
    steps:
      - name: Checkout Code
        uses: actions/checkout@v4

      - name: Compile using GCC
        run: make

      - name: Upload Binary Artifact
        uses: actions/upload-artifact@v4
        with:
          name: volang-binary
          path: volang
          retention-days: 1

  # 2. Package into tar.gz and zip IN PARALLEL using Matrix Strategy
  package:
    name: Package Archive
    needs: build
    runs-on: ubuntu-latest
    strategy:
      matrix:
        format: [zip, tar]
    steps:
      - name: Checkout Code
        uses: actions/checkout@v4

      - name: Download Compiled Binary
        uses: actions/download-artifact@v4
        with:
          name: volang-binary
          path: build_bin/

      - name: Set Executable Permission
        run: chmod +x build_bin/volang

      - name: Sanitize Version Name
        # Replaces spaces with hyphens to prevent shell errors during tar/zip
        run: |
          SAFE_VERSION=$(echo "${{ github.event.inputs.version }}" | tr ' ' '-')
          echo "SAFE_VERSION=$SAFE_VERSION" >> $GITHUB_ENV

      - name: Package as ZIP
        if: matrix.format == 'zip'
        run: |
          mkdir -p release_pack/bin
          cp build_bin/volang release_pack/bin/
          cp install.sh release_pack/ || true
          cd release_pack
          zip -r "../volang-${{ env.SAFE_VERSION }}-linux-amd64.zip" *

      - name: Package as TAR.GZ
        if: matrix.format == 'tar'
        run: |
          mkdir -p release_pack/bin
          cp build_bin/volang release_pack/bin/
          cp install.sh release_pack/ || true
          cd release_pack
          tar -czvf "../volang-${{ env.SAFE_VERSION }}-linux-amd64.tar.gz" *

      - name: Upload Packaged Archive
        uses: actions/upload-artifact@v4
        with:
          name: volang-${{ matrix.format }}
          path: volang-*.${{ matrix.format == 'zip' && 'zip' || 'tar.gz' }}
          retention-days: 1

  # 3. Publish to GitHub Releases
  publish:
    name: Publish to GitHub
    needs: package
    runs-on: ubuntu-latest
    steps:
      - name: Download All Archives
        uses: actions/download-artifact@v4
        with:
          path: archives/

      - name: Create GitHub Release
        uses: softprops/action-gh-release@v2
        with:
          tag_name: ${{ github.event.inputs.version }}
          name: "VoLang ${{ github.event.inputs.version }} - Native Fast Compiler"
          body: |
            ## VoLang ${{ github.event.inputs.version }}
            A highly optimized, zero-copy programming language compiler targeting LLVM IR.
            
            ### Modern Installation (Linux/Termux/macOS):
            ```bash
            curl -sSL [https://raw.githubusercontent.com/YOUR_GITHUB_NAME/VoLang/main/install.sh](https://raw.githubusercontent.com/YOUR_GITHUB_NAME/VoLang/main/install.sh) | bash
            # or using wget:
            # wget -qO- [https://raw.githubusercontent.com/YOUR_GITHUB_NAME/VoLang/main/install.sh](https://raw.githubusercontent.com/YOUR_GITHUB_NAME/VoLang/main/install.sh) | bash
            ```
          files: |
            archives/volang-zip/*.zip
            archives/volang-tar/*.tar.gz
