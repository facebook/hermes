param(
    [switch]$Release = $false
)

function Get-ScriptDirectory { Split-Path $MyInvocation.ScriptName }
$scriptPath = Join-Path (Get-ScriptDirectory) 'configure.py'
$vsprojPath = Join-Path (Get-ScriptDirectory) '../../build/ALL_BUILD.vcxproj'
python $scriptPath --build-system='Visual Studio 16 2019' --cmake-flags='-A x64'
$confiugration = If ($Release -eq $true) {"/p:Configuration=Release"} Else {"/p:Configuration=Debug"}
MSBuild.exe $vsprojPath $confiugration