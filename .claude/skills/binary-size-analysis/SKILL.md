---
name: binary-size-analysis
description: >
  This skill should be used when the user wants to analyze hermesvm binary size
  changes across a range of commits. Use when the user mentions "binary size",
  "size analysis", "size regression", "size increase", or asks to measure how
  commits affect the hermesvm library size.
tags:
  - hermes
  - binary-size
  - analysis
---

# hermesvm Binary Size Analysis

Analyze per-commit binary size changes of the `hermesvm` shared library across a
git commit range. Produces a markdown report with per-commit sizes and summary
tables of significant increases and decreases.

## When This Skill Applies

- Analyzing which commits caused hermesvm binary size growth
- Investigating size regressions in the hermesvm library
- Producing a size report for a range of commits

## Pre-Run Prompt

Before starting, ask the user whether to enable **parent-commit verification**
(enabled by default). This verification builds each BUILD commit's parent to
confirm no skipped commit changed the binary size. It catches miscategorized SKIP
commits but roughly doubles the build time. If the user declines, skip all
verification substeps in Step 2.

## Required User Input

- **Commit range**: A base commit and an end commit (or HEAD). For example:
  `b48b6cc..HEAD` or `v0.290.0..v0.300.0`
- **CMake flags** (optional): Additional CMake configuration flags. Defaults are
  listed below.
- **Parent-commit verification** (optional): Enabled by default. Disable to
  halve build time at the risk of missing miscategorized SKIP commits.

## Default Build Configuration

```
cmake -B cmake-build-minsizerel -GNinja \
  -DHERMESVM_GCKIND=HADES \
  -DHERMESVM_HEAP_HV_MODE=HEAP_HV_PREFER32 \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DHERMESVM_ALLOW_JIT=2 \
  -DHERMESVM_ALLOW_CONTIGUOUS_HEAP=ON \
  -DHERMES_IS_MOBILE_BUILD=ON
```

If the user provides additional flags, append them to the cmake command. If the
user provides a different value to flags in default build configuration above,
replace it with the user-provided value (e.g., -DHERMESVM_ALLOW_JIT=0).

## Procedure

### Step 0: Verify Git Repository

This skill requires a GitHub checkout of the static_h repo (commit hashes are git
hashes). Before proceeding, verify the working directory is a valid git repo:

```bash
git rev-parse --is-inside-work-tree
```

If this fails (e.g., the repo is a Sapling/Mercurial checkout or not a repo at
all), stop and inform the user that this skill only works in a git checkout of
the Hermes repository.

### Step 1: Categorize Commits

Generate the list of commits in the range and categorize each as BUILD or SKIP.

A commit is SKIP if it **only** touches files outside the directories that
contribute to the hermesvm binary. The relevant source directories are:

- `lib/` (VM, Parser, Sema, IR, BCGen, Support, Regex, etc.)
- `API/hermes/`
- `public/hermes/`
- `include/hermes/`
- `external/llvh/`, `external/dtoa/`, `external/zip/`, `external/boost/`,
  `external/fast_float/`, `external/dragonbox/`
- `CMakeLists.txt`, `cmake/`, `lib/config/`

Files that never affect the binary: `test/`, `unittests/`, `.github/`, docs
(`.md`, `.txt`, `.rst`), `tools/`, npm packages (`hermes-parser`,
`prettier-plugin`, etc.), `utils/`, `benchmarks/`, `website/`, `blog/`.

Use this Python script to categorize:

