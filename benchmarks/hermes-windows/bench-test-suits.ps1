param(
    [Parameter(Mandatory = $true)]
    [Alias("c")]
    [int]$count,

    [Parameter(Mandatory = $true)]
    [Alias("l")]
    [string]$label,

    [Parameter(Mandatory = $true)]
    [string]$output
)

if ($count -lt 1) {
    Write-Error "-c must be >= 1"
    exit 1
}

$benchmarksDir = Join-Path $PSScriptRoot ".."
$repoRoot = Join-Path $benchmarksDir ".."
$benchRunner = Join-Path (Join-Path $benchmarksDir "bench-runner") "bench-runner.py"
$hermes = Join-Path (Join-Path (Join-Path (Join-Path $repoRoot "build") "ninja-clang-release") "bin") "hermes.exe"

if (-not (Test-Path $hermes)) {
    Write-Error "hermes.exe not found at: $hermes"
    exit 1
}

$hermes = (Resolve-Path $hermes).Path

if (-not (Test-Path $benchRunner)) {
    Write-Error "bench-runner.py not found at: $benchRunner"
    exit 1
}

$benchRunner = (Resolve-Path $benchRunner).Path

# tsc category is excluded — tsc-vs-chunk.js is not available in this repo
Write-Host "Running test suites: v8 octane micros"
Write-Host "hermes: $hermes"
Write-Host "count: $count"
Write-Host "label: $label"
Write-Host ""

& python3 $benchRunner --hermes -b $hermes --cats v8 octane micros -c $count -l $label -f json --out $output

if ($LASTEXITCODE -ne 0) {
    Write-Error "bench-runner.py failed with exit code $LASTEXITCODE"
    exit 1
}

Write-Host ""
Write-Host "Results written to: $output"
