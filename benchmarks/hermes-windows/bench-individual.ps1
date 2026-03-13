param(
    [Parameter(Mandatory = $true)]
    [string]$output
)

# Paths anchored to script location (benchmarks/hermes-windows/)
$benchmarksDir = Join-Path $PSScriptRoot ".."
$repoRoot = Join-Path $benchmarksDir ".."
$hermes = Join-Path (Join-Path (Join-Path (Join-Path $repoRoot "build") "ninja-clang-release") "bin") "hermes.exe"

if (-not (Test-Path $hermes)) {
    Write-Error "hermes.exe not found at: $hermes"
    exit 1
}

$hermes = (Resolve-Path $hermes).Path
$results = [ordered]@{}

# ---------------------------------------------------------------------------
# Run a benchmark. If $Parser is provided, parse time from stdout.
# If $Parser is $null, measure wall-clock time with Measure-Command.
# ---------------------------------------------------------------------------
function Run-Benchmark {
    param(
        [string]$RelativePath,
        [string[]]$Flags = @(),
        [scriptblock]$Parser = $null
    )

    $fullPath = Join-Path $script:benchmarksDir $RelativePath
    if (-not (Test-Path $fullPath)) {
        Write-Warning "  SKIP $RelativePath (file not found)"
        return
    }

    Write-Host "  $RelativePath " -NoNewline

    $hermesArgs = @()
    if ($Flags.Count -gt 0) { $hermesArgs += $Flags }
    $hermesArgs += (Resolve-Path $fullPath).Path

    try {
        if ($Parser) {
            $stdout = & $script:hermes @hermesArgs 2>$null
            $combined = $stdout -join "`n"
            $time = & $Parser $combined
            if ($null -ne $time -and $time -gt 0) {
                $key = $RelativePath -replace '\\', '/'
                $script:results[$key] = $time
                Write-Host "$time ms"
            }
            else {
                Write-Warning "failed to parse time"
            }
        }
        else {
            $elapsed = Measure-Command {
                & $script:hermes @hermesArgs 2>$null | Out-Null
            }
            $time = [math]::Round($elapsed.TotalMilliseconds)
            $key = $RelativePath -replace '\\', '/'
            $script:results[$key] = $time
            Write-Host "$time ms (measured)"
        }
    }
    catch {
        Write-Warning "ERROR $RelativePath : $_"
    }
}

# ---------------------------------------------------------------------------
# Parsers — each takes the combined stdout string, returns [int] ms or $null
# ---------------------------------------------------------------------------

# "Time: 1234" or "Time:1234"
$pTimeColon = {
    param($s)
    if ($s -match 'Time:\s*(\d+)') { [int]$Matches[1] } else { $null }
}

# "time=1234"
$pTimeEquals = {
    param($s)
    if ($s -match 'time=(\d+)') { [int]$Matches[1] } else { $null }
}

# "1234 ms 10000 iterations"
$pMsIterations = {
    param($s)
    if ($s -match '(\d+)\s+ms\s+\d+\s+iterations') { [int]$Matches[1] } else { $null }
}

# "exec time:  1434 ms" or "exec time:  1434.5 ms"
$pExecTime = {
    param($s)
    if ($s -match 'exec time:\s+([\d.]+)\s+ms') { [math]::Round([double]$Matches[1]) } else { $null }
}

# Last line matching "1234 ms" (for MiniReact HTML output followed by timing)
$pTrailingMs = {
    param($s)
    $lines = $s -split "`n"
    for ($i = $lines.Count - 1; $i -ge 0; $i--) {
        if ($lines[$i] -match '^\s*(\d+)\s+ms\s*$') {
            return [int]$Matches[1]
        }
    }
    $null
}

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------

Write-Host "Running individual benchmarks..."
Write-Host "hermes: $hermes"
Write-Host ""

