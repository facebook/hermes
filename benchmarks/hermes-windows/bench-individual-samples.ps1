param(
    [Parameter(Mandatory = $true)]
    [Alias("c")]
    [int]$count,

    [Parameter(Mandatory = $true)]
    [string]$output
)

if ($count -lt 1) {
    Write-Error "-c must be >= 1"
    exit 1
}

$benchIndividual = Join-Path $PSScriptRoot "bench-individual.ps1"

if (-not (Test-Path $benchIndividual)) {
    Write-Error "bench-individual.ps1 not found at: $benchIndividual"
    exit 1
}

# ---------------------------------------------------------------------------
# Run bench-individual.ps1 $count times, saving each to $output.0 .. $output.N
# ---------------------------------------------------------------------------
$tempFiles = @()
for ($i = 0; $i -lt $count; $i++) {
    $tempFile = "$output.$i"
    $tempFiles += $tempFile
    Write-Host "=== Run $($i + 1) of $count ==="
    & $benchIndividual -output $tempFile
    Write-Host ""
}

# ---------------------------------------------------------------------------
# Load all runs and collect samples per benchmark
# ---------------------------------------------------------------------------
$allSamples = [ordered]@{}

foreach ($tempFile in $tempFiles) {
    $json = Get-Content -Path $tempFile -Raw | ConvertFrom-Json
    foreach ($prop in $json.PSObject.Properties) {
        $key = $prop.Name
        $val = [double]$prop.Value
        if (-not $allSamples.Contains($key)) {
            $allSamples[$key] = @()
        }
        $allSamples[$key] += $val
    }
}

# ---------------------------------------------------------------------------
# Compute mean and stdev, build output matching benchmark-test-suits format
# ---------------------------------------------------------------------------
$results = [ordered]@{}

foreach ($key in $allSamples.Keys) {
    $samples = $allSamples[$key]
    $n = $samples.Count

    # mean
    $sum = 0.0
    foreach ($s in $samples) { $sum += $s }
    $mean = $sum / $n

    # population stdev (matching bench-runner's numpy-style stdev)
    $sumSqDiff = 0.0
    foreach ($s in $samples) { $sumSqDiff += ($s - $mean) * ($s - $mean) }
    if ($n -gt 1) {
        $stdev = [math]::Sqrt($sumSqDiff / ($n - 1))
    } else {
        $stdev = 0.0
    }

    $results[$key] = [ordered]@{
        totalTime = [ordered]@{
            mean    = [math]::Round($mean, 1)
            stdev   = $stdev
            samples = @($samples)
        }
    }
}

# ---------------------------------------------------------------------------
# Write JSON
# ---------------------------------------------------------------------------
$outputObj = [ordered]@{
    results = $results
}

$outputObj | ConvertTo-Json -Depth 5 | Set-Content -Path $output -Encoding UTF8

# ---------------------------------------------------------------------------
# Clean up temp files
# ---------------------------------------------------------------------------
foreach ($tempFile in $tempFiles) {
    Remove-Item -Path $tempFile -Force
}

Write-Host "Results written to: $output"
Write-Host "$($results.Count) benchmarks, $count samples each."
