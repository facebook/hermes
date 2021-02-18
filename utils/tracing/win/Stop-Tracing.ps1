<#
  .SYNOPSIS
  Stops a Trace recording session previously started by running Start-Tracing.ps1, and optionally start analysis.

  .DESCRIPTION
  Stops a Trace recording session previously started by running Start-Tracing.ps1, and optionally start analysis.

  .PARAMETER NoAnalysis
  Include this switch if the script should only stop the session. This can be useful when including this call in other scripts, or to defer analysis to a later time.

  .PARAMETER OutputPath
  None.

  .INPUTS
  None. You cannot pipe objects to this script.

  .OUTPUTS
  None. This script does not generate any output.

  .EXAMPLE
  PS> .\utils\tracing\win\Start-Tracing.ps1 -IncludeGeneralTrace

  .EXAMPLE
  PS> .\utils\tracing\win\Start-Tracing.ps1
#>


[CmdletBinding()]
Param(
    [Parameter(ParameterSetName='Default', Position=0, ValueFromPipeline=$true, ValueFromPipelineByPropertyName=$true)]
    [switch]$NoAnalysis
)

begin {
    Set-StrictMode -Version Latest

    $scriptPath = Split-Path -parent $MyInvocation.MyCommand.Definition
    $timeStamp = [DateTime]::Now.ToString("yyyyMMdd_HHmmss") 
    $etlFileName = "rnw_" + $timeStamp + ".etl"
    $etlFilePath = Join-Path -Path $scriptPath -ChildPath $etlFileName
    $wprArgs = "-stop", $etlFilePath
}

process{
    if (!(Get-Command "wpr.exe" -ErrorAction SilentlyContinue)) { 
        throw "
    ################################################################
    wpr.exe (Windows Performance Recorder) unavailable ! 
    Ensure that WPT (Windows Performance Toolkit) is installed and its tools are added to the path 
    ################################################################"
    }

    Write-Host "wpr.exe $wprArgs"
    & Start-Process -Verb RunAs "wpr.exe" -ArgumentList $wprArgs -PassThru -Wait

    if (!(Get-Command "wpa.exe" -ErrorAction SilentlyContinue)) { 
        Write-Host "wpa.exe (Windows Performance Analyzer) unavailable !"
        Write-Host "Ensure that WPT (Windows Performance Toolkit) is installed and its tools are added to the path "
    }

    if ($NoAnalysis.IsPresent) {
        Write-Host "To analys this trace, install WPT and run: 'wpa $etlFilePath'"
    }
    else {

        if (!(Get-Command "wpa.exe" -ErrorAction SilentlyContinue)) { 
            throw "
            ################################################################
            wpa.exe (Windows Performance Analyzer) unavailable ! 
            Ensure that WPT (Windows Performance Toolkit) is installed and its tools are added to the path 
            ################################################################"
        }

        $wpaArgs = $etlFilePath
        Write-Host "wpa.exe $wpaArgs"
        & Start-Process -Verb RunAs "wpa.exe" -ArgumentList $wpaArgs
    }
}
