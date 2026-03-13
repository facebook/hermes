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

$benchTestSuits = Join-Path $PSScriptRoot "bench-test-suits.ps1"
$benchIndividualSamples = Join-Path $PSScriptRoot "bench-individual-samples.ps1"
$formatJson = Join-Path $PSScriptRoot "format-json.ps1"

$tempTestSuits = "$output.testsuits"
$tempIndividual = "$output.individual"

# ---------------------------------------------------------------------------
# Run bench-test-suits.ps1
# ---------------------------------------------------------------------------
Write-Host "=========================================="
Write-Host "  Running test suites"
Write-Host "=========================================="
& $benchTestSuits -c $count -l $label -output $tempTestSuits
Write-Host ""

# ---------------------------------------------------------------------------
# Run bench-individual-samples.ps1
# ---------------------------------------------------------------------------
Write-Host "=========================================="
Write-Host "  Running individual benchmarks"
Write-Host "=========================================="
& $benchIndividualSamples -c $count -output $tempIndividual
Write-Host ""

# ---------------------------------------------------------------------------
# Merge: take test-suits JSON as base, add individual results into it
# ---------------------------------------------------------------------------
Write-Host "Merging results..."

$testSuitsJson = Get-Content -Path $tempTestSuits -Raw | ConvertFrom-Json
$individualJson = Get-Content -Path $tempIndividual -Raw | ConvertFrom-Json

# test-suits output has "runtime" and "results" at top level
# individual output has "results" at top level
# Merge individual results into test-suits results
foreach ($prop in $individualJson.results.PSObject.Properties) {
    $testSuitsJson.results | Add-Member -NotePropertyName $prop.Name -NotePropertyValue $prop.Value -Force
}

$testSuitsJson | ConvertTo-Json -Depth 5 | Set-Content -Path $output -Encoding UTF8

# Format with consistent indentation
& $formatJson $output

# ---------------------------------------------------------------------------
# Clean up temp files
# ---------------------------------------------------------------------------
Remove-Item -Path $tempTestSuits -Force
Remove-Item -Path $tempIndividual -Force

Write-Host ""
Write-Host "All results written to: $output"
