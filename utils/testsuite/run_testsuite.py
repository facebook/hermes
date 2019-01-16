#!/usr/bin/env python3

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
        args.keep_tmp,
        args.show_all,
    )


if __name__ == "__main__":
    sys.exit(main())
