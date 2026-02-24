#!/usr/bin/env node
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/**
 * Wrapper script for fork-sync that points to the hermes-windows root.
 *
 * Usage:
 *   node tools/fork-sync/sync.ts --dep icu-small
 *   node tools/fork-sync/sync.ts --dep icu-small --status
 *   node tools/fork-sync/sync.ts --dep icu-small --continue
 *
 * All arguments are forwarded to fork-sync with `-C ..` prepended
 * so it finds the sync-manifest.json in the repository root.
 */

import { execFileSync } from "node:child_process";
import * as path from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const syncScript = path.join(
  __dirname,
  "node_modules",
  "@rnx-kit",
  "fork-sync",
  "lib",
  "sync.js"
);

const args = [syncScript, "-C", path.join(__dirname, ".."), ...process.argv.slice(2)];

try {
  execFileSync("node", args, { stdio: "inherit" });
} catch (e: unknown) {
  // execFileSync throws on non-zero exit; stdio is already inherited
  const err = e as { status?: number };
  process.exit(err.status ?? 1);
}
