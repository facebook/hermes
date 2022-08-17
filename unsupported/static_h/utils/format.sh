#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

IFS=$'\n'
set -f

all=0
force=0
if [ "$1" == "-a" ]; then
  shift
  all=1
elif [ "$1" == "-f" ]; then
  shift
  force=1
fi

[ -n "$1" ] && {
  echo "$0: Too many arguments" >&2
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
clang_format_version="$("$clang_format" --version)"
if echo "$clang_format_version" | grep -q -v '12.0.*'
then
   printf "ERROR: clang-format's version must match 12.0.*\n  clang-format path: %s\n  clang-format --version: %s" "$clang_format" "$clang_format_version"
   exit 1
fi

THIS_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$THIS_DIR"/../ || exit 1

have_changes=0       # We have uncommitted changes
authored_previous=0  # We authored the most recent commit on this branch

if [[ $(hg status -mard 2>/dev/null) ]]; then
  have_changes=1
fi

if hg log -l 1 --template="{author}" | grep -q -e "<$USER@" -e "^$USER$"
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
  files=( $(hg st . -man --include "**.{h,cpp,inc,def,mm,m}") )

elif (( authored_previous )) || (( force )); then
  last_commit_files=( $(hg st . -man --change . | grep -E '\.(h|cpp|inc|def|mm|m)$') )
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
  before=$(date -r "$f")
  "$clang_format" --verbose -i -style=file "$f" 2>&1 | tr -d "\n"
  echo -n '...'
  after=$(date -r "$f")
  test "$before" = "$after" && echo "ok" || echo "reformatted"
done
echo
echo "Done"
