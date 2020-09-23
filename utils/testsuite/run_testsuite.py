#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import sys

import testsuite


def main():
    parser = testsuite.get_arg_parser()
    parser.add_argument(
        "-b",
        "--binary-path",
        dest="binary_path",
        default="./bin/",
        help="Path to binaries",
    )
    args = parser.parse_args()
    return testsuite.run(
        args.paths,
        args.chunk,
        args.fail_fast,
        args.binary_path,
        args.hvm_filename,
        args.jobs,
        args.verbose,
        args.match,
        args.source,
        args.test_skiplist,
        args.num_slowest_tests,
        args.keep_tmp,
        args.show_all,
        args.lazy,
    )


if __name__ == "__main__":
    sys.exit(main())
