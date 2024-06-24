#!/usr/bin/env fbpython
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import argparse
import subprocess
from typing import List


def _buck2_run(cmd: str, args: List[str]) -> List[str]:
    return ["buck2", "run", cmd, "--"] + args


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--hbcbundle",
        help="Path to an .hbcbundle file",
    )
    parser.add_argument(
        "--sourcemap",
        help="Path to a sourcemap file",
    )
    args = parser.parse_args()

    hbc_attribute = subprocess.Popen(
        _buck2_run(
            "//xplat/hermes/tools/hbc-attribute:hbc-attribute.run", [args.hbcbundle]
        ),
        stdout=subprocess.PIPE,
    )
    hbc_attribute_output, _ = hbc_attribute.communicate()

    symbolicate = subprocess.Popen(
        _buck2_run(
            "//xplat/js/tools/metro/packages/metro-symbolicate:symbolicate",
            [
                args.sourcemap,
                "--attribution",
            ],
        ),
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
    )
    symbolicate_output, _ = symbolicate.communicate(input=hbc_attribute_output)

    accumulate = subprocess.Popen(
        _buck2_run("//xplat/hermes/tools/hbc-attribute:accumulate", []),
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
    )
    output, _ = accumulate.communicate(input=symbolicate_output)
    print(output.decode("utf-8"))


if __name__ == "__main__":
    main()
