#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Read a Hermes bytecode file and emit a JS file that defines `BC` as a
# Uint8Array of its exact bytes. Used by worker-from-bytecode.js to feed
# precompiled bytecode to the Worker constructor from a pure-JS lit test.
#
# Usage: hbc_to_js.py <input.hbc> <output.js>

import sys

with open(sys.argv[1], "rb") as f:
    data = f.read()

with open(sys.argv[2], "w") as out:
    out.write("var BC = Uint8Array.from([" + ",".join(str(b) for b in data) + "]);\n")
