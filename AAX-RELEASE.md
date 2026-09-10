# RadioBox AAX release requirements

RadioBox 0.4 adds AAX Native build targets without redistributing Avid's licensed SDK.

## Supported targets

- Windows x64: 64-bit AAX Native for Pro Tools 12 and later on a compatible Windows installation.
- macOS universal: Intel x86_64 plus Apple Silicon arm64 AAX Native. Native Apple Silicon requires Pro Tools 2023.3 or later; Pro Tools 2022.12 offered a public beta.
- Pro Tools 12 for Mac supports Intel Macs only. It cannot run natively on Apple Silicon.

## Required release credentials

Set `AAX_SDK_DIR` to an accepted Avid AAX SDK, build with the platform script, then sign the `.aaxplugin` using the Avid/PACE commercial signing tools. A locally compiled or ad-hoc signed AAX bundle is an evaluation artifact and will not load in an ordinary retail Pro Tools installation.

Commercial distribution also requires the appropriate Avid developer agreement, PACE signing access, an iLok account and USB key, macOS Developer ID signing, and Apple notarization.

The AAX SDK is licensed and must not be committed to this repository or bundled in source archives.
