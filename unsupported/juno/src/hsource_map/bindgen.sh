#!/usr/bin/env bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

input=../../../../include/hermes/SourceMap/c-api.h
a='@'

{
    cat <<EOF
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

EOF
    echo "/* ${a}generated DO NOT EDIT */"
    bindgen \
        --no-layout-tests \
        --allowlist-type 'Hermes.*' \
        --allowlist-function 'hermes_.*' \
        --blocklist-type 'size_t' \
        --raw-line "use libc::size_t;" \
        $input
} > c_api.rs
