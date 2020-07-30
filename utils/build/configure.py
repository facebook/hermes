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
    build_dir_suffix,
    common_cmake_flags,
    get_parser,
    is_visual_studio,
    run_command,
    which,
)


def parse_args():
    parser = get_parser()
    parser.add_argument("hermes_build_dir", type=str, nargs="?", default=None)
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
    parser.add_argument(
        "--wasm",
        action="store_true",
        help="Build Hermes as WebAssembly instead of a native binary",
    )
    parser.add_argument(
        "--emscripten-root",
        dest="emscripten_root",
        help="Path to the root of emscripten. Use emsdk to download",
    )
    parser.add_argument(
        "--emscripten-platform",
        dest="emscripten_platform",
        choices=("upstream", "fastcomp"),
        default="fastcomp",
        help="Use either the upstream emscripten backend based on LLVM or the "
        "fastcomp backend",
    )
    args = parser.parse_args()
    if args.icu_root:
        args.icu_root = os.path.realpath(args.icu_root)
    if args.fbsource_dir:
        args.fbsource_dir = os.path.realpath(args.fbsource_dir)
    if args.emscripten_root:
        args.emscripten_root = os.path.realpath(args.emscripten_root)
    if args.wasm:
        # Check that if wasm is specified, that emscripten_root is also specified.
        if not args.emscripten_root:
            raise ValueError("WASM build requested, but emscripten-root not given")
        if not os.path.exists(args.emscripten_root):
            raise ValueError(
                "WASM build requested, but emscripten-root doesn't exist: "
                + args.emscripten_root
            )

    if not args.build_type:
        if args.distribute:
            # WASM doesn't need to be built to be small.
            args.build_type = "Release" if args.wasm else "MinSizeRel"
        else:
            args.build_type = "Debug"

    if not args.hermes_build_dir:
        args.hermes_build_dir = "build" + build_dir_suffix(args)
    args.hermes_build_dir = os.path.realpath(args.hermes_build_dir)

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

    cmake_flags = (
        args.cmake_flags.split()
        + common_cmake_flags()
        + ["-DCMAKE_BUILD_TYPE=" + args.build_type]
    )

    if (
        platform.system() == "Windows"
        and platform.machine().endswith("64")
        and is_visual_studio(args.build_system)
    ):
        cmake_flags += ["-Thost=x64"]
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
    if args.wasm:
        cmake_flags += [
            "-DCMAKE_TOOLCHAIN_FILE={}".format(
                os.path.join(
                    args.emscripten_root,
                    "cmake",
                    "Modules",
                    "Platform",
                    "Emscripten.cmake",
                )
            ),
            "-DCMAKE_EXE_LINKER_FLAGS="
            "-s NODERAWFS=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1",
        ]
        if args.emscripten_platform == "fastcomp":
            cmake_flags += ["-DEMSCRIPTEN_FASTCOMP=1"]

    if args.icu_root:
        cmake_flags += ["-DICU_ROOT=" + args.icu_root]
    elif os.environ.get("SANDCASTLE") and platform.system() not in (
        "macos",
        "Darwin",
        "Windows",
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
