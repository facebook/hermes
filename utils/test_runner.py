#!/usr/bin/env fbpython
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import asyncio
import sys

from testsuite.cli import main

if __name__ == "__main__":
    sys.exit(asyncio.run(main()))
