import { spawnSync } from 'node:child_process';
import { existsSync, readFileSync, writeFileSync, unlinkSync } from 'node:fs';
import { join, dirname, resolve, delimiter } from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseArgs } from 'node:util';
import { tmpdir } from 'node:os';

// Resolve paths relative to this script (benchmarks/hermes-windows/)
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);
const benchmarksDir = resolve(__dirname, '..');
const repoRoot = resolve(benchmarksDir, '..');
// Binary path — set after CLI arg parsing.
let hermes: string;

// ---------------------------------------------------------------------------
// Interfaces
// ---------------------------------------------------------------------------

interface Benchmark {
  results: { [key: string]: number };
}

interface SampledBenchmark {
  results: {
    [key: string]: {
      totalTime: {
        mean: number;
        stdev: number;
        samples: number[];
      };
    };
  };
}

interface TestSuiteBenchmark extends SampledBenchmark {
  runtime: string;
}

// ---------------------------------------------------------------------------
// Parsers — each takes combined stdout, returns ms or null
// ---------------------------------------------------------------------------

type Parser = (stdout: string) => number | null;

// "Time: 1234" or "Time:1234"
const pTimeColon: Parser = (s) => {
  const m = s.match(/Time:\s*(\d+)/);
  return m ? parseInt(m[1]) : null;
};

// "time=1234"
const pTimeEquals: Parser = (s) => {
  const m = s.match(/time=(\d+)/);
  return m ? parseInt(m[1]) : null;
};

// "1234 ms 10000 iterations"
const pMsIterations: Parser = (s) => {
  const m = s.match(/(\d+)\s+ms\s+\d+\s+iterations/);
  return m ? parseInt(m[1]) : null;
};

// "exec time:  1434 ms" or "exec time:  1434.5 ms"
const pExecTime: Parser = (s) => {
  const m = s.match(/exec time:\s+([\d.]+)\s+ms/);
  return m ? Math.round(parseFloat(m[1])) : null;
};

// Last line matching "1234 ms" (for MiniReact HTML output followed by timing)
const pTrailingMs: Parser = (s) => {
  const lines = s.split('\n');
  for (let i = lines.length - 1; i >= 0; i--) {
    const m = lines[i].match(/^\s*(\d+)\s+ms\s*$/);
    if (m) return parseInt(m[1]);
  }
  return null;
};

// ---------------------------------------------------------------------------
// Benchmark definitions
// ---------------------------------------------------------------------------

interface BenchmarkDef {
  path: string;
  flags?: string[];
  parser?: Parser;
}

