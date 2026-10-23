#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Compare the JIT-emitted code of two hermes binaries over the same corpus.
#
# Takes binaries rather than driving a build, so it works the same whether the
# binaries came from CMake, buck2, or anywhere else.
#
# Reports whether any *instruction* changed, separately from comment lines,
# because that is usually the question being asked of a refactor.

set -u
set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  cat <<'EOF'
Usage: jit-diff.sh [options] <hermes-before> <hermes-after>

Captures a canonicalized JIT dump from each binary and compares them,
reporting comment-line changes separately from instruction changes.

Options:
  -k DIR         Keep the captures and the diff in DIR instead of a temp dir.
  -c FILE        Add FILE to the corpus, run untyped. Repeatable.
  -t FILE        Add FILE to the corpus, run with -typed. Repeatable.
  -n N           Show at most N lines of the diff (default 40, 0 for all).
  --comments-ok  Exit 0 when only comment lines differ.
  --dumps        The two arguments are existing jit-dump.sh outputs rather
                 than binaries. Useful when the "before" binary is gone but
                 its capture was kept.
  -h, --help     Show this help.

Exit status:
  0  dumps identical (or only comments differ, with --comments-ok)
  1  dumps differ
  2  usage or capture error

Example:
  git stash && cmake --build build --target hermes && cp build/bin/hermes /tmp/h-before
  git stash pop && cmake --build build --target hermes
  utils/jit/jit-diff.sh /tmp/h-before build/bin/hermes
EOF
}

KEEP=""
MAXLINES=40
COMMENTS_OK=0
DUMPS=0
CORPUS=()

# Abort rather than loop forever when an option is given without its operand.
need_arg() {
  if [ "$2" -lt 2 ]; then
    echo "jit-diff: option $1 requires an argument" >&2
    exit 2
  fi
}

while [ $# -gt 0 ]; do
  case "$1" in
    -k) need_arg -k $#; KEEP="$2"; shift 2 ;;
    -c) need_arg -c $#; CORPUS+=(-c "$2"); shift 2 ;;
    -t) need_arg -t $#; CORPUS+=(-t "$2"); shift 2 ;;
    -n) need_arg -n $#; MAXLINES="$2"; shift 2 ;;
    --comments-ok) COMMENTS_OK=1; shift ;;
    --dumps) DUMPS=1; shift ;;
    -h|--help) usage; exit 0 ;;
    -*) echo "jit-diff: unknown option: $1" >&2; usage >&2; exit 2 ;;
    *) break ;;
  esac
done

if [ $# -ne 2 ]; then
  echo "jit-diff: expected two ${DUMPS:+dump files}${DUMPS:-hermes binaries}" >&2
  usage >&2
  exit 2
fi
BEFORE="$1"
AFTER="$2"

if [ -n "$KEEP" ]; then
  mkdir -p "$KEEP" || exit 2
  D="$KEEP"
else
  D="$(mktemp -d)"
  trap 'rm -rf "$D"' EXIT
fi

if [ "$DUMPS" -eq 1 ]; then
  for f in "$BEFORE" "$AFTER"; do
    if [ ! -f "$f" ] || [ ! -r "$f" ]; then
      echo "jit-diff: cannot read dump file: $f" >&2
      exit 2
    fi
  done
  cp "$BEFORE" "$D/before.txt" || exit 2
  cp "$AFTER" "$D/after.txt" || exit 2
else
  echo "jit-diff: capturing 'before' from $BEFORE" >&2
  "$SCRIPT_DIR/jit-dump.sh" ${CORPUS[@]+"${CORPUS[@]}"} -o "$D/before.txt" "$BEFORE" || exit 2
  echo "jit-diff: capturing 'after'  from $AFTER" >&2
  "$SCRIPT_DIR/jit-dump.sh" ${CORPUS[@]+"${CORPUS[@]}"} -o "$D/after.txt" "$AFTER" || exit 2
fi

# diff exits 0 for same, 1 for differing, >1 for trouble. Treating trouble as
# "differing" would report a misleading result for an unreadable capture.
diff -u "$D/before.txt" "$D/after.txt" > "$D/jit.diff"
DSTATUS=$?
if [ "$DSTATUS" -eq 0 ]; then
  echo "jit-diff: dumps are identical"
  exit 0
elif [ "$DSTATUS" -gt 1 ]; then
  echo "jit-diff: diff failed with status $DSTATUS" >&2
  exit 2
fi

# Classify each changed line. A line that begins with '//' after the diff
# marker is a comment emitted by Emitter::comment(); anything else is a label
# or an instruction.
changed=$(grep -cE '^[-+]' "$D/jit.diff")
headers=$(grep -cE '^(\+\+\+|---)' "$D/jit.diff")
changed=$((changed - headers))
added=$(grep -cE '^\+' "$D/jit.diff")
removed=$(grep -cE '^-' "$D/jit.diff")
added=$((added - 1))
removed=$((removed - 1))
comments=$(grep -E '^[-+]' "$D/jit.diff" | grep -vE '^(\+\+\+|---)' \
           | sed -E 's/^[-+][[:space:]]*//' | grep -cE '^//' || true)
insns=$((changed - comments))

echo "jit-diff: $changed changed lines (+$added / -$removed)"
echo "  comment lines:     $comments"
echo "  instruction lines: $insns"
if [ "$insns" -eq 0 ]; then
  echo "RESULT: comments only, no instruction changed"
else
  echo "RESULT: INSTRUCTIONS CHANGED"
fi
[ -n "$KEEP" ] && echo "captures and diff kept in $D"

if [ "$MAXLINES" -ne 0 ]; then
  echo "--- first $MAXLINES diff lines ---"
  head -n "$MAXLINES" "$D/jit.diff"
else
  cat "$D/jit.diff"
fi

if [ "$insns" -eq 0 ] && [ "$COMMENTS_OK" -eq 1 ]; then
  exit 0
fi
exit 1
