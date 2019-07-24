#!/usr/bin/env python
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

# python build_hermes.py --target-platform windows --build-system "Visual Studio 15 2017 Win64" --build-type Debug --build-command MSBuild

import argparse
import os
import platform
import shutil
import subprocess
import sys

def parse_args():

    if platform.system() == "Linux":
        default_platform = "linux"
    elif platform.system() == "macos":
        default_platform = "macosx"
    elif platform.system() == "Windows":
        default_platform = "windows"
    else:
        default_platform = "unknown"

    parser = argparse.ArgumentParser()
    
    parser.add_argument(
        "--target-platform",
        type=str,
        dest="target_platform",
        choices=["linux", "macosx", "windows", "iphoneos", "iphonesimulator"],
        default=default_platform,
    )

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
        "--build-dir",
        type=str,
        dest="build_dir",
        default=None,
        help="build directory",
    )

    parser.add_argument(
        "--build-type",
        type=str,
        dest="build_type",
        choices=["MinSizeRel", "Debug"],
        default="Debug",
        help="Optimization level of build",
    )

    parser.add_argument(
        "--icu-root",
        type=str,
        dest="icu_root",
        default=None,
        help="ICU binaries root directory",
    )

    parser.add_argument(
        "--llvm-build-dir",
        type=str,
        dest="llvm_build_dir",
        default=None,
        help="LLVM build directory",
    )

    parser.add_argument(
        "--build-command",
        type=str,
        dest="build_command",
        default=None,
        help="Command to run once cmake finishes",
    )

    parser.add_argument(
        "--libfuzzer-path",
        type=str,
        dest="libfuzzer_path",
        default=None,
        help="libfuzzer path",
    )

    parser.add_argument(
        "--hermesvm-opcode-stats",
        dest="hermesvm_opcode_stats",
        action="store_true"
    )

    parser.add_argument(
        "--hermesvm-profiler-bb",
        dest="hermesvm_profiler_bb",
        action="store_true"
    )

    parser.add_argument(
        "--enable-werror",
        dest="enable_werror",
        action="store_true"
    )

    parser.add_argument(
        "--static-link",
        dest="static_link",
        action="store_true"
    )

    parser.add_argument(
        "--fb-source_dir",
        type=str,
        dest="fb_source_dir",
        default=None,
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
    parser.add_argument(
        "--cross-compile-only", dest="cross_compile_only", action="store_true"
    )
    args = parser.parse_args()
    


def main():
    args = parse_args()

    hermes_ws_dir = os.environ.get("HERMES_WS_DIR", "")
    if not os.path.isdir(hermes_ws_dir) or not os.path.isabs(hermes_ws_dir):
        raise Exception("HERMES_WS_DIR:{} must exist and must be an absolute path.".format(hermes_ws_dir))

    hermes_dir = os.path.join(hermes_ws_dir, "hermes")
    if not os.path.isfile(os.path.join(hermes_dir, "utils", "build_hermes.py")):
        raise Exception("Could not detect source dir.")

    # Get the path without symlinks, as CMake doesn't handle symlink/.. correctly
    hermes_dir = os.path.realpath(hermes_dir)

    os.chdir(hermes_ws_dir)

    build_system = args.build_system
    if not build_system:
        if args.target_platform == "windows":
            build_system = "Visual Studio 15 2017 Win64"
        else:
            build_system = Ninja

    build_dir_suffix = ""
    build_dir_suffix=build_dir_suffix + ("_asan" if args.enable-asan else "")
    build_dir_suffix = build_dir_suffix + ("_release" if args.distribute else "_debug")
    build_dir_suffix = build_dir_suffix + ("_32" if build_type else "")

    icu_root = args.icu_root
    if not icu_root and args.platform == 'linux':
        guess_path="/mnt/gvfs/third-party2/icu/4e8f3e00e1c7d7315fd006903a9ff7f073dfc02b/53.1/gcc-4.8.1-glibc-2.17/c3f970a/"
        icu_root = guess_path if os.isdir(guess_path) else None
            
    build_type = args.build_type
    if not build_type:
        if args.distribute:
            build_type = "MinSizeRel"
        else:
            build_type = "Debug"

    llvm_build_dir = args.llvm_build_dir
    build_dir = args.build_dir

    if not llvm_build_dir:
        llvm_build_dir = os.path.join(hermes_ws_dir, "llvm_build" + build_dir_suffix);

    if not build_dir:
        build_dir = os.path.join(hermes_ws_dir, "build" + build_dir_suffix);

    print("Building hermes using {} into {}".format(build_system, build_dir))

    os.path.mkdir(build_dir)
    os.path.chdir(build_dir)

    print("Hermes Path: {}".format(hermes_dir))

    cmake_flags=args.cmake_flags
    cmake_flags += " -DLLVM_BUILD_DIR=" + llvm_build_dir
    cmake_flags += " -DLLVM_SRC_DIR=" + os.path.join(hermes_ws_dir, "llvm")
    cmake_flags += " -DCMAKE_BUILD_TYPE=" + build_type
    
    if args.is_32_bit:
        cmake_flags += " -DLLVM_BUILD_32_BITS=On"

    if platform.machine().endswith('64'):
        cmake_flags += " -Thost=x64"

    if not args.distribute:
        cmake_flags += " -DLLVM_ENABLE_ASSERTIONS=On"

    enable_asan=args.enable_asan
    if not args.libfuzzer_path:
        cmake_flags += " -DLIBFUZZER_PATH=" + args.libfuzzer_path
        enable_asan = true

    if enable_asan:
        cmake_flags += " -DLLVM_USE_SANITIZER=Address"

    if args.hermesvm_opcode_stats:
        cmake_flags += " -DHERMESVM_PROFILER_OPCODE=ON"

    if args.hermesvm_profiler_bb:
        cmake_flags += " -DHERMESVM_PROFILER_BB=ON"

    if args.enable_werror:
        cmake_flags += " -DHERMES_ENABLE_WERROR=ON"

    if args.static_link:
        cmake_flags += " -DHERMES_STATIC_LINK=ON"

    if not args.fb_source_dir:
        cmake_flags += " -DFBSOURCE_DIR=" + args.fb_source_dir

    if icu_root:
        cmake_flags += " -DICU_ROOT=" + icu_root
    else:
        if platform not in ("macosx", "windows"):
            raise Exception("No ICU path provided.")

    # TODO :: ICU & SANDCASTLE


    print("cmake flags: {}".format(cmake_flags));

    run_command(
        [
            which("cmake"),
            "{}".format(hermes_dir),
            "-G",
            "{}".format(build_system),
            "{}".format(args.llvm_src_dir),
        ]
        + cmake_flags,
        env=os.environ,
        cwd=args.llvm_build_dir,
    )

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