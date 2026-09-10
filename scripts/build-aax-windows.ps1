$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
if (-not $env:AAX_SDK_DIR) { throw 'Set AAX_SDK_DIR to your licensed Avid AAX SDK folder.' }
function Run-Native {
    param([string]$Exe, [string[]]$Arguments)
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe failed with exit code $LASTEXITCODE" }
}
Run-Native 'cmake' @('-S','.', '-B','build-aax-windows','-G','Visual Studio 17 2022','-A','x64',"-DAAX_SDK_DIR=$env:AAX_SDK_DIR",'-DRADIOBOX_BUILD_AAX=ON')
Run-Native 'cmake' @('--build','build-aax-windows','--config','Release','--target','RadioBox_AAX','RadioBoxDSPTests','RadioBoxIntegrationTests','--parallel','2')
Run-Native 'ctest' @('--test-dir','build-aax-windows','-C','Release','--output-on-failure')
$plugin = 'build-aax-windows/RadioBox_artefacts/Release/AAX/RadioBox.aaxplugin'
if (-not (Test-Path "$plugin/Contents/x64/RadioBox.aaxplugin")) { throw 'Expected 64-bit AAX binary missing.' }
Write-Host "Built $plugin. PACE signing is required before use in a retail Pro Tools installation."
