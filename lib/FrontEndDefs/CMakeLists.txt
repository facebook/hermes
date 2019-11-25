# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
  Builtins.cpp
)

add_llvm_library(hermesFrontEndDefs STATIC ${source_files})
