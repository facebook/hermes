#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Intended use:
#  If you build hermes in (say) ~/workspace/build, run
#    test_static_asserts.sh ~/workspace/build

build_dir="${1}"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd "${build_dir}" || exit
ninja FileCheck

outfile=$(mktemp)

cmake -D STATIC_ASSERT_TESTS=ON .

# This step is necessary, rather than just piping the result to FileCheck,
# to swallow any non-zero return code from the ninja invocation.
ninja test/static_asserts/libhermesSafeMathStaticAssertTest.a > "${outfile}" 2>&1

# Reset the STATIC_ASSERT_TESTS flag in the cmake cache.
cmake -D STATIC_ASSERT_TESTS=OFF .

# If we come to have many of these tests, we'd put their filenames in a list, and
# do the steps below in a loop.

"${build_dir}"/bin/FileCheck -implicit-check-not error: --dump-input-on-failure "${SCRIPT_DIR}"/HermesSafeMathStaticAssertTest.cpp < "$outfile"
fcResult="$?"

# If we add more tests, the loop discussed above will keep an "all passed" variable,
# and test that here.
if [ "${fcResult}" -eq 0 ]; then
    echo All pass
    exit 0
else
    echo Failures
    exit 1
fi
