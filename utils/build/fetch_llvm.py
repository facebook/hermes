#!/usr/bin/env python
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import platform
import subprocess

from common import (
    get_parser,
    run_command,
    which,
)


# It references the commit day so we can shallow clone
# and still manage to checkout this specific revision.
# NOTE: The revision date must be before the actual commit
# date.
_LLVM_REV = "c179d7b006348005d2da228aed4c3c251590baa3"
_LLVM_REV_DATE = "2018-10-07"


def parse_args():
    parser = get_parser()
    parser.add_argument("llvm_src_dir", type=str, nargs="?", default="llvm")
    args = parser.parse_args()
    args.llvm_src_dir = os.path.realpath(args.llvm_src_dir)
    return args


def build_git_command(http_proxy):
    # Otherwise, trust that the user has git on the path.
    command = [which("git")]
    if http_proxy:
        command += ["-c", "http.proxy={}".format(http_proxy)]
    if platform.system() == "Windows":
        command += ["-c", "core.filemode=false"]
        command += ["-c", "core.autocrlf=false"]
    return command


def clone_and_patch_llvm(args):
    git = build_git_command(args.http_proxy)
    if not os.path.exists(args.llvm_src_dir):
        # If the directory doesn't exist, clone LLVM there.
        print("Cloning LLVM into {}".format(args.llvm_src_dir))
        run_command(
            git
            + [
                "clone",
                "--shallow-since",
                _LLVM_REV_DATE,
                "https://github.com/llvm-mirror/llvm.git",
                args.llvm_src_dir,
            ],
            retries=3,
        )

    # Checkout a specific revision in LLVM.
    run_command(git + ["checkout", _LLVM_REV], cwd=args.llvm_src_dir)

    # Check that the respository is clean.
    try:
        run_command(git + ["diff-index", "--quiet", "HEAD"], cwd=args.llvm_src_dir)
    except subprocess.CalledProcessError:
        raise Exception("llvm dir is dirty (contains uncommitted changes)")

    # Apply small edits to LLVM from patch files.
    run_command(
        git
        + [
            "apply",
            "--ignore-space-change",
            "--ignore-whitespace",
            os.path.join(
                os.path.dirname(os.path.realpath(__file__)),
                "llvm-changes-for-hermes.patch",
            ),
        ],
        cwd=args.llvm_src_dir,
    )

    # Commit the patch.
    run_command(
        git
        + [
            "-c",
            "user.name=nobody",
            "-c",
            "user.email='nobody@example.com'",
            "commit",
            "-a",
            "-m",
            "Patch by Hermes build script",
        ],
        cwd=args.llvm_src_dir,
    )


def main():
    args = parse_args()
    print("Source Dir: {}".format(args.llvm_src_dir))
    clone_and_patch_llvm(args)


if __name__ == "__main__":
    main()
