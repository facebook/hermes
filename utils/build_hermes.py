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

    parser.add_argument("hermes_src_dir", type=str, nargs="?", default=".")
    parser.add_argument("hermes_build_dir", type=str, nargs="?", default="hermes_build")
    
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
        default="",
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
        default="",
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
        default="",
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

    args.hermes_src_dir = os.path.realpath(args.hermes_src_dir)
    args.hermes_build_dir = os.path.realpath(args.hermes_build_dir)

    if not args.build_command:
        # Choose a default based on the build system chosen
        if args.build_system == "Ninja":
            args.build_command = "ninja"
        elif args.build_system == "Unix Makefiles":
            args.build_command = "make"
        elif "Visual Studio" in args.build_system:
            args.build_command = (
                "MSBuild.exe Hermes.sln -target:build -maxcpucount -verbosity:normal"
            )
        else:
            raise Exception("Unrecognized build system: {}".format(args.build_system))

    args.build_type = args.build_type or ("MinSizeRel" if args.distribute else "Debug")

    if args.build_type == "Debug":
        args.build_command += " /p:Configuration=Debug"
    else:
        args.build_command += " /p:Configuration=MinSizeRel"

    return args
    
def which(cmd):
    if sys.version_info >= (3, 3):
        # On Python 3.3 and above, use shutil.which for a quick error message.
        resolved = shutil.which(cmd)
        if not resolved:
            raise Exception("{} not found on PATH".format(cmd))
        return os.path.realpath(resolved)
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
                        return os.path.realpath(p_and_extension)
            else:
                if os.path.isfile(p) and os.access(p, os.X_OK):
                    return os.path.realpath(p)
        raise Exception("{} not found on PATH".format(cmd))



def run_command(cmd, **kwargs):
    print("+ " + " ".join(cmd))
    return subprocess.check_call(cmd, stdout=sys.stdout, stderr=sys.stderr, **kwargs)

def main():
    args = parse_args()

    if not os.path.isdir(args.hermes_src_dir) or not os.path.isabs(args.hermes_src_dir):
        raise Exception("Hermes source directory:{} - must exist and must be an absolute path.".format(args.hermes_src_dir))

    # hermes_dir = os.path.join(hermes_ws_dir, "hermes")
    # if not os.path.isfile(os.path.join(hermes_dir, "utils", "build_hermes.py")):
    #    raise Exception("Could not detect source dir.")

    # Get the path without symlinks, as CMake doesn't handle symlink/.. correctly
    # hermes_dir = os.path.realpath(hermes_dir)

    # os.chdir(hermes_ws_dir)

    if not args.build_system:
        if args.target_platform == "windows":
            args.build_system = "'Visual Studio 15 2017 Win64'"
        else:
            args.build_system = Ninja


    if not args.icu_root and args.target_platform == 'linux':
        guess_path="/mnt/gvfs/third-party2/icu/4e8f3e00e1c7d7315fd006903a9ff7f073dfc02b/53.1/gcc-4.8.1-glibc-2.17/c3f970a/"
        args.icu_root = guess_path if os.isdir(guess_path) else None
            
    if not args.build_type:
        if args.distribute:
            args.build_type = "MinSizeRel"
        else:
            args.build_type = "Debug"

    build_dir_suffix = ""
    build_dir_suffix +=  ("_asan" if args.enable_asan else "")
    build_dir_suffix +=  ("_release" if args.distribute else "_debug")
    build_dir_suffix +=  ("_32" if args.is_32_bit else "")

    args.hermes_build_dir += build_dir_suffix

    if not args.llvm_build_dir:
        args.llvm_build_dir = os.path.join(args.hermes_src_dir, "llvm_build" + build_dir_suffix);

    #build_dir = os.path.join(hermes_ws_dir, "build" + build_dir_suffix);

    print("Building hermes using {} into {}".format(args.build_system, args.hermes_build_dir))

    if not os.path.exists(args.hermes_build_dir):
        os.mkdir(args.hermes_build_dir)

    os.chdir(args.hermes_build_dir)

    print("Hermes Path: {}".format(args.hermes_src_dir))

    cmake_flags = args.cmake_flags.split()
    cmake_flags += ["-DLLVM_BUILD_DIR=" + args.llvm_build_dir.replace('\\','/')]
    cmake_flags += ["-DLLVM_SRC_DIR=" + os.path.join(args.hermes_src_dir, "llvm").replace('\\','/')]
    cmake_flags += ["-DCMAKE_BUILD_TYPE=" + args.build_type]
    
    if args.is_32_bit:
        cmake_flags += ["-DLLVM_BUILD_32_BITS=On"]

    if platform.machine().endswith('64'):
        cmake_flags += ["-Thost=x64"]

    if not args.distribute:
        cmake_flags += ["-DLLVM_ENABLE_ASSERTIONS=On"]

    enable_asan=args.enable_asan
    if args.libfuzzer_path:
        cmake_flags += ["-DLIBFUZZER_PATH=" + args.libfuzzer_path]
        enable_asan = True

    if enable_asan:
        cmake_flags += ["-DLLVM_USE_SANITIZER=Address"]

    if args.hermesvm_opcode_stats:
        cmake_flags += ["-DHERMESVM_PROFILER_OPCODE=ON"]

    if args.hermesvm_profiler_bb:
        cmake_flags += ["-DHERMESVM_PROFILER_BB=ON"]

    if args.enable_werror:
        cmake_flags += ["-DHERMES_ENABLE_WERROR=ON"]

    if args.static_link:
        cmake_flags += ["-DHERMES_STATIC_LINK=ON"]

    if args.fb_source_dir:
        cmake_flags += ["-DFBSOURCE_DIR=" + args.fb_source_dir]

    if args.icu_root:
        cmake_flags += ["-DICU_ROOT=" + args.icu_root]
    else:
        if args.target_platform not in ("macosx", "windows"):
            raise Exception("No ICU path provided.")

    # TODO :: ICU & SANDCASTLE


    print("cmake flags: {}".format(cmake_flags));

    run_command(
        [
            which("cmake"),
            "-G",
            "{}".format(args.build_system),
            "{}".format(args.hermes_src_dir),
        ]
        + cmake_flags,
        env=os.environ,
        cwd=args.hermes_build_dir,
    )

    tries = 3 if "MSBuild" in args.build_command else 1
    for i in range(tries):
        try:
            run_command(args.build_command.split(), cwd=args.hermes_build_dir)
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