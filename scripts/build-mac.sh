#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
command -v cmake >/dev/null || { echo 'Install CMake first: https://cmake.org/download/'; exit 1; }
xcrun --find clang >/dev/null
cmake -S . -B build-mac -G Xcode -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build-mac --config Release --target RadioBox_VST3 RadioBoxDSPTests RadioBoxIntegrationTests --parallel 2
ctest --test-dir build-mac -C Release --output-on-failure
plugin='build-mac/RadioBox_artefacts/Release/VST3/RadioBox.vst3'
lipo "$plugin/Contents/MacOS/RadioBox" -verify_arch arm64
codesign --force --sign - "$plugin"
codesign --verify --strict "$plugin"
mkdir -p dist
ditto -c -k --sequesterRsrc --keepParent "$plugin" dist/RadioBox-macOS-AppleSilicon.zip
echo 'Built dist/RadioBox-macOS-AppleSilicon.zip. Local ad-hoc signature only; not notarized.'