const benchmarks: BenchmarkDef[] = [
  // micros/
  { path: 'micros/getNodeById.js', parser: pTimeColon },
  { path: 'micros/setInsert.js', parser: pTimeColon },
  { path: 'micros/stringify-number.js', parser: pTimeColon },
  { path: 'micros/typed-array-sort.js' }, // multi-line per-type output, measure wall-clock

  // jit-benches/
  { path: 'jit-benches/idisp.js', parser: pTimeColon },
  { path: 'jit-benches/idispn.js', parser: pTimeColon },

  // many-subclasses/
  { path: 'many-subclasses/many.js', parser: pTimeEquals },
  { path: 'many-subclasses/many-sh-1.js', flags: ['-typed'], parser: pTimeEquals },
  { path: 'many-subclasses/many-sh-2.js', flags: ['-typed'], parser: pTimeEquals },
  { path: 'many-subclasses/many-sh-3.js', flags: ['-typed'], parser: pTimeEquals },
  { path: 'many-subclasses/many-sh-4.js', flags: ['-typed'], parser: pTimeEquals },

  // map-objects/
  { path: 'map-objects/map-objects-untyped.js', parser: pMsIterations },
  { path: 'map-objects/map-objects-typed.js', flags: ['-typed'], parser: pMsIterations },

  // map-strings/
  { path: 'map-strings/map-strings-untyped.js', parser: pMsIterations },
  { path: 'map-strings/map-strings-typed.js', flags: ['-typed'], parser: pMsIterations },

  // nbody/ (no timing output, measure wall-clock)
  { path: 'nbody/original/nbody.js' },
  { path: 'nbody/fully-typed/nbody.js', flags: ['-typed'] },
  { path: 'nbody/fully-typed/nbody.ts', flags: ['-parse-ts'] },

  // string-switch/ (multi-line output, measure wall-clock)
  { path: 'string-switch/plain/bench.js' },

  // raytracer/
  { path: 'raytracer/original/bench-raytracer.js', parser: pExecTime },
  { path: 'raytracer/original/raytracer.ts', flags: ['-parse-ts'], parser: pExecTime },

  // MiniReact/
  { path: 'MiniReact/no-objects/out/simple-stripped.js', parser: pTrailingMs },
  { path: 'MiniReact/no-objects/out/simple-lowered.js', parser: pTrailingMs },
  { path: 'MiniReact/no-objects/out/music-stripped.js' },  // no timing, measure
  { path: 'MiniReact/no-objects/out/music-lowered.js' },   // no timing, measure
  { path: 'MiniReact/no-deps/stripped/MiniReact.js' },     // no output, measure

  // MiniReact Flow-typed variants — use -parse-flow (not -typed) because
  // -typed segfaults due to unimplemented CheckedTypeCastInst in HBC ISel
  // (see lib/BCGen/HBC/ISel.cpp generateCheckedTypeCastInst TODO).
  // MiniReact/original/MiniReact.js is not listed — it is a library with no
  // entry point, not a runnable benchmark.
  { path: 'MiniReact/no-deps/MiniReact.js', flags: ['-parse-flow'] },
  { path: 'MiniReact/no-objects/out/simple.js', flags: ['-parse-flow'], parser: pTrailingMs },
  { path: 'MiniReact/no-objects/out/music.js', flags: ['-parse-flow'] },

  // widgets/
  { path: 'widgets/simple-classes/widgets.js', flags: ['-typed'], parser: pMsIterations },
  { path: 'widgets/original/es5/widgets.js', parser: pMsIterations },
  { path: 'widgets/single-file/es5/widgets.js', parser: pMsIterations },
];

// ---------------------------------------------------------------------------
// Static Hermes helpers — two-phase compile-then-run so that compilation
// time is excluded from wall-clock measurements.
// ---------------------------------------------------------------------------

let shermesExecEnv: Record<string, string> | undefined;

/// Build a process env with DLL directories on PATH so that executables
/// compiled by shermes can find hermesvm.dll and shermes_console.dll.
function getShermesExecEnv(): Record<string, string> {
  if (shermesExecEnv) return shermesExecEnv;
  const buildDir = resolve(dirname(hermes), '..');
  const libDir = join(buildDir, 'lib');
  const consoleDir = join(buildDir, 'tools', 'shermes');
  shermesExecEnv = { ...process.env } as Record<string, string>;
  shermesExecEnv['PATH'] =
    libDir + delimiter + consoleDir + delimiter + (process.env['PATH'] || '');
  return shermesExecEnv;
}

let shermesSeq = 0;

/// Compile a JS file to a native executable via shermes.
/// Returns the path to the temp .exe, or null on failure.
function shermesCompile(def: BenchmarkDef, jsPath: string): string | null {
  const exePath = join(tmpdir(), `shermes-bench-${process.pid}-${shermesSeq++}.exe`);
  const args = ['-O', ...(def.flags || []), '-o', exePath, jsPath];
  const result = spawnSync(hermes, args, {
    encoding: 'utf-8',
    stdio: ['pipe', 'pipe', 'pipe'],
  });
  if (result.status !== 0) {
    console.warn(`  ${def.path} compile failed: ${(result.stderr || '').trim()}`);
    return null;
  }
  return exePath;
}

// ---------------------------------------------------------------------------
// Run a single benchmark
// ---------------------------------------------------------------------------

