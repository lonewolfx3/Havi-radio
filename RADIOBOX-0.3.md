# RadioBox 0.3 by UOC SOUND

The top station selector, previous/next buttons, and large tuning dial load the same saved settings. Knobs animate to the actual parameter values; audio uses the engine's existing 25 ms smoothing. Reload resets the current station after manual edits. Host program selection and station automation also load the factory settings.

The center needle and small meter show input peak level in dBFS with fast attack and slower return. These are not calibrated VU meters. AM enables saturation, FM enables wavering movement, and SW enables static. Their lights reflect enabled states.

Choose Original / Olive Green, Gold, Red, or Burgundy. The original panel is retained. Color selection is saved with the session and does not change sound. Manual values and effect states are saved too. The internal plugin identity and state tag remain compatible with Havi Radio.

## Installation

Close the DAW. Remove the previous Havi Radio installation first to avoid two filenames with the same plugin identity. Back up existing projects before upgrading.

- Windows 10/11 x64: run RadioBox-Windows-x64-Setup.exe, then rescan plugins in FL Studio and load RadioBox on a Mixer track.
- macOS 11+ Apple Silicon: run RadioBox-macOS-AppleSilicon.pkg. This build is arm64 only.
- Development installers: Windows is unsigned; Mac is ad-hoc signed and not notarized.

## Validation

The build workflow runs DSP and integration checks, captures all four interface colors, and validates each VST3 with pluginval at strictness 5. Check the workflow results for the exact commit before claiming tests passed. Manual FL Studio testing remains necessary for release qualification.
