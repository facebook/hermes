# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_library(hermesPublic INTERFACE)
target_include_directories(hermesPublic INTERFACE ../../)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/public/hermes/Public" DESTINATION include/hermes
  FILES_MATCHING PATTERN "*.h")