```python
#!/usr/bin/env python3
import subprocess, sys

RELEVANT_PREFIXES = [
    "lib/", "API/hermes/", "public/hermes/", "include/hermes/",
    "external/llvh/", "external/dtoa/", "external/zip/",
    "external/boost/", "external/fast_float/", "external/dragonbox/",
    "CMakeLists.txt", "cmake/", "lib/config/",
]
ALWAYS_SKIP_SUFFIXES = [".md", ".txt", ".rst"]

base_commit = sys.argv[1]
end_commit = sys.argv[2] if len(sys.argv) > 2 else "HEAD"

result = subprocess.run(
    ["git", "log", "--reverse", "--format=%H %s", f"{base_commit}..{end_commit}"],
    capture_output=True, text=True
)

for line in result.stdout.strip().split("\n"):
    if not line:
        continue
    commit, subject = line.split(" ", 1)
    files_result = subprocess.run(
        ["git", "diff-tree", "--no-commit-id", "--name-only", "-r", commit],
        capture_output=True, text=True
    )
    files = files_result.stdout.strip().split("\n") if files_result.stdout.strip() else []
    has_relevant = False
    for f in files:
        if any(f.endswith(s) for s in ALWAYS_SKIP_SUFFIXES):
            continue
        if any(f.startswith(p) or f == p for p in RELEVANT_PREFIXES):
            has_relevant = True
            break
    category = "BUILD" if has_relevant else "SKIP"
    print(f"{category}\t{commit[:11]}\t0\t0\t{subject}")
```

Save the BUILD-only lines to a file for the build loop.

### Step 2: Build and Measure

**Critical**: Always run `cmake -B ...` before each `ninja hermesvm` to ensure
new/removed source files are detected. Without reconfiguration, incremental
builds miss new files (especially JS polyfills in `lib/InternalJavaScript/`),
causing deltas to be misattributed to later commits.

Initialize the markdown results file immediately, then iterate through BUILD
commits only:

1. Checkout the base commit, configure, build, record baseline size.
2. For each BUILD commit in chronological order:
   a. `git checkout <hash> --force --quiet`
   b. **Parent-commit verification** (if enabled): Before building this commit,
      build its parent (`<hash>~1`) to verify the categorization of intervening
      SKIP commits:
      - `git checkout <hash>~1 --force --quiet`
      - `cmake -B <build_dir> <CMAKE_ARGS>`
      - `cd <build_dir> && ninja hermesvm`
      - Record the parent's size
      - Compare to the previous BUILD commit's size. If they differ, a SKIP
        commit between the previous BUILD and this one changed the binary.
        Log a **VERIFICATION FAILURE** with the two sizes and the commit range
        that needs re-examination. Stop the analysis and report the failure so
        Step 1's categorization can be fixed.
      - `git checkout <hash> --force --quiet` (return to the BUILD commit)
   c. `cmake -B <build_dir> <CMAKE_ARGS>`
   d. `cd <build_dir> && ninja hermesvm`
   e. Record `stat -f "%z"` (macOS) or `stat -c "%s"` (Linux) of
      `<build_dir>/lib/libhermesvm.dylib` (or `.so` on Linux)
   f. Compute delta from previous BUILD commit's size
   g. **Write the row to the markdown file immediately** before proceeding
   h. If build fails, record BUILD_FAIL and continue

Do **not** include SKIP commits in the output table.

### Step 3: Generate Summary

After all commits are processed, append to the markdown file:

- Overall summary (baseline, final, net change, percentage)
- **Significant Size Increases** table (delta >= +1000), sorted by delta
  descending
- **Significant Size Decreases** table (delta <= -1000), sorted by delta
  ascending (largest decrease first)

**Important**: When parsing the markdown table to extract deltas, handle pipe
characters (`|`) in commit subjects carefully. Use regex anchored to the table
structure rather than naive pipe splitting.

### Step 4: Restore Branch

After analysis, restore the original branch:
```bash
git checkout <original_branch> --force --quiet
```

## Output Format

The markdown file (`hermesvm_size_analysis.md` in the repo root) contains:

1. Build configuration header
2. Per-commit table: `| # | Commit | Subject | Size (bytes) | Delta | Total Delta |`
3. Summary section with significant changes tables

## Notes

- The build directory is `cmake-build-minsizerel` inside the repo.
- Each cmake reconfiguration + incremental build takes ~20-30 seconds per
  commit. For 500+ BUILD commits, expect ~3-5 hours total.
- Run the build loop as a background task and monitor with periodic tail checks.
- Build failures near HEAD are common when commits depend on later fixes;
  record them and continue.
