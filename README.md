# RadioBox by UOC SOUND

RadioBox is a vocal radio-effect VST3 for FL Studio and other VST3 hosts.

## Targets

- Windows 10/11 x64: `RadioBox-Windows-x64-Setup.exe`
- Apple Silicon macOS 11+: `RadioBox-macOS-AppleSilicon.pkg`

## Interface and DSP

The 10 factory presets load real saved values for Lo Roll-Off, Mid EQ, Hi Roll-Off, Bandwidth, Tuning, Filter, Drive, Tone, Stereo Width, and Dry/Wet. Every control supports host automation and manual adjustment. AM, FM, and SW are exclusive band modes. Signal Dropout adds unstable transmission artifacts. Power off bypasses the processing.

The VU needle follows the input peak with fast attack and analog-style release. SIGNAL and OVERLOAD LEDs follow the input, and the amber lamp toggles the VU backlight. The interface uses the original supplied layered artwork in Olive Green, Burgundy, and Blue.

The top toolbar provides the preset browser, Save, preset folder, Reload, UI Size, and UI Color controls. User presets capture the complete parameter state and support save, load, rename, and delete. They are stored outside the plug-in bundle in **Radio Box Presets/User Presets** under `%APPDATA%` on Windows or `~/Library/Application Support` on macOS. Factory presets remain built into the plug-in and are represented by the protected **Factory Presets** directory.

UI Size choices are 50%, 60%, 70%, 80%, 90%, 100%, 110%, 125%, and 150%. The last choice is saved globally for future RadioBox instances.

## Install in FL Studio

Close FL Studio, run the platform installer, then open **Options > Manage plugins > Find installed plugins**. Search for **RadioBox** and add it to a vocal mixer insert.

## Build

The GitHub Actions workflow compiles and tests both native targets, validates the VST3 with pluginval, and packages the installers. Local scripts are in `scripts/`.

Build target: RadioBox 0.4.0.
