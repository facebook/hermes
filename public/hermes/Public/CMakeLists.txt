# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH_RTTI ON)
add_hermes_library(hermesPublic rtti.cpp "${HERMES_COMMON_DIR}/public/hermes/Public/rtti_common.cpp")
target_include_directories(hermesPublic PUBLIC ../../ "${HERMES_COMMON_DIR}/public")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/public/hermes/Public" DESTINATION include/hermes
  FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "${HERMES_COMMON_DIR}/public/hermes/Public" DESTINATION include/hermes
  FILES_MATCHING PATTERN "*.h")
