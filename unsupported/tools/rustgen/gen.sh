#!/bin/bash

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

[ ! -d "$HERMES_BUILD" ] && echo "HERMES_BUILD must be valid" && exit 1

rustgen=$HERMES_BUILD/bin/rustgen

$rustgen ffi > "$THIS_DIR/../../juno/hermes/src/parser/generated_ffi.rs"
$rustgen cvt > "$THIS_DIR/../../juno/src/hparser/generated_cvt.rs"

