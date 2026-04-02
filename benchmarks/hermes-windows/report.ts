import { readFileSync, writeFileSync } from 'node:fs';
import { parseArgs } from 'node:util';

// ---------------------------------------------------------------------------
// CLI
// ---------------------------------------------------------------------------

const { values } = parseArgs({
  options: {
    input: { type: 'string', short: 'i', multiple: true },
    output: { type: 'string', short: 'o' },
  },
});

const inputs = values.input;
const output = values.output;

if (!inputs || inputs.length === 0) {
  console.error('Error: at least one -i <input.json> is required.');
  console.error('Usage: report.ts -i <input.json> [-i <input2.json> ...] -o <output.md>');
  process.exit(1);
}

if (!output) {
  console.error('Error: exactly one -o <output.md> is required.');
  console.error('Usage: report.ts -i <input.json> [-i <input2.json> ...] -o <output.md>');
  process.exit(1);
}

// ---------------------------------------------------------------------------
// Interfaces
// ---------------------------------------------------------------------------

interface BenchResult {
  totalTime: {
    mean: number;
    stdev: number;
    samples: number[];
  };
}

interface BenchData {
  runtime?: string;
  results: { [key: string]: BenchResult };
}

// ---------------------------------------------------------------------------
// Load all inputs
// ---------------------------------------------------------------------------

const datasets: BenchData[] = inputs.map(
  (f) => JSON.parse(readFileSync(f, 'utf-8')) as BenchData,
);

const runtimeNames = datasets.map((d, i) => d.runtime || `input-${i + 1}`);
const multiInput = datasets.length > 1;

// ---------------------------------------------------------------------------
// Collect all benchmark names and group them
// ---------------------------------------------------------------------------

function groupOf(name: string): string {
  if (name.includes('/')) return name.split('/')[0];
  if (name.startsWith('v8-')) return 'v8';
  return 'test-suites';
}

// Ordered groups and ordered benchmark names within each group.
const groupOrder: string[] = [];
const groupBenchmarks = new Map<string, string[]>();

for (const data of datasets) {
  for (const name of Object.keys(data.results)) {
    const g = groupOf(name);
    if (!groupBenchmarks.has(g)) {
      groupBenchmarks.set(g, []);
      groupOrder.push(g);
    }
    const list = groupBenchmarks.get(g)!;
    if (!list.includes(name)) list.push(name);
  }
}

// Build a quick lookup: datasetIndex -> benchmarkName -> mean
const meanLookup: Map<string, number>[] = datasets.map((d) => {
  const m = new Map<string, number>();
  for (const [name, result] of Object.entries(d.results)) {
    m.set(name, result.totalTime.mean);
  }
  return m;
});

// ---------------------------------------------------------------------------
// Generate markdown
// ---------------------------------------------------------------------------

const lines: string[] = [];

lines.push(`# Benchmark Results`);
lines.push('');

const totalCount = new Set(datasets.flatMap((d) => Object.keys(d.results))).size;
lines.push(`**Total benchmarks:** ${totalCount}`);
lines.push('');

for (const group of groupOrder) {
  const benchNames = groupBenchmarks.get(group)!;
  lines.push(`## ${group}`);
  lines.push('');

  // Header
  const header = ['Benchmark (ms)', ...runtimeNames];
  lines.push('| ' + header.join(' | ') + ' |');
  lines.push('|' + header.map((_, i) => i === 0 ? '---|' : '---:|').join(''));

  // Rows
  for (const name of benchNames) {
    const vals = meanLookup.map((lookup) => lookup.get(name));
    const min = multiInput
      ? Math.min(...vals.filter((v): v is number => v !== undefined))
      : undefined;
    const cells = [name];
    for (const v of vals) {
      if (v === undefined) {
        cells.push('-');
      } else if (multiInput && v === min) {
        cells.push(`**${v}**`);
      } else {
        cells.push(String(v));
      }
    }
    lines.push('| ' + cells.join(' | ') + ' |');
  }
  lines.push('');
}

const md = lines.join('\n');
writeFileSync(output, md);
console.log(`Report written to: ${output}`);
