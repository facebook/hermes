---
name: non-interactive-git-rebase
description: Use when needing to reorder, split, drop, or amend git commits that are not the top commit, without interactive editor access. Covers programmatic rebase via GIT_SEQUENCE_EDITOR, commit splitting with automated hunk selection, and metadata changes (author, message, dates) on any commit in a range.
---

# Non-Interactive Git Rebase

## Overview

`git rebase -i` normally opens an editor for the todo list. By setting `GIT_SEQUENCE_EDITOR` to a command that replaces the todo file with a pre-prepared one, you can perform any interactive rebase operation — reorder, drop, squash, edit, amend — entirely from scripts.

## CRITICAL: Always Create a Backup Branch

**Before any rebase, create a backup branch:**

```bash
git branch backup-before-rebase HEAD
```

**After the rebase succeeds and verification passes, do NOT delete the backup branch automatically.** Always ask the user before deleting it, unless the user explicitly requested automatic cleanup. The backup is cheap to keep and the user may discover problems later.

## Core Technique

### How GIT_SEQUENCE_EDITOR Works

The editor receives the todo file path as `$1`. Any command that overwrites that file works:

```bash
# Replace the todo with our prepared version
GIT_SEQUENCE_EDITOR="cp /tmp/prepared-todo" git rebase -i <base>
```

Git generates the default todo (all `pick` lines), then invokes `cp /tmp/prepared-todo <todo-file>`, which replaces it with your version. Git then executes the rewritten todo.

### Preparing the Todo File

Extract the default todo without actually rebasing — copy it out, then fail the editor so git aborts:

```bash
GIT_SEQUENCE_EDITOR='cp $1 /tmp/default-todo && false' git rebase -i <base>
```

The `false` exits non-zero, so git aborts the rebase. You have the todo file, no rebase happened.

Or build it directly from `git log`:

```bash
git rev-list --reverse <base>..HEAD | while read hash; do
  echo "pick $hash $(git log --oneline -1 $hash | cut -d' ' -f2-)"
done > /tmp/prepared-todo
```

Then edit the file: reorder lines, change `pick` to `drop`/`squash`/`edit`/etc.

### Using --exec for Bulk Amendments

The `--exec <cmd>` flag inserts an `exec <cmd>` line after every `pick` in the todo. Combined with `GIT_SEQUENCE_EDITOR=:` (a no-op that succeeds, leaving the default todo unchanged), this runs a command after each commit is replayed — no editor needed.

**Change author email on all commits in a range:**

```bash
GIT_SEQUENCE_EDITOR=: git rebase -i <base> --exec \
  'if [ "$(git log -1 --format=%ae)" != "correct@email.com" ]; then git commit --amend --no-edit --author="Name <correct@email.com>"; fi'
```

**Change commit message pattern:**

```bash
GIT_SEQUENCE_EDITOR=: git rebase -i <base> --exec \
  'git commit --amend -m "$(git log -1 --format=%B | sed s/old/new/)"'
```

## Operations

### Reorder Commits

Rearrange lines in the todo file. Example — move commit `abc123` to the front:

```bash
# Build todo with abc123 first, then everything else in original order
{
  echo "pick abc123 The commit to move first"
  git rev-list --reverse <base>..HEAD | while read hash; do
    full=$(git rev-parse abc123)
    [ "$hash" != "$full" ] && echo "pick $hash $(git log --oneline -1 $hash | cut -d' ' -f2-)"
  done
} > /tmp/prepared-todo

GIT_SEQUENCE_EDITOR="cp /tmp/prepared-todo" git rebase -i <base>
```

### Drop Commits

Remove lines from the todo, or change `pick` to `drop`.

### Reword Commit Messages

**NEVER use `reword` in the todo.** The `reword` command opens an interactive editor for the new message, which cannot be scripted via `GIT_SEQUENCE_EDITOR`. Instead, use `edit` and amend with `-m`:

```bash
# Step 1: Prepare todo with 'edit' on commits to reword

# Step 2: At each stop, amend the message non-interactively:
git commit --amend -m "New subject line

New body text."

# Step 3: Continue to the next stop
git rebase --continue
```

### Squash / Fixup

Change `pick` to `squash` or `fixup`. For squash, you also need `GIT_SEQUENCE_EDITOR` for the commit message editor that opens after squashing:

```bash
# Squash second commit into first, keep first message
# In the todo: pick aaa, fixup bbb
GIT_SEQUENCE_EDITOR="cp /tmp/prepared-todo" git rebase -i <base>
```

### Split a Commit

This requires `edit` in the todo. When rebase stops, you run commands to split, then continue.

```bash
# Step 1: Prepare todo with 'edit' on the target commit
sed 's/^pick TARGET_HASH/edit TARGET_HASH/' /tmp/default-todo > /tmp/prepared-todo
GIT_SEQUENCE_EDITOR="cp /tmp/prepared-todo" git rebase -i <base>

# Step 2: Rebase stops at the target commit. Split it:
git reset HEAD~1

# Step 3: Selectively stage and commit pieces.
# If the commit has N hunks in a file and you want hunk 3:
printf 'n\nn\ny\n' | git add -p path/to/file
git commit -m "First piece: just hunk 3"

# Stage the rest
git add .
git commit -m "Second piece: remaining changes"

# Step 4: Continue the rebase
git rebase --continue
```

**Automating hunk selection with `git add -p`:** Each `y`/`n` in the printf corresponds to a hunk in order. This works reliably when hunks are well-separated (different regions of the file). If hunks are adjacent, git may present them as one hunk, and you may need `s` (split) first:

```bash
# Split first hunk, then accept/reject sub-hunks
printf 's\ny\nn\ny\n' | git add -p path/to/file
```

**Alternative — patch-based splitting** (more reliable for complex cases):

```bash
git reset HEAD~1
git diff path/to/file > /tmp/full.patch
git checkout -- path/to/file

# Apply only the lines you want for commit 1
# (manually prepare /tmp/part1.patch from full.patch)
git apply /tmp/part1.patch
git add path/to/file
git commit -m "First piece"

git apply /tmp/full.patch  # won't re-apply already-applied hunks
# Or: git checkout <original-hash> -- path/to/file
git add .
git commit -m "Second piece"
```

## Verification

After any rebase, verify the tree content is identical to before (unless you intentionally dropped commits):

```bash
git diff backup-before-rebase HEAD --stat
# Empty output = content preserved correctly
```

## Common Mistakes

| Mistake | Fix |
|---------|-----|
| Using `reword` in the todo | `reword` opens an interactive editor. Use `edit` + `git commit --amend -m` instead |
| Todo file has wrong hashes after prior rebase | Regenerate from `git log` — hashes change after every rebase |
| `git add -p` gets unexpected hunks | Hunks may merge if context overlaps. Use patch-based splitting instead |
| Deleting backup branch too early | Always ask the user before deleting. They may discover problems later |

## Quick Reference

| Operation | Command |
|-----------|---------|
| Extract todo | `GIT_SEQUENCE_EDITOR='cp $1 /tmp/default-todo && false' git rebase -i <base>` |
| Reorder/drop | `GIT_SEQUENCE_EDITOR="cp /tmp/prepared-todo" git rebase -i <base>` |
| Reword message | Same as above, use `edit` in todo, then `git commit --amend -m "..."` + `--continue` |
| Squash (keep msg) | Same as above, use `fixup` in todo |
| Squash (edit msg) | Same as above, use `squash`, then handle message editor |
| Split | Same as above, use `edit`, then manual split + `--continue` |
| Bulk amend | `GIT_SEQUENCE_EDITOR=: git rebase -i <base> --exec '<cmd>'` |
