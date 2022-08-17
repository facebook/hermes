#! /bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Exit on any failure
set -e
set -o pipefail

# Check that the supplied binaries exist
[ -x "$HERMES" ] || exit 3
[ -x "$HBCDUMP" ] || exit 4
[ -x "$HBC_READ_TRACE" ] || exit 5

# Create a workspace and make sure it is cleaned up when we exit.
TMP=$(mktemp -d)
trap 'rm -rf $TMP' exit

# The JS file we will compile.
cat <<EOF > "$TMP/test.js"
print("Hello, world!")
EOF

"$HERMES" -O -emit-binary -target=HBC -out="$TMP/test.hbc" "$TMP/test.js"

# The test trace of page accesses (states that we saw one access, of the 0'th
# page and it took 42 microseconds).
# The output test.log is a tsv file, and therefore there are tabs in the string
# below, thus
# @lint-ignore-every TXT2
cat <<EOF > "$TMP/test.log"
Page	Time (us)
0	42
EOF

# The sections we expect hbc-read-trace to output
cat <<EOF > "$TMP/expected"
<unknown>
function-table
string-kinds
identifier-hashes
string-table
overflow-string-table
string-storage
array-buffer
object-key-buffer
object-value-buffer
bigint-storage
regular-expression-table
regular-expression-storage
commonjs-module-table
function-body
function-info
debug-info
EOF

# Run hbc-read-trace and compare it with the above list.
"$HBC_READ_TRACE" -v -D "$HBCDUMP" -B "$TMP/test.hbc" < "$TMP/test.log" \
  | head -n 1 \
  | perl -pe 's/\//\n/g' \
  | diff -u - "$TMP/expected"
