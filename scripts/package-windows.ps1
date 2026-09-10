$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
$plugin = [IO.Path]::GetFullPath('build-windows/HaviRadio_artefacts/Release/VST3/RadioBox.vst3')
if (-not (Test-Path "$plugin/Contents/x86_64-win/RadioBox.vst3")) {
    throw 'No compiled Windows plug-in. Run scripts/build-windows.ps1 first.'
}
$nsis = Join-Path ${env:ProgramFiles(x86)} 'NSIS/makensis.exe'
if (-not (Test-Path $nsis)) { throw 'Install NSIS 3 before packaging.' }
New-Item -ItemType Directory -Force -Path 'dist' | Out-Null
$output = [IO.Path]::GetFullPath('dist/RadioBox-Windows-x64-Setup.exe')
& $nsis "/DPLUGIN_DIR=$plugin" "/DOUTPUT_FILE=$output" 'installers/windows.nsi'
if ($LASTEXITCODE -ne 0) { throw 'NSIS packaging failed.' }
if (-not (Test-Path $output)) { throw 'Installer output missing.' }
Write-Host "Created $output (unsigned development installer)"
