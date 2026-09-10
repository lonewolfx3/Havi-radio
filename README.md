# RadioBox by UOC SOUND

RadioBox is a vocal radio-effect VST3 for FL Studio and other VST3 hosts.

## Targets

- Windows 10/11 x64: `RadioBox-Windows-x64-Setup.exe`
- Apple Silicon macOS 11+: `RadioBox-macOS-AppleSilicon.pkg`

## Interface and DSP

The 10 factory presets load real saved values for Lo Roll-Off, Mid EQ, Hi Roll-Off, Bandwidth, Tuning, Filter, Drive, Tone, Stereo Width, and Dry/Wet. Every control supports host automation and manual adjustment. AM, FM, and SW are exclusive band modes. Signal Dropout adds unstable transmission artifacts. Power off bypasses the processing.

The VU needle follows the input peak with fast attack and analog-style release. SIGNAL and OVERLOAD LEDs follow the input, and the amber lamp toggles the VU backlight. The interface includes Olive Green, Gold, Red, Burgundy, and Blue themes based on the supplied RadioBox artwork.

## Install in FL Studio

Close FL Studio, run the platform installer, then open **Options > Manage plugins > Find installed plugins**. Search for **RadioBox** and add it to a vocal mixer insert.

## Build

The GitHub Actions workflow compiles and tests both native targets, validates the VST3 with pluginval, and packages the installers. Local scripts are in `scripts/`.
