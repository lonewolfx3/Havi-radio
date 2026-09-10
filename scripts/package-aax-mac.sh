#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
plugin='build-aax-mac/RadioBox_artefacts/Release/AAX/RadioBox.aaxplugin'
binary="$plugin/Contents/MacOS/RadioBox"
test -f "$binary"
lipo "$binary" -verify_arch arm64 x86_64
codesign --verify --deep --strict "$plugin"
mkdir -p dist
stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT
mkdir -p "$stage/root"
ditto "$plugin" "$stage/root/RadioBox.aaxplugin"
pkgbuild --root "$stage/root" --identifier com.uocsound.radiobox.aax.installer \
  --version 0.4.0 --install-location /Library/Application\ Support/Avid/Audio/Plug-Ins \
  dist/RadioBox-AAX-macOS-Universal.pkg
printf '%s\n' 'Created the universal AAX installer package. Sign and notarize the package for distribution.'
