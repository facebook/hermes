#!/usr/bin/env python
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import platform
import subprocess

from common import build_dir_suffix, get_parser, is_visual_studio, run_command, which


def parse_args():
    parser = get_parser()
    parser.add_argument("hermes_build_dir", type=str, nargs="?", default="build")
    parser.add_argument("llvm_build_dir", type=str, nargs="?", default="llvm_build")
    parser.add_argument("llvm_src_dir", type=str, nargs="?", default="llvm")
    parser.add_argument("--icu", type=str, dest="icu_root", default="")
    parser.add_argument("--fbsource", type=str, dest="fbsource_dir", default="")
    parser.add_argument("--opcode-stats", dest="opcode_stats", action="store_true")
    parser.add_argument(
        "--basic-block-profiler", dest="basic_block_profiler", action="store_true"
    )
    parser.add_argument(
        "--warnings-as-errors", dest="warnings_as_errors", action="store_true"
    )
    parser.add_argument("--static-link", dest="static_link", action="store_true")
    args = parser.parse_args()
    args.hermes_build_dir = os.path.realpath(args.hermes_build_dir)
    args.llvm_build_dir = os.path.realpath(args.llvm_build_dir)
    args.llvm_src_dir = os.path.realpath(args.llvm_src_dir)
    if args.icu_root:
        args.icu_root = os.path.realpath(args.icu_root)
    if args.fbsource_dir:
        args.fbsource_dir = os.path.realpath(args.fbsource_dir)

    args.build_type = args.build_type or ("MinSizeRel" if args.distribute else "Debug")
    suffix = build_dir_suffix(args)
    args.hermes_build_dir += suffix
    args.llvm_build_dir += suffix
    # Guess the ICU directory based on platform.
    if not args.icu_root and platform.system() == "Linux":
        icu_prefs = [
            "/mnt/gvfs/third-party2/icu/4e8f3e00e1c7d7315fd006903a9ff7f073dfc02b/53.1/gcc-5-glibc-2.23/9bc6787",
            "/mnt/gvfs/third-party2/icu/4e8f3e00e1c7d7315fd006903a9ff7f073dfc02b/53.1/gcc-4.8.1-glibc-2.17/c3f970a/",
        ]
        for pref in icu_prefs:
            if os.path.exists(pref):
                args.icu_root = pref
                break
    return args


def main():
    args = parse_args()
    print(
        "Building hermes using {} into {}".format(
            args.build_system, args.hermes_build_dir + os.path.sep
        )
    )

    try:
        os.mkdir(args.hermes_build_dir)
    except OSError:
        # It's alright if the file already exists.
        pass

    cmake_flags = args.cmake_flags.split() + [
        "-DLLVM_BUILD_DIR=" + args.llvm_build_dir,
        "-DLLVM_SRC_DIR=" + args.llvm_src_dir,
        "-DCMAKE_BUILD_TYPE=" + args.build_type,
    ]
    if args.is_32_bit:
        cmake_flags += ["-DLLVM_BUILD_32_BITS=On"]

    if (
        platform.system() == "Windows"
        and platform.machine().endswith("64")
        and is_visual_studio(args.build_system)
    ):
        cmake_flags += ["-Thost=x64"]
    if not args.distribute:
        cmake_flags += ["-DLLVM_ENABLE_ASSERTIONS=On"]
    if args.enable_asan:
        cmake_flags += ["-DLLVM_USE_SANITIZER=Address"]
    if args.opcode_stats:
        cmake_flags += ["-DHERMESVM_PROFILER_OPCODE=On"]
    if args.basic_block_profiler:
        cmake_flags += ["-DHERMESVM_PROFILER_BB=On"]
    if args.warnings_as_errors:
        cmake_flags += ["-DHERMES_ENABLE_WERROR=On"]
    if args.static_link:
        cmake_flags += ["-DHERMES_STATIC_LINK=On"]
    if args.fbsource_dir:
        cmake_flags += ["-DFBSOURCE_DIR=" + args.fbsource_dir]

    if args.icu_root:
        cmake_flags += ["-DICU_ROOT=" + args.icu_root]
    elif (
        os.environ.get("SANDCASTLE")
        and platform.system() != "macos"
        and platform.system() != "Windows"
    ):
        raise Exception("No ICU path provided on sandcastle")

    print("CMake flags: {}".format(" ".join(cmake_flags)))
    hermes_src_dir = os.path.realpath(__file__)
    # The hermes directory is three directories up from this file.
    # If this file is moved, make sure to update this.
    for _ in range(3):
        hermes_src_dir = os.path.dirname(hermes_src_dir)

    cmake = which("cmake")
    # Print the CMake version to assist in diagnosing issues.
    print(
        "CMake version:\n{}".format(
            subprocess.check_output([cmake, "--version"], stderr=subprocess.STDOUT)
        )
    )
    run_command(
        [cmake, hermes_src_dir, "-G", args.build_system] + cmake_flags,
        env=os.environ,
        cwd=args.hermes_build_dir,
    )


if __name__ == "__main__":
    main()
