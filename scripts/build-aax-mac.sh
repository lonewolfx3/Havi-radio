#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${AAX_SDK_DIR:?Set AAX_SDK_DIR to your licensed Avid AAX SDK folder.}"
cmake -S . -B build-aax-mac -G Xcode \
  -DRADIOBOX_BUILD_AAX=ON -DAAX_SDK_DIR="$AAX_SDK_DIR" \
  '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build-aax-mac --config Release --target RadioBox_AAX RadioBoxDSPTests RadioBoxIntegrationTests --parallel 2
ctest --test-dir build-aax-mac -C Release --output-on-failure
plugin='build-aax-mac/RadioBox_artefacts/Release/AAX/RadioBox.aaxplugin'
binary="$plugin/Contents/MacOS/RadioBox"
test -f "$binary"
lipo "$binary" -verify_arch arm64 x86_64
codesign --force --deep --sign - "$plugin"
codesign --verify --deep --strict "$plugin"
printf '%s\n' "Built $plugin as universal arm64/x86_64. Avid/PACE signing is required before retail Pro Tools will load it."
