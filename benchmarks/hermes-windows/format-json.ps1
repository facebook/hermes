param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Path
)

if (-not (Test-Path $Path)) {
    Write-Error "File not found: $Path"
    exit 1
}

$raw = Get-Content -Path $Path -Raw

# Strip PowerShell's alignment-based indentation to get compact JSON
$compact = $raw -replace '(?m)^\s+' -replace "`n", '' -replace "`r", ''

# Re-format with consistent 4-space indent
$indent = 0
$inString = $false
$sb = [System.Text.StringBuilder]::new()

for ($i = 0; $i -lt $compact.Length; $i++) {
    $c = $compact[$i]

    if ($c -eq '"' -and ($i -eq 0 -or $compact[$i - 1] -ne '\')) {
        $inString = !$inString
        [void]$sb.Append($c)
        continue
    }

    if ($inString) {
        [void]$sb.Append($c)
        continue
    }

    switch ($c) {
        '{' { [void]$sb.Append("{`n"); $indent++; [void]$sb.Append('    ' * $indent) }
        '}' { $indent--; [void]$sb.Append("`n"); [void]$sb.Append('    ' * $indent); [void]$sb.Append('}') }
        '[' { [void]$sb.Append("[`n"); $indent++; [void]$sb.Append('    ' * $indent) }
        ']' { $indent--; [void]$sb.Append("`n"); [void]$sb.Append('    ' * $indent); [void]$sb.Append(']') }
        ',' { [void]$sb.Append(",`n"); [void]$sb.Append('    ' * $indent) }
        ':' { [void]$sb.Append(': ') }
        default { if ($c -notmatch '\s') { [void]$sb.Append($c) } }
    }
}

$sb.ToString() | Set-Content -Path $Path -Encoding UTF8
Write-Host "Formatted: $Path"
