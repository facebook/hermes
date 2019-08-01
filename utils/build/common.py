#!/usr/bin/env python
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
import os
import platform
import shutil
import subprocess
import sys


def get_parser():
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
    return parser


def is_visual_studio(build_system):
    return "Visual Studio" in build_system


def run_command(cmd, **kwargs):
    print("+ " + " ".join(cmd))
    return subprocess.check_call(cmd, stdout=sys.stdout, stderr=sys.stderr, **kwargs)


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


def build_dir_suffix(args):
    build_dir_suffix = ""
    if args.enable_asan:
        build_dir_suffix += "_asan"
    if args.distribute:
        build_dir_suffix += "_release"
    if args.is_32_bit:
        build_dir_suffix += "_32"
    return build_dir_suffix
