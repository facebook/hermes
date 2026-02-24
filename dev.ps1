# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

<#
.SYNOPSIS
  Developer task runner for hermes-windows.

.DESCRIPTION
  A single entry point for common development tasks.
  Run without arguments or with -? to see available commands.

.EXAMPLE
  .\dev build --help
  .\dev fork-sync --dep icu-small
  .\dev fork-sync --dep icu-small --status
#>

param(
    [Parameter(Position = 0)]
    [string]$Command,

    [Parameter(Position = 1, ValueFromRemainingArguments)]
    [string[]]$Args
)

$ErrorActionPreference = 'Stop'
$ScriptDir = $PSScriptRoot

# =============================================================================
# Helpers
# =============================================================================

function Ensure-ForkSync {
    $stamp = Join-Path $ScriptDir 'tools\fork-sync\node_modules\.package-lock.json'
    $pkg = Join-Path $ScriptDir 'tools\fork-sync\package.json'

    if (-not (Test-Path $stamp) -or
        (Get-Item $pkg).LastWriteTime -gt (Get-Item $stamp).LastWriteTime) {
        Write-Host 'Installing fork-sync dependencies...'
        Push-Location (Join-Path $ScriptDir 'tools\fork-sync')
        try { npm install } finally { Pop-Location }
    }
}

function Show-Help {
    Write-Host ''
    Write-Host '  hermes-windows developer tasks'
    Write-Host '  =============================='
    Write-Host ''
    Write-Host '  .\dev build [args]              Build Hermes for Windows'
    Write-Host '  .\dev build-fork-sync           Install fork-sync dependencies'
    Write-Host '  .\dev fork-sync [args]           Run fork-sync tool'
    Write-Host ''
    Write-Host '  Examples:'
    Write-Host '    .\dev fork-sync --dep icu-small'
    Write-Host '    .\dev fork-sync --dep icu-small --status'
    Write-Host '    .\dev fork-sync --dep icu-small --continue'
    Write-Host '    .\dev fork-sync --dep icu-small --abort'
    Write-Host '    .\dev fork-sync --help'
    Write-Host '    .\dev build --help'
    Write-Host ''
}

# =============================================================================
# Commands
# =============================================================================

switch ($Command) {
    'build' {
        & node (Join-Path $ScriptDir '.ado\scripts\build.js') @Args
    }

    'build-fork-sync' {
        Ensure-ForkSync
        Write-Host 'fork-sync dependencies installed.'
    }

    'fork-sync' {
        Ensure-ForkSync
        & node (Join-Path $ScriptDir 'tools\fork-sync\sync.ts') @Args
    }

    default {
        Show-Help
    }
}

exit $LASTEXITCODE
