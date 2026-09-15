import assert from "node:assert";
import { spawn } from "node:child_process";
import fs from "node:fs";
import path from "node:path";

assert(
  typeof import.meta.dirname === "string",
  "Expecting a recent Node.js runtime API version"
);

const ROOT_PATH = path.resolve(import.meta.dirname, "..", "..");
const TESTS_ROOT_PATH = path.join(ROOT_PATH, "tests");
const ASSERT_MODULE_PATH = path.join(
  ROOT_PATH,
  "implementors",
  "node",
  "assert.js"
);
const LOAD_ADDON_MODULE_PATH = path.join(
  ROOT_PATH,
  "implementors",
  "node",
  "load-addon.js"
);

export function listDirectoryEntries(dir: string) {
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  const directories: string[] = [];
  const files: string[] = [];

  for (const entry of entries) {
    if (entry.isDirectory()) {
      directories.push(entry.name);
    } else if (entry.isFile() && entry.name.endsWith(".js")) {
      files.push(entry.name);
    }
  }

  directories.sort();
  files.sort();

  return { directories, files };
}

export function runFileInSubprocess(
  cwd: string,
  filePath: string
): Promise<void> {
  return new Promise((resolve, reject) => {
    const child = spawn(
      process.execPath,
      [
        // Using file scheme prefix when to enable imports on Windows
        "--import",
        "file://" + ASSERT_MODULE_PATH,
        "--import",
        "file://" + LOAD_ADDON_MODULE_PATH,
        filePath,
      ],
      { cwd }
    );

    let stderrOutput = "";
    child.stderr.setEncoding("utf8");
    child.stderr.on("data", (chunk) => {
      stderrOutput += chunk;
    });

    child.stdout.pipe(process.stdout);

    child.on("error", reject);

    child.on("close", (code, signal) => {
      if (code === 0) {
        resolve();
        return;
      }

      const reason =
        code !== null ? `exit code ${code}` : `signal ${signal ?? "unknown"}`;
      const trimmedStderr = stderrOutput.trim();
      const stderrSuffix = trimmedStderr
        ? `\n--- stderr ---\n${trimmedStderr}\n--- end stderr ---`
        : "";
      reject(
        new Error(
          `Test file ${path.relative(
            TESTS_ROOT_PATH,
            filePath
          )} failed (${reason})${stderrSuffix}`
        )
      );
    });
  });
}
