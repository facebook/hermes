// Compute a SHA-256 hash of package-lock.json and write it to node_modules.sha256.
// This is a cross-platform replacement for:
//   tar -cf - node_modules | sha256sum | awk '{print $1}' > node_modules.sha256

import { createHash } from "node:crypto";
import { readFileSync, writeFileSync } from "node:fs";

const dir = new URL(".", import.meta.url);
const lockfile = new URL("package-lock.json", dir);
const hash = createHash("sha256").update(readFileSync(lockfile)).digest("hex");
writeFileSync(new URL("node_modules.sha256", dir), hash + "\n");
