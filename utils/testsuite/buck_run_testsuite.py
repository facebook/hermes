#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import os
import shutil
import stat
import sys
import tempfile

import pkg_resources
from testsuite.testsuite import get_arg_parser, run


def main():
    parser = get_arg_parser()
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as binary_dir:
        # Copy binaries from pkg_resources into the temporary directory.
        resources = [
            ("hermes", "hermes"),
            ("hermesc", "hermesc"),
            (args.hvm_filename, args.hvm_filename),
            ("icudt/stubdata/icudt55l.dat", "icudt55l.dat"),
        ]
        for rsc, out_name in resources:
            with open(os.path.join(binary_dir, out_name), "w+b") as f:
                shutil.copyfileobj(pkg_resources.resource_stream("testsuite", rsc), f)
                st = os.stat(f.name)
                os.chmod(f.name, st.st_mode | stat.S_IEXEC)
        return run(
            args.paths,
            args.chunk,
            args.fail_fast,
            binary_dir,
            args.hvm_filename,
            args.jobs,
            args.verbose,
            args.match,
            args.source,
            args.test_skiplist,
            args.num_slowest_tests,
            args.workdir,
            args.nuke_workdir,
            args.generate_test_only,
            args.show_all,
            args.lazy,
            args.test_intl,
        )


if __name__ == "__main__":
    sys.exit(main())