function runBenchmark(def: BenchmarkDef): [string, number] | null {
  const fullPath = join(benchmarksDir, def.path);
  if (!existsSync(fullPath)) {
    console.warn(`  SKIP ${def.path} (file not found)`);
    return null;
  }

  const resolvedPath = resolve(fullPath);
  const key = def.path;

  // In static mode, pre-compile to a native exe so wall-clock measurements
  // reflect only execution time, not compilation.
  if (mode === 'static') {
    const exePath = shermesCompile(def, resolvedPath);
    if (!exePath) return null;
    const env = getShermesExecEnv();
    try {
      return runCompiledBenchmark(def, key, exePath, env);
    } finally {
      try { unlinkSync(exePath); } catch {}
    }
  }

  const args = [...(def.flags || []), resolvedPath];
  try {
    if (def.parser) {
      const result = spawnSync(hermes, args, {
        encoding: 'utf-8',
        stdio: ['pipe', 'pipe', 'ignore'],
      });
      const stdout = result.stdout || '';
      const time = def.parser(stdout);
      if (time !== null && time > 0) {
        console.log(`  ${def.path} ${time} ms`);
        return [key, time];
      } else {
        console.warn(`  ${def.path} failed to parse time`);
        return null;
      }
    } else {
      const start = Date.now();
      spawnSync(hermes, args, { stdio: 'ignore' });
      const elapsed = Date.now() - start;
      console.log(`  ${def.path} ${elapsed} ms (measured)`);
      return [key, elapsed];
    }
  } catch (e) {
    console.warn(`  ERROR ${def.path}: ${e}`);
    return null;
  }
}

/// Run an already-compiled native exe and collect timing.
function runCompiledBenchmark(
  def: BenchmarkDef,
  key: string,
  exePath: string,
  env: Record<string, string>,
): [string, number] | null {
  try {
    if (def.parser) {
      const result = spawnSync(exePath, [], {
        encoding: 'utf-8',
        stdio: ['pipe', 'pipe', 'ignore'],
        env,
      });
      const stdout = result.stdout || '';
      const time = def.parser(stdout);
      if (time !== null && time > 0) {
        console.log(`  ${def.path} ${time} ms`);
        return [key, time];
      } else {
        console.warn(`  ${def.path} failed to parse time`);
        return null;
      }
    } else {
      const start = Date.now();
      spawnSync(exePath, [], { stdio: 'ignore', env });
      const elapsed = Date.now() - start;
      console.log(`  ${def.path} ${elapsed} ms (measured)`);
      return [key, elapsed];
    }
  } catch (e) {
    console.warn(`  ERROR ${def.path}: ${e}`);
    return null;
  }
}

// ---------------------------------------------------------------------------
// benchIndividual — run each individual benchmark once
// ---------------------------------------------------------------------------

function benchIndividual(): Benchmark {
  if (!existsSync(hermes)) {
    console.error(`Binary not found at: ${hermes}`);
    process.exit(1);
  }

  console.log('Running individual benchmarks...');
  console.log(`binary: ${hermes}`);
  console.log('');

  const results: { [key: string]: number } = {};
  let currentSection = '';

  for (const def of benchmarks) {
    const section = def.path.split('/')[0];
    if (section !== currentSection) {
      console.log(`${section}/`);
      currentSection = section;
    }
    const result = runBenchmark(def);
    if (result) {
      results[result[0]] = result[1];
    }
  }

  console.log('');
  console.log(`${Object.keys(results).length} benchmarks completed.`);
  return { results };
}

// ---------------------------------------------------------------------------
// benchIndividualSamples — run benchIndividual N times, compute stats
// ---------------------------------------------------------------------------

function benchIndividualSamples(count: number): SampledBenchmark {
  if (count < 1) {
    console.error('-c must be >= 1');
    process.exit(1);
  }

  const allSamples: { [key: string]: number[] } = {};

  for (let i = 0; i < count; i++) {
    console.log(`=== Run ${i + 1} of ${count} ===`);
    const run = benchIndividual();

    for (const [key, val] of Object.entries(run.results)) {
      if (!allSamples[key]) allSamples[key] = [];
      allSamples[key].push(val);
    }
  }

  const results: SampledBenchmark['results'] = {};

  for (const [key, samples] of Object.entries(allSamples)) {
    const n = samples.length;
    const sum = samples.reduce((a, b) => a + b, 0);
    const mean = sum / n;

    // sample stdev (Bessel's correction, n-1) matching bench-individual-samples.ps1
    let stdev = 0;
    if (n > 1) {
      const sumSqDiff = samples.reduce((a, s) => a + (s - mean) ** 2, 0);
      stdev = Math.sqrt(sumSqDiff / (n - 1));
    }

    results[key] = {
      totalTime: {
        mean: Math.round(mean * 10) / 10,
        stdev,
        samples,
      },
    };
  }

  console.log(`${Object.keys(results).length} benchmarks, ${count} samples each.`);
  return { results };
}

