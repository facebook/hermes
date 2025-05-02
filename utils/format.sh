#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

IFS=$'\n'
set -f

all=0
force=0
interactive=0
if [ "$1" == "-a" ]; then
  shift
  all=1
elif [ "$1" == "-f" ]; then
  shift
  force=1
elif [ "$1" == "-i" ]; then
  shift
  interactive=1
fi

[ -n "$1" ] && {
  echo "Usage: $0 [-a|-f|-i]" >&2
  echo "  -a: Format all files" >&2
  echo "  -f: Force format files in last commit without prompting" >&2
  echo "  -i: Interactive mode, show diff and ask before applying changes" >&2
  exit 1
}

# Look for clang-format at a fixed path relative to the script.
script_path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
clang_format_path="${script_path}/../../../tools/third-party/clang-format"
if [ "$(uname)" == Linux ]; then
  clang_format="${clang_format_path}/linux/x86_64/clang-format"
else
  clang_format="${clang_format_path}/macos/clang-format"
fi

# Fall back to one in $PATH if not found.
[ -x "${clang_format}" ] || clang_format="$(which clang-format)"

if [ ! -x "$clang_format" ]; then
  echo "ERROR: Must have clang-format in PATH"
  exit 1
fi

# Determine which diff command to use
if [ -t 1 ] && diff --color=always /dev/null /dev/null >/dev/null 2>&1; then
  # Terminal supports color and GNU diff with color is available
  diff_cmd=(diff --color=always -u)
elif which colordiff >/dev/null 2>&1; then
  # Use colordiff as alternative
  diff_cmd=(colordiff -u)
else
  # Fallback to plain diff
  diff_cmd=(diff -u)
fi

THIS_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$THIS_DIR"/../ || exit 1

have_changes=0       # We have uncommitted changes
authored_previous=0  # We authored the most recent commit on this branch

if [[ $(git status --porcelain 2>/dev/null) ]]; then
  have_changes=1
fi

if git log -1 --pretty=format:"%an <%ae>" | grep -q -e "<$USER@" -e "^$USER" -e "<$USER>" -e "$USER@"
then
  authored_previous=1
fi

files=()

if (( all )); then
  echo "Formatting all files..."
  files=(
    $(find lib include tools unittests \
        -name \*.h -print -o -name \*.cpp -print)
  )
elif (( have_changes )); then
  echo "Formatting only modified files..."
  files=( $(git diff --name-only --diff-filter=ACMR | grep -E '\.(h|cpp|inc|def|mm|m)$') )

elif (( authored_previous )) || (( force )); then
  last_commit_files=( $(git diff-tree --no-commit-id --name-only -r HEAD | grep -E '\.(h|cpp|inc|def|mm|m)$') )
  echo "There are no modified files, but you authored the last commit:"
  printf '  %s\n' "${last_commit_files[@]}"
  if (( !force )); then
    read -r -p "Format files in that commit? [y/N]" reply
    if [[ $reply != [Yy] ]]; then
      echo "Not doing anything."
      exit 0
    fi
  fi
  files=( "${last_commit_files[@]}" )
else
  echo "You have no modified files and you didn't recently commit on this branch."
  echo "To format all files: $0 -a"
  exit 1
fi

for f in "${files[@]}"; do
  if [ ! -f "$f" ]; then
    echo "File not found: $f (skipping)"
    continue
  fi

  # Create a temporary file for the formatted content
  temp_file=$(mktemp)
  "$clang_format" -style=file "$f" > "$temp_file"

  # Check if the file would be modified
  if ! diff -q "$f" "$temp_file" >/dev/null 2>&1; then
    echo "File would be reformatted: $f"

    # Always show the differences with color
    "${diff_cmd[@]}" "$f" "$temp_file" | sed "s|$temp_file|$f (formatted)|"

    if (( interactive )); then
      # Ask user whether to apply changes
      read -r -p "Apply these changes? [y/N] " reply
      if [[ $reply == [Yy] ]]; then
        cp "$temp_file" "$f"
        echo "Changes applied to $f"
      else
        echo "Changes not applied to $f"
      fi
    else
      # Apply changes without asking
      cp "$temp_file" "$f"
      echo "Reformatted: $f"
    fi
  else
    echo "No changes needed for $f"
  fi

  # Clean up the temporary file
  rm -f "$temp_file"
done

echo "Done"