# --- micros/ ---
Write-Host "micros/"
Run-Benchmark "micros/getNodeById.js" @() $pTimeColon
Run-Benchmark "micros/setInsert.js" @() $pTimeColon
Run-Benchmark "micros/stringify-number.js" @() $pTimeColon
Run-Benchmark "micros/typed-array-sort.js"  # multi-line per-type output, measure wall-clock

# --- jit-benches/ ---
Write-Host "jit-benches/"
Run-Benchmark "jit-benches/idisp.js" @() $pTimeColon
Run-Benchmark "jit-benches/idispn.js" @() $pTimeColon

# --- many-subclasses/ ---
Write-Host "many-subclasses/"
Run-Benchmark "many-subclasses/many.js" @() $pTimeEquals
Run-Benchmark "many-subclasses/many-sh-1.js" @("-typed") $pTimeEquals
Run-Benchmark "many-subclasses/many-sh-2.js" @("-typed") $pTimeEquals
Run-Benchmark "many-subclasses/many-sh-3.js" @("-typed") $pTimeEquals
Run-Benchmark "many-subclasses/many-sh-4.js" @("-typed") $pTimeEquals

# --- map-objects/ ---
Write-Host "map-objects/"
Run-Benchmark "map-objects/map-objects-untyped.js" @() $pMsIterations
Run-Benchmark "map-objects/map-objects-typed.js" @("-typed") $pMsIterations

# --- map-strings/ ---
Write-Host "map-strings/"
Run-Benchmark "map-strings/map-strings-untyped.js" @() $pMsIterations
Run-Benchmark "map-strings/map-strings-typed.js" @("-typed") $pMsIterations

# --- nbody/ (no timing output, measure wall-clock) ---
Write-Host "nbody/"
Run-Benchmark "nbody/original/nbody.js"
Run-Benchmark "nbody/fully-typed/nbody.js" @("-typed")
Run-Benchmark "nbody/fully-typed/nbody.ts" @("-parse-ts")

# --- string-switch/ (multi-line output, measure wall-clock) ---
Write-Host "string-switch/"
Run-Benchmark "string-switch/plain/bench.js"

# --- raytracer/ ---
Write-Host "raytracer/"
Run-Benchmark "raytracer/original/bench-raytracer.js" @() $pExecTime

# --- MiniReact/ ---
Write-Host "MiniReact/"
Run-Benchmark "MiniReact/no-objects/out/simple-stripped.js" @() $pTrailingMs
Run-Benchmark "MiniReact/no-objects/out/simple-lowered.js" @() $pTrailingMs
Run-Benchmark "MiniReact/no-objects/out/music-stripped.js"   # no timing, measure
Run-Benchmark "MiniReact/no-objects/out/music-lowered.js"    # no timing, measure
Run-Benchmark "MiniReact/no-deps/stripped/MiniReact.js"      # no output, measure

# --- widgets/ ---
Write-Host "widgets/"
Run-Benchmark "widgets/simple-classes/widgets.js" @("-typed") $pMsIterations

# ---------------------------------------------------------------------------
# Does not work — commented out
# ---------------------------------------------------------------------------
# Run-Benchmark "MiniReact/no-deps/MiniReact.js" @("-typed")          # unsupported type annotations
# Run-Benchmark "MiniReact/original/MiniReact.js" @("-typed")         # ES module imports
# Run-Benchmark "MiniReact/no-objects/out/simple.js" @("-typed")      # crashes
# Run-Benchmark "MiniReact/no-objects/out/music.js" @("-typed")       # unsupported type annotations
# Run-Benchmark "widgets/original/app_runner.js" @("-typed")          # ES module imports
# Run-Benchmark "widgets/single-file/widgets.js" @("-typed")          # unsupported type annotations
# Run-Benchmark "raytracer/original/raytracer.ts" @("-parse-ts")      # unsupported TS parameter properties

# ---------------------------------------------------------------------------
# Write JSON
# ---------------------------------------------------------------------------
Write-Host ""
$results | ConvertTo-Json | Set-Content -Path $output -Encoding UTF8
Write-Host "Results written to: $output"
Write-Host "$($results.Count) benchmarks completed."
