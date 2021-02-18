<#
  .SYNOPSIS
  Starts a Trace recording session enabling Hermes TraceLogging provider.

  .DESCRIPTION
  Starts a Trace recording session enabling Hermes TraceLogging provider.

  .PARAMETER IncludeGeneralTrace
  Include this switch if the session should record events from general system providers along with Hermes provider. This results in bigger trace files and it takes longer to load these traces into WPA.

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
   [switch]$IncludeGeneralTrace
)

begin {
   Set-StrictMode -Version Latest

   $scriptPath = Split-Path -parent $MyInvocation.MyCommand.Definition
   $wprpPath = Join-Path -Path $scriptPath -ChildPath "hermes.wprp"

   $wprArgs = "-start", $wprpPath

   if ($IncludeGeneralTrace.IsPresent) {
      $wprArgs += "-start", "GeneralProfile"
   }
}

process{

   if (!(Get-Command "wpr.exe" -ErrorAction SilentlyContinue)) { 
      throw "
      ################################################################
      wpr.exe (Windows Performance Recorder) unavailable ! 
      Ensure that WPT (Windows Performance Toolkit) is installed and its tools are added to the path 
      ################################################################"
   }

   Write-Host "wpr.exe $wprArgs -PassThru -Wait"
   & Start-Process -Verb RunAs "wpr.exe" -ArgumentList $wprArgs -PassThru -Wait

   Write-Host "To stop this trace and start analysis, run .\Stop-Tracing.ps1"
   Write-Host "To stop this trace only, run .\Stop-Tracing.ps1 -NoAnalysis $True"
   Write-Host
}
