# Radio Box / Havi Radio v0.2

Vocal radio-effect VST3 project using your supplied Radio Box artwork. The host plug-in name remains **Havi Radio** to preserve the original project identity.

**Build status: Windows x64 and Apple Silicon VST3s compiled successfully, passed standalone DSP tests and pluginval strictness level 5, and were packaged into installers.** Download the artifacts from the successful [build run](https://github.com/lonewolfx3/Havi-radio/actions/runs/34434727377). Windows includes an `.exe` installer; Mac includes a `.pkg`. They are unsigned development installers, and the Mac package is not notarized. FL Studio 2026 listening and installation tests remain to be completed.

## Requested targets

| Platform | Build target | Intended host |
|---|---|---|
| Apple Silicon Mac | Native arm64, macOS 11 or later | FL Studio 2026 running natively |
| Windows 10/11 | x64 | FL Studio 2026 64-bit |

Native builds and automated plug-in validation passed for both targets. This does not establish tested FL Studio compatibility. FL Studio's own OS requirements also apply. This version does not target Windows ARM or Intel Macs.

## Interactive hardware interface

- The supplied JPEG is embedded in the plug-in, so it requires no external image path after building.
- Six live knobs overlay the photographed controls. Drag vertically, use the scroll wheel, or double-click for defaults. The drawn knob indicators and grip details rotate with their values. Host automation updates the same controls.
- Bandwidth narrows/widens the passband. Filter adjusts tone. Drive controls saturation. Output adjusts dB. Dry/Wet blends the signal.
- The large tuning knob selects the ten stations. A selector inside the radio display shows the station name and allows direct selection.
- The small meter's static face is covered with a drawn scale and live needle. It reads incoming audio, with about 35 ms attack and 250 ms release. It returns toward zero when audio stops, including when the host stops calling the processor.
- This meter is an input peak meter marked in dBFS, not a calibrated analog VU meter. The long printed frequency pointer in the main display remains decorative.
- AM switches saturation, FM switches cutoff/volume wavering, and SW switches signal-gated static. Each switch lights only when its own parameter is on. Their states and knob positions save with the FL Studio project. All three default on to retain the original preset sounds.
- The existing Static amount parameter remains available through host automation. SW switches it on/off at its saved level.
- The original photograph is not a layered asset. Live controls are drawn over its knobs, button faces and meter face, so moving parts are a styled reconstruction rather than exact photographic rotations. The original decorative knob scales are normalized visual guides; tooltips/value popups show actual parameter values.

## Stations

| Station | Intended character |
|---|---|
| VINTAGE RADIO | Narrow mids, gritty saturation, gentle wavering and hiss |
| RADIO 2000'S | Brighter radio vocal with moderate saturation |
| 80'S RADIO | Broader bandwidth with a soft wavering texture |
| SUMMER 2015 | Airier, lightly colored vocal |
| URBAN CITY RADIO | Forward mids and stronger grit |
| UNDER WATER RADIO | Dark filtering with slow moving cutoff and volume |
| MODERN RADIO | Widest bandwidth and subtle saturation |
| BOOM BOX RADIO | Low-mid weight, rolled-off highs and grit |
| TELEPHONE RADIO | Very narrow telephone-style vocal, proposed extra |
| MIDNIGHT AM | Dark, rough AM-style texture, proposed extra |

These are creative interpretations, not measured hardware or historical broadcast emulations. Default filtering spans 100–10,500 Hz for Modern and 90–850 Hz for Under Water. Tone shifts filter cutoffs up or down an octave. Grit scales preset saturation; Static scales signal-gated hiss. Mix runs from 0 (dry) to 1 (wet). Output is -24 to +6 dB. Double-click a knob to restore its default.

## Build locally

Internet access and Git are needed for the first build, which fetches JUCE 8.0.6. An existing JUCE source checkout can instead be supplied through CMake's JUCE_DIR option.

**Mac:** Install full Xcode and its command line tools, plus CMake 3.22 or later. From this folder run:

```sh
bash scripts/build-mac.sh
```

Output: `dist/HaviRadio-macOS-AppleSilicon.zip`. The script verifies arm64 architecture and adds a local ad-hoc signature. This is not Apple Developer signing or notarization. A publicly distributed Mac release needs the appropriate signing/notarization process; do not disable Gatekeeper to install it.

**Windows:** Install Visual Studio 2022 with Desktop development with C++, a Windows SDK, CMake 3.22 or later, and Git. From PowerShell in this folder run:

```powershell
./scripts/build-windows.ps1
```

Output: `dist/HaviRadio-Windows-x64.zip`. The project uses the static MSVC runtime. The script checks that the x64 VST3 binary exists before packaging it. Both platform scripts have now run successfully on native GitHub Actions runners.

## Build both through GitHub Actions

Put the **contents** of this HaviRadio directory at the root of a GitHub repository, including `.github/workflows/build.yml`. The supplied workflow uses Windows and Mac runners to compile, run DSP tests, and upload separate ZIP artifacts. It runs on pushes to main or manually from Actions > Build Radio Box VST3 > Run workflow. The project is hosted at https://github.com/lonewolfx3/Havi-radio and the workflow has run successfully. These build jobs are not ongoing scheduled automations.

## Generate installers after successful compilation

The automated workflow now also packages a Windows `.exe` installer using NSIS and an Apple Silicon `.pkg` using Apple's pkgbuild. These packaging steps have run successfully on their native operating systems. They deliberately fail when the compiled VST3 is missing.

For local packaging:

- Mac: `bash scripts/package-mac.sh`
- Windows: install NSIS 3, then run `./scripts/package-windows.ps1`

Expected outputs are `dist/HaviRadio-Windows-x64-Setup.exe` and `dist/HaviRadio-macOS-AppleSilicon.pkg`. Both are development installers without publisher signing; the Mac installer is not notarized. A successful workflow does not replace FL Studio testing or distribution signing.

Windows installs the bundle to the shared 64-bit VST3 folder and adds an uninstaller. Mac installs to `/Library/Audio/Plug-Ins/VST3/`. Both use system locations that require administrator authorization when run. Close FL Studio first. The project ZIP you downloaded contains installer source, not these generated installer files.

## Install after successful compilation

Extract the correct platform ZIP and copy the complete `Havi Radio.vst3` bundle:

- Mac: `~/Library/Audio/Plug-Ins/VST3/`
- Windows: `C:/Program Files/Common Files/VST3/` (may require administrator permission)

Open FL Studio's Options > Manage plugins, scan, and load Havi Radio in a vocal Mixer effect slot. Begin with a low monitoring level. Output can exceed 0 dBFS at high input or gain settings; this effect is not a limiter.

## Validation still required

Completed: standalone C++ engine tests at five sample rates, all ten presets, switch combinations during preset changes, bandwidth extremes, finite output, channel isolation, silence and dry mix. Meter tests check peak capture/reset, attack and return to zero. Mac shell script syntax was checked. Native JUCE compilation, Windows script execution, and pluginval level 5 checks passed on both targets, including editor creation, audio processing, automation and state tests. Visual inspection and FL Studio listening/project-recall tests have not been completed.

Before release, run pluginval or the VST3 validator; verify scanning, all controls and switches, automation, project save/reopen, bypass, mono/stereo, multiple instances, resized editor, sample-rate changes and offline rendering on both targets. Audition and level-match all stations using real vocals.

The distortion is not oversampled and may alias at high Drive settings. There is no compressor, delay, stereo widening or measured analog circuit model. Review JUCE's applicable license before distributing binaries; JUCE is a separate dependency.

## References

- JUCE CMake documentation: https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md
- FL Studio plug-in installation: https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm

