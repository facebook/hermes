# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(LLVM_REQUIRES_EH ON)
set(LLVM_REQUIRES_RTTI ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${HERMES_JSI_DIR})

add_subdirectory(hermes)

