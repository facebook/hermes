#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [ $(which clang-tidy) ]; then
  FILES=`find lib include tools unittests -name \*.h -print -o -name \*.cpp -print`

  FARRAY=( $FILES ) # count the number of files to process
  echo  Inspecting ${#FARRAY[@]} files

  for F in $FILES; do
    clang-tidy $F -p ../build/ $1
    echo -n .
  done
  echo
  echo "Done"
  exit
fi

echo "ERROR: can't find clang-tidy in your path."
