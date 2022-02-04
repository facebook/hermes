#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

[ ! -d "$HERMES_BUILD" ] && echo "HERMES_BUILD must be valid" && exit 1

rustgen=$HERMES_BUILD/bin/rustgen

$rustgen ffi > "$THIS_DIR/../../juno/crates/hermes/src/parser/generated_ffi.rs"
$rustgen cvt > "$THIS_DIR/../../juno/crates/juno/src/hparser/generated_cvt.rs"