// ---------------------------------------------------------------------------
// benchTestSuites — run bench-runner.py for v8, octane, micros categories
// ---------------------------------------------------------------------------

function benchTestSuites(count: number, label: string): TestSuiteBenchmark {
  if (count < 1) {
    console.error('-c must be >= 1');
    process.exit(1);
  }

  if (!existsSync(hermes)) {
    console.error(`Binary not found at: ${hermes}`);
    process.exit(1);
  }

  const benchRunner = resolve(benchmarksDir, 'bench-runner', 'bench-runner.py');
  if (!existsSync(benchRunner)) {
    console.error(`bench-runner.py not found at: ${benchRunner}`);
    process.exit(1);
  }

  // tsc category is excluded -- tsc-vs-chunk.js is not available in this repo
  console.log('Running test suites: v8 octane micros');
  console.log(`binary: ${hermes}`);
  console.log(`count: ${count}`);
  console.log(`label: ${label}`);
  console.log('');

  const tempOut = join(tmpdir(), `bench-runner-${process.pid}.json`);

  try {
    const result = spawnSync('python3', [
      benchRunner, mode === 'static' ? '--shermes' : '--hermes', '-b', hermes,
      '--cats', 'v8', 'octane', 'micros',
      '-c', String(count), '-l', label,
      '-f', 'json', '--out', tempOut,
    ], { stdio: 'inherit' });

    if (result.status !== 0) {
      console.error(`bench-runner.py failed with exit code ${result.status}`);
      process.exit(1);
    }

    const json = JSON.parse(readFileSync(tempOut, 'utf-8')) as TestSuiteBenchmark;
    console.log('');
    console.log(`Results: ${Object.keys(json.results).length} test suite benchmarks`);
    return json;
  } finally {
    if (existsSync(tempOut)) unlinkSync(tempOut);
  }
}

// ---------------------------------------------------------------------------
// benchAll — run test suites + individual benchmarks, merge results
// ---------------------------------------------------------------------------

function benchAll(count: number, label: string): TestSuiteBenchmark {
  if (count < 1) {
    console.error('-c must be >= 1');
    process.exit(1);
  }

  console.log('==========================================');
  console.log('  Running test suites');
  console.log('==========================================');
  const testSuites = benchTestSuites(count, label);
  console.log('');

  console.log('==========================================');
  console.log('  Running individual benchmarks');
  console.log('==========================================');
  const individual = benchIndividualSamples(count);
  console.log('');

  // Merge individual results into test-suite results
  console.log('Merging results...');
  for (const [key, val] of Object.entries(individual.results)) {
    testSuites.results[key] = val;
  }

  return testSuites;
}

// ---------------------------------------------------------------------------
// CLI
// ---------------------------------------------------------------------------

const { values } = parseArgs({
  options: {
    count: { type: 'string', short: 'c' },
    label: { type: 'string', short: 'l' },
    output: { type: 'string', short: 'o' },
    dynamic: { type: 'boolean', default: false },
    static: { type: 'boolean', default: false },
    binary: { type: 'string', short: 'b' },
  },
});

const count = parseInt(values.count || '1');
const label = values.label || 'test';
const output = values.output;
const useDynamic = values.dynamic!;
const useStatic = values.static!;

if (useDynamic && useStatic) {
  console.error('Error: --dynamic and --static cannot be used together.');
  process.exit(1);
}

if (!output) {
  console.error(
    'Usage: node bench.ts -c <count> -l <label> -o <output.json> [--dynamic|--static] [--binary <path>]',
  );
  process.exit(1);
}

// Default to dynamic (hermes.exe) unless --static is specified.
const mode: 'dynamic' | 'static' = useStatic ? 'static' : 'dynamic';
if (values.binary) {
  hermes = resolve(values.binary);
} else {
  const binaryName = mode === 'static' ? 'shermes.exe' : 'hermes.exe';
  hermes = resolve(repoRoot, 'build', 'ninja-clang-release', 'bin', binaryName);
}

const result = benchAll(count, label);
writeFileSync(output, JSON.stringify(result, undefined, 4) + '\n');
console.log('');
console.log(`All results written to: ${output}`);
