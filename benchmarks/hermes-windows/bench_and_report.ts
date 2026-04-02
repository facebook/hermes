import { spawnSync } from 'node:child_process';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const benchScript = join(__dirname, 'bench.ts');
const reportScript = join(__dirname, 'report.ts');
const resultJson = join(__dirname, 'bench_result.json');
const reportMd = join(__dirname, 'bench_report.md');

// Step 1: Run benchmarks
console.log('=== Running benchmarks ===');
const benchResult = spawnSync(
  process.execPath,
  ['--experimental-strip-types', benchScript, '--dynamic', '-c', '5', '-l', 'CI', '-o', resultJson],
  { stdio: 'inherit' },
);

if (benchResult.status !== 0) {
  console.error(`bench.ts failed with exit code ${benchResult.status}`);
  process.exit(1);
}

// Step 2: Generate report
console.log('');
console.log('=== Generating report ===');
const reportResult = spawnSync(
  process.execPath,
  ['--experimental-strip-types', reportScript, '-i', resultJson, '-o', reportMd],
  { stdio: 'inherit' },
);

if (reportResult.status !== 0) {
  console.error(`report.ts failed with exit code ${reportResult.status}`);
  process.exit(1);
}

console.log('');
console.log('Done.');
