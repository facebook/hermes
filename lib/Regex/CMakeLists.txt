# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
  RegexParser.cpp
  Executor.cpp
)

add_llvm_library(hermesRegex
    STATIC ${source_files}
    LINK_LIBS hermesPlatformUnicode
)
