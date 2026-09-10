$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
$plugin = [IO.Path]::GetFullPath('build-aax-windows/RadioBox_artefacts/Release/AAX/RadioBox.aaxplugin')
if (-not (Test-Path "$plugin/Contents/x64/RadioBox.aaxplugin")) { throw 'Signed 64-bit AAX build missing.' }
$nsis = Join-Path ${env:ProgramFiles(x86)} 'NSIS/makensis.exe'
if (-not (Test-Path $nsis)) { throw 'Install NSIS 3 before packaging.' }
New-Item -ItemType Directory -Force -Path dist | Out-Null
$output = [IO.Path]::GetFullPath('dist/RadioBox-AAX-Windows-x64-Setup.exe')
& $nsis "/DPLUGIN_DIR=$plugin" "/DOUTPUT_FILE=$output" installers/windows-aax.nsi
if ($LASTEXITCODE -ne 0) { throw 'AAX installer packaging failed.' }
Write-Host "Created $output"
