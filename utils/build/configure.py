#!/usr/bin/env python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
import os
import platform
import shutil
import subprocess
import sys
import time
import warnings


def build_dir_suffix(args):
    suffices = []
    if args.enable_asan:
        suffices += ["asan"]
    if args.enable_ubsan:
        suffices += ["ubsan"]
    if args.enable_tsan:
        suffices += ["tsan"]
    if args.distribute:
        suffices += ["release"]
    if args.is_32_bit:
        suffices += ["32"]
    if args.wasm:
        suffices += ["wasm", args.emscripten_platform]
    return ("_" + "_".join(suffices)) if suffices else ""


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("hermes_build_dir", type=str, nargs="?", default=None)
    parser.add_argument(
        "--build-system",
        type=str,
        dest="build_system",
        default="Ninja",
        help="Generator to pass into CMake",
    )
    parser.add_argument(
        "--cmake-flags",
        type=str,
        dest="cmake_flags",
        default="",
        help="Additional flags to pass to CMake",
    )
    parser.add_argument(
        "--build-type",
        type=str,
        dest="build_type",
        choices=["MinSizeRel", "Debug"],
        default=None,
        help="Optimization level of build",
    )
    parser.add_argument(
        "--http-proxy",
        type=str,
        dest="http_proxy",
        default=os.environ.get("HTTP_PROXY", ""),
    )
    parser.add_argument("--distribute", action="store_true")
    parser.add_argument("--32-bit", dest="is_32_bit", action="store_true")
    parser.add_argument("--enable-asan", dest="enable_asan", action="store_true")
    parser.add_argument("--enable-ubsan", dest="enable_ubsan", action="store_true")
    parser.add_argument("--enable-tsan", dest="enable_tsan", action="store_true")
    parser.add_argument(
        "--code-coverage",
        dest="code_coverage",
        action="store_true",
        help="Enables code coverage to be collected from binaries. Coverage "
        'output will be placed in a subdirectory called "coverage" of the '
        "build directory",
    )
    parser.add_argument(
        "--enable-trace-pc-guard", dest="enable_trace_pc_guard", action="store_true"
    )
    parser.add_argument("--icu", type=str, dest="icu_root", default="")
    parser.add_argument("--unicode-lite", dest="unicode_lite", action="store_true")
    parser.add_argument("--fbsource", type=str, dest="fbsource_dir", default="")
    parser.add_argument("--jsidir", type=str, dest="jsi_dir", default="")
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
        default="upstream",
        help="Use either the upstream emscripten backend based on LLVM or the "
        "fastcomp backend. Note that the fastcomp backend is deprecated as of "
        "emscripten v2 and above",
    )
    args = parser.parse_args()
    if args.icu_root:
        args.icu_root = os.path.realpath(args.icu_root)
    if args.fbsource_dir:
        args.fbsource_dir = os.path.realpath(args.fbsource_dir)
    if args.jsi_dir:
        args.jsi_dir = os.path.realpath(args.jsi_dir)
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


def run_command(cmd, **kwargs):
    print("+ " + " ".join(cmd))
    retries = kwargs.pop("retries", 0)
    seconds_between_retries = kwargs.pop("seconds_between_retries", 10)

    while True:
        try:
            return subprocess.check_call(
                cmd, stdout=sys.stdout, stderr=sys.stderr, **kwargs
            )
        except subprocess.CalledProcessError as e:
            if retries == 0:
                raise
            retries -= 1
            print("Command failed, retrying soon: " + str(e))
            time.sleep(seconds_between_retries)
            print("Retrying...")


def which(cmd):
    if sys.version_info >= (3, 3):
        # On Python 3.3 and above, use shutil.which for a quick error message.
        resolved = shutil.which(cmd)
        if not resolved:
            raise Exception("{} not found on PATH".format(cmd))
        return resolved
    else:
        # Manually check PATH
        for p in os.environ["PATH"].split(os.path.pathsep):
            p = os.path.join(p, cmd)
            if "PATHEXT" in os.environ:
                # try out adding each extension to the PATH as well
                for ext in os.environ["PATHEXT"].split(os.path.pathsep):
                    # Add the extension.
                    p_and_extension = p + ext
                    if os.path.exists(p_and_extension) and os.access(
                        p_and_extension, os.X_OK
                    ):
                        return p_and_extension
            else:
                if os.path.isfile(p) and os.access(p, os.X_OK):
                    return p
        raise Exception("{} not found on PATH".format(cmd))


def python_executable_flag():
    if sys.executable and sys.version_info.major < 3:
        warnings.warn(
            "Configuring CMake with Python2. "
            "Python3 is recommended for the configuration of the Hermes build"
        )
    return ["-DPYTHON_EXECUTABLE={}".format(sys.executable or which("python"))]


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
        + python_executable_flag()
        + ["-DCMAKE_BUILD_TYPE=" + args.build_type]
    )

    if (
        platform.system() == "Windows"
        and platform.machine().endswith("64")
        and "Visual Studio" in args.build_system
    ):
        cmake_flags += ["-AWin32"] if args.is_32_bit else ["-Ax64"]
    if args.opcode_stats:
        cmake_flags += ["-DHERMESVM_PROFILER_OPCODE=ON"]
    if args.basic_block_profiler:
        cmake_flags += ["-DHERMESVM_PROFILER_BB=ON"]
    if args.warnings_as_errors:
        cmake_flags += ["-DHERMES_ENABLE_WERROR=ON"]
    if args.static_link:
        cmake_flags += ["-DHERMES_STATIC_LINK=ON"]
    if args.enable_asan:
        cmake_flags += ["-DHERMES_ENABLE_ADDRESS_SANITIZER=ON"]
    if args.enable_ubsan:
        cmake_flags += ["-DHERMES_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER=ON"]
    if args.enable_tsan:
        cmake_flags += ["-DHERMES_ENABLE_THREAD_SANITIZER=ON"]
    if args.code_coverage:
        cmake_flags += ["-DHERMES_ENABLE_CODE_COVERAGE=ON"]
    if args.enable_trace_pc_guard:
        cmake_flags += ["-DHERMES_ENABLE_TRACE_PC_GUARD=ON"]
    if args.fbsource_dir:
        cmake_flags += ["-DFBSOURCE_DIR=" + args.fbsource_dir]
    if args.jsi_dir:
        cmake_flags += ["-DJSI_DIR=" + args.jsi_dir]
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
            "-DEMSCRIPTEN_FASTCOMP=" + str(int(args.emscripten_platform == "fastcomp")),
        ]
    if args.unicode_lite:
        cmake_flags += ["-DHERMES_UNICODE_LITE=ON"]

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
