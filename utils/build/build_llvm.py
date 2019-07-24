#!/usr/bin/env python
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import platform
import subprocess
import sys

from common import build_dir_suffix, get_parser, run_command, which


# It references the commit day so we can shallow clone
# and still manage to checkout this specific revision.
# NOTE: The revision date must be the day before the
# actual commit date.
_LLVM_REV = "c179d7b006348005d2da228aed4c3c251590baa3"
_LLVM_REV_DATE = "2018-10-08"


def parse_args():
    def default_build_command(args):
        # Choose a default based on the build system chosen
        if args.build_system == "Ninja":
            return "ninja"
        elif args.build_system == "Unix Makefiles":
            return "make"
        elif "Visual Studio" in args.build_system:
            return "MSBuild.exe LLVM.sln -target:build -maxcpucount -verbosity:normal"
        else:
            raise Exception("Unrecognized build system: {}".format(args.build_system))

    parser = get_parser()
    parser.add_argument("llvm_src_dir", type=str, nargs="?", default="llvm")
    parser.add_argument("llvm_build_dir", type=str, nargs="?", default="llvm_build")
    parser.add_argument(
        "--build-command",
        type=str,
        dest="build_command",
        default=None,
        help="Command to run once cmake finishes",
    )
    parser.add_argument(
        "--cross-compile-only", dest="cross_compile_only", action="store_true"
    )
    args = parser.parse_args()
    args.llvm_src_dir = os.path.realpath(args.llvm_src_dir)
    args.llvm_build_dir = os.path.realpath(args.llvm_build_dir)
    if not args.build_command:
        args.build_command = default_build_command(args)
    if args.cross_compile_only:
        args.build_command += " " + os.path.join("bin", "llvm-tblgen")
    args.build_type = args.build_type or ("MinSizeRel" if args.distribute else "Debug")
    args.llvm_build_dir += build_dir_suffix(args)
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
            ]
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
    print("Using Build system: {}".format(args.build_system))
    print("Using Build command: {}".format(args.build_command))
    print("Build Dir: {}".format(args.llvm_build_dir))
    print("Build Type: {}".format(args.build_type))

    clone_and_patch_llvm(args)

    cmake_flags = args.cmake_flags.split() + [
        "-DLLVM_TARGETS_TO_BUILD=",
        "-DCMAKE_BUILD_TYPE={}".format(args.build_type),
    ]
    if args.is_32_bit:
        cmake_flags += ["-DLLVM_BUILD_32_BITS=On"]
    if platform.system() == "Windows":
        if platform.machine().endswith("64"):
            cmake_flags += ["-Thost=x64"]
        cmake_flags += ["-DLLVM_INCLUDE_EXAMPLES=Off"]
    if args.enable_asan:
        cmake_flags += ["-DLLVM_USE_SANITIZER=Address"]
    cmake_flags += ["-DLLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO=Off"]

    if args.target_platform.startswith("iphone"):
        cmake_flags += [
            "-DCMAKE_C_FLAGS=-miphoneos-version-min=8.0",
            "-DCMAKE_CXX_FLAGS=-miphoneos-version-min=8.0",
            "-DCMAKE_TOOLCHAIN_FILE={}".format(
                os.path.join(args.llvm_src_dir, "cmake", "platforms", "iOS.cmake")
            ),
            "-DLLVM_BUILD_RUNTIME=Off",
            "-DLLVM_INCLUDE_TESTS=Off",
            "-DLLVM_INCLUDE_EXAMPLES=Off",
            "-DLLVM_ENABLE_BACKTRACES=Off",
            "-DLLVM_INCLUDE_UTILS=Off",
            "-DLLVM_ENABLE_TERMINFO=Off",
        ]
    if args.target_platform == "iphoneos":
        cmake_flags += ["-DCMAKE_OSX_ARCHITECTURES=armv7;arm64"]
    elif args.target_platform == "iphonesimulator":
        xcode_sysroot = subprocess.check_output(
            [which("xcodebuild"), "-version", "-sdk", "iphonesimulator", "Path"]
        )
        cmake_flags += [
            "-DCMAKE_OSX_ARCHITECTURES=x86_64",
            "-DCMAKE_OSX_SYSROOT={}".format(xcode_sysroot),
        ]
    elif args.target_platform == "macosx":
        cmake_flags += [
            "-DCMAKE_C_FLAGS=-mmacosx-version-min=10.9",
            "-DCMAKE_CXX_FLAGS=-mmacosx-version-min=10.9",
        ]

    cmake_flags += ["-DPYTHON_EXECUTABLE={}".format(sys.executable or which("python"))]

    try:
        os.mkdir(args.llvm_build_dir)
    except OSError:
        # It's alright if the file already exists.
        pass

    print("CMake flags: {}".format(" ".join(cmake_flags)))
    run_command(
        [which("cmake"), "-G", args.build_system, args.llvm_src_dir] + cmake_flags,
        env=os.environ,
        cwd=args.llvm_build_dir,
    )
    # MSBuild needs retries to handle an unexplainable linker error: LNK1000.
    tries = 3 if "MSBuild" in args.build_command else 1
    for i in range(tries):
        try:
            run_command(args.build_command.split(), cwd=args.llvm_build_dir)
            break
        except subprocess.CalledProcessError as e:
            if i == tries - 1:
                # If all retries failed, re-throw the last exception
                raise
            else:
                print("Exec failed: {}\nRetrying...".format(str(e)))
                continue


if __name__ == "__main__":
    main()
