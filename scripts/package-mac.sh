#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
plugin='build-mac/RadioBox_artefacts/Release/VST3/RadioBox.vst3'
[[ -f "$plugin/Contents/MacOS/RadioBox" ]] || { echo 'No compiled Mac plug-in. Run scripts/build-mac.sh first.'; exit 1; }
lipo "$plugin/Contents/MacOS/RadioBox" -verify_arch arm64
codesign --verify --strict "$plugin"
mkdir -p dist
stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT
mkdir -p "$stage/root"
ditto "$plugin" "$stage/root/RadioBox.vst3"
# Prevent Installer from relocating an existing copy outside the VST3 folder.
pkgbuild --analyze --root "$stage/root" "$stage/components.plist"
python3 - "$stage/components.plist" <<'PYPLIST'
import plistlib, sys
with open(sys.argv[1], 'rb') as f:
    components = plistlib.load(f)
for component in components:
    component['BundleIsRelocatable'] = False
with open(sys.argv[1], 'wb') as f:
    plistlib.dump(components, f)
PYPLIST
pkgbuild --root "$stage/root" --component-plist "$stage/components.plist" \
  --identifier com.havilegrand.haviradio.installer --version 0.3.0 \
  --install-location /Library/Audio/Plug-Ins/VST3 \
  dist/RadioBox-macOS-AppleSilicon.pkg
pkgutil --payload-files dist/RadioBox-macOS-AppleSilicon.pkg
printf '%s\n' 'Created unsigned development installer. Developer ID signing/notarization is still required for normal public Mac distribution.'
