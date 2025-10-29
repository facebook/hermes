/**
 * BLAKE2b-based deterministic hash of the entire node_modules tree.
 *
 * NOT currently wired into build. To use instead of the shell pipeline:
 * 1. Add a script entry to package.json, e.g.:
 *    "postinstall": "node hashNodeModulesBLAKE2b.mjs > node_modules.sha256"
 *    (Change the output filename or extension as desired.)
 * 2. (Optional) Rename the expected OUTPUT in CMakeLists.txt from node_modules.sha256
 *    to match whatever filename you output here if you switch.
 *
 * Rationale:
 * - Avoids relying on external coreutils hashing by doing everything in Node.
 * - Uses blake2b256 (fast, secure, built-in to Node >=19 via crypto.createHash).
 * - Creates a stable manifest-style hash: sorts file paths, hashes each file,
 *   then hashes the sequence (path + NUL + fileHash + NUL) to produce one digest.
 * - Output is a single 256-bit hex string.
 *
 * Notes:
 * - For very large dependency trees this loads each file fully. Disk I/O dominates;
 *   streaming each file would not significantly change total time. If needed, you
 *   can replace readFile with a streaming hash per file.
 * - Symlinks are skipped; adjust as necessary.
 */
import { createHash } from 'node:crypto';
import { readdir, readFile, lstat } from 'node:fs/promises';
import { join, relative } from 'node:path';

const ROOT = 'node_modules';

async function *walk(dir) {
  const entries = await readdir(dir, { withFileTypes: true });
  for (const ent of entries) {
    const full = join(dir, ent.name);
    if (ent.isDirectory()) {
      // Skip nested node_modules inside packages? (Keep them for correctness.)
      yield *walk(full);
    } else if (ent.isFile()) {
      yield full;
    } else if (ent.isSymbolicLink()) {
      // Optionally resolve symlinks; currently ignoring to keep determinism simple.
      // const target = await realpath(full); // if needed
    }
  }
}

async function hashAll({ root = ROOT, perFileAlgo = 'blake2b256', finalAlgo = 'blake2b256' } = {}) {
  const files = [];
  for await (const f of walk(root)) files.push(f);
  files.sort(); // deterministic order

  const final = createHash(finalAlgo);
  for (const abs of files) {
    const rel = relative(root, abs);
    const st = await lstat(abs);
    // Include size & mtime ns to short-circuit content read for unchanged files? We choose content for robustness.
    const buf = await readFile(abs);
    const fileHash = createHash(perFileAlgo).update(buf).digest('hex');
    final.update(rel).update('\0').update(fileHash).update('\0');
  }
  return final.digest('hex');
}

if (process.argv[1] === new URL(import.meta.url).pathname) {
  hashAll().then(d => {
    process.stdout.write(d + '\n');
  }).catch(err => {
    console.error('[hashNodeModulesBLAKE2b] Failed:', err);
    process.exit(1);
  });
}

export { hashAll };
