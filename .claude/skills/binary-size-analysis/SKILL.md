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

Before starting, inform the user how size is measured and ask about verification:

1. **Explain the measurement method**: Tell the user that sizes are measured by
   summing ALLOC section sizes (via `readelf -SW` on Linux or `otool -l` on
   macOS), not by measuring the file size with `stat`. This excludes alignment
   padding and non-shipping sections (symbol tables, string tables, code
   signatures), matching what actually ships on-device after `strip`.

2. **Ask about parent-commit verification**: This verification is enabled by
   default. It builds each BUILD commit's parent to confirm no skipped commit
   changed the binary size. It catches miscategorized SKIP commits but roughly
   doubles the build time. If the user declines, skip all verification substeps
   in Step 2.

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

### Step 0: Verify Git Repository and Tools

This skill requires a GitHub checkout of the static_h repo (commit hashes are git
hashes). Before proceeding, verify the working directory is a valid git repo:

```bash
git rev-parse --is-inside-work-tree
```

If this fails (e.g., the repo is a Sapling/Mercurial checkout or not a repo at
all), stop and inform the user that this skill only works in a git checkout of
the Hermes repository.

**Bloaty setup**: This skill uses [bloaty](https://github.com/google/bloaty) for
detailed per-section diffs on large size increases. Download and build it:

```bash
git clone https://github.com/google/bloaty.git ~/bloaty
cd ~/bloaty && cmake -B build -GNinja && cmake --build build
```

If the download or build fails (e.g., no network access, missing dependencies),
ask the user to provide the path to a pre-built `bloaty` binary.

### Step 1: Categorize Commits

Generate the list of commits in the range and categorize each as BUILD or SKIP.

A commit is SKIP if it **only** touches files outside the directories that
contribute to the hermesvm binary. The relevant source directories are:

- `lib/` (VM, Parser, Sema, IR, BCGen, Support, Regex, etc.)
- `API/hermes/`
- `public/hermes/`
- `include/hermes/`
- `external/`
- `CMakeLists.txt` (exact match, not filtered by `.txt` suffix), `cmake/`,
  `lib/config/`

Files that never affect the binary: `test/`, `unittests/`, `.github/`, docs
(`.md`, `.txt`, `.rst`), `tools/`, npm packages (`hermes-parser`,
`prettier-plugin`, etc.), `utils/`, `benchmarks/`, `website/`, `blog/`.

Use this Python script to categorize:

```python
#!/usr/bin/env python3
import subprocess, sys

RELEVANT_PREFIXES = [
    "lib/", "API/hermes/", "API/jsi/", "public/hermes/", "include/hermes/",
    "external/", "cmake/", "lib/config/",
]
# Files that match exactly (not as prefixes) and should always be BUILD.
RELEVANT_EXACT = ["CMakeLists.txt"]
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
        if f in RELEVANT_EXACT:
            has_relevant = True
            break
        if any(f.endswith(s) for s in ALWAYS_SKIP_SUFFIXES):
            continue
        if any(f.startswith(p) for p in RELEVANT_PREFIXES):
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

**Size measurement**: Use the sum of ALLOC section sizes, not `stat` on the
library file. This is important for two reasons:

1. **Excludes alignment padding**: The linker aligns loadable segments to page
   boundaries (64KB on Linux, 16KB on macOS). A 1-byte code change can shift a
   segment boundary and cause up to one page of padding change in the file size,
   creating false deltas.
2. **Matches production**: ALLOC sections are exactly the sections that remain
   after `strip` and ship on-device. Non-ALLOC sections (`.symtab`, `.strtab`,
   `.shstrtab`, `.comment`, `.gnu.build.attributes`) are stripped in production.

Use the following Python helper to measure size on both platforms:

```python
import platform, re, subprocess

def get_section_size(library_path):
    """Sum section sizes that ship in production, excluding symbol/string
    tables and inter-segment alignment padding.

    - Linux (ELF): sums all sections with the ALLOC flag via readelf.
      This excludes .symtab, .strtab, .shstrtab, .comment, and
      .gnu.build.attributes which are stripped in production.
    - macOS (Mach-O): sums all individual section sizes via otool.
      Sections are listed under __TEXT, __DATA, __DATA_CONST, etc.
      The __LINKEDIT segment (which contains symbol tables, string
      tables, code signatures) has no sub-sections so it is naturally
      excluded.
    """
    if platform.system() == "Darwin":
        return _get_section_size_macho(library_path)
    else:
        return _get_section_size_elf(library_path)

def _get_section_size_elf(library_path):
    output = subprocess.check_output(
        ["readelf", "-SW", library_path], text=True
    )
    total = 0
    for line in output.split('\n'):
        # Match section lines: [Nr] Name Type Addr Off Size ES Flg ...
        m = re.match(
            r'\s*\[\s*\d+\]\s+\S+\s+\S+\s+[0-9a-f]+\s+[0-9a-f]+\s+'
            r'([0-9a-f]+)\s+[0-9a-f]+\s+(.*)', line
        )
        if m and 'A' in m.group(2):
            total += int(m.group(1), 16)
    return total

def _get_section_size_macho(library_path):
    output = subprocess.check_output(
        ["otool", "-l", library_path], text=True
    )
    total = 0
    in_section = False
    for line in output.split('\n'):
        # Each section entry in otool -l contains both "sectname" and
        # "segname" lines, followed by "size 0x...".  Only reset on
        # "cmd " (a new load command), NOT on "segname" — otherwise
        # the segname line inside each section entry clears the flag
        # before the size line is reached.
        if 'sectname' in line:
            in_section = True
        elif 'cmd ' in line:
            in_section = False
        elif in_section:
            m = re.match(r'\s+size\s+(0x[0-9a-f]+)', line)
            if m:
                total += int(m.group(1), 16)
    return total
```

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
      - Record the parent's ALLOC size
      - Compare to the previous BUILD commit's ALLOC size. If they differ, a
        SKIP commit between the previous BUILD and this one changed the binary.
        Log a **VERIFICATION FAILURE** with the two sizes and the commit range
        that needs re-examination. Stop the analysis and report the failure so
        Step 1's categorization can be fixed.
      - `git checkout <hash> --force --quiet` (return to the BUILD commit)
   c. `cmake -B <build_dir> <CMAKE_ARGS>`
   d. `cd <build_dir> && ninja hermesvm`
   e. Record ALLOC section size sum using `get_section_size()` on
      `<build_dir>/lib/libhermesvm.so` (Linux) or `libhermesvm.dylib` (macOS)
   f. Compute delta from previous BUILD commit's size
   g. **Write the row to the markdown file immediately** before proceeding
   h. **Bloaty diff** (if delta >= 3000 bytes): Run `bloaty` to get a per-section
      breakdown of the size increase. This helps identify whether the growth is
      in code (`.text`), data (`.rodata`), or elsewhere.
      - The previous BUILD commit's binary must be saved before building the
        current commit. Copy it at the start of each iteration (before step a):
        `cp <build_dir>/lib/libhermesvm.so /tmp/libhermesvm_prev.so`
        Note: when verification is enabled, the parent binary from step (b)
        can be used instead, but using the previous BUILD binary is simpler
        and works in both modes.
      - Run: `bloaty <current_binary> -- /tmp/libhermesvm_prev.so`
      - Filter the output to keep only ALLOC sections (section names are the
        last token on each line): `.text`, `.rodata`, `.data`, `.data.rel.ro`,
        `.bss`, `.tbss`, `.gcc_except_table`, `.eh_frame`, `.eh_frame_hdr`,
        `.init_array`, `.fini_array`, `.plt`, `.got`, `.got.plt`, `.rela.dyn`,
        `.rela.plt`, `.dynsym`, `.dynstr`, `.gnu.hash`, `.gnu.version`,
        `.init`, `.fini`, and the `TOTAL` row.
      - Append the filtered bloaty output to the end of the markdown file under
        a `## Bloaty Diff` section, with a heading for each commit.
   i. If build fails, record BUILD_FAIL and continue

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
4. Bloaty diff section with per-section breakdowns for commits with delta >= 3KB

## Notes

- The build directory is `cmake-build-minsizerel` inside the repo.
- Each cmake reconfiguration + incremental build takes ~20-30 seconds per
  commit. For 500+ BUILD commits, expect ~3-5 hours total.
- Run the build loop as a background task and monitor with periodic tail checks.
- Build failures near HEAD are common when commits depend on later fixes;
  record them and continue.
