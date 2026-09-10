$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
function Run-Native {
    param([string]$Exe, [string[]]$Arguments)
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe failed with exit code $LASTEXITCODE" }
}
Run-Native 'cmake' @('-S', '.', '-B', 'build-windows', '-G', 'Visual Studio 17 2022', '-A', 'x64')
Run-Native 'cmake' @('--build', 'build-windows', '--config', 'Release', '--target', 'HaviRadio_VST3', 'HaviRadioDSPTests', '--parallel', '2')
Run-Native 'ctest' @('--test-dir', 'build-windows', '-C', 'Release', '--output-on-failure')
$plugin = 'build-windows/HaviRadio_artefacts/Release/VST3/Havi Radio.vst3'
if (-not (Test-Path "$plugin/Contents/x86_64-win/Havi Radio.vst3")) { throw 'Expected x64 VST3 binary missing' }
New-Item -ItemType Directory -Force -Path 'dist' | Out-Null
Compress-Archive -Path $plugin -DestinationPath 'dist/HaviRadio-Windows-x64.zip' -Force
Write-Host 'Built dist/HaviRadio-Windows-x64.zip'
