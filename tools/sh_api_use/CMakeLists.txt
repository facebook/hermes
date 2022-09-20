# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(CMAKE_C_STANDARD 11)

add_executable(sh_api_use
  sh_api_use.c
  ${ALL_HEADER_FILES}
  )

target_link_libraries(sh_api_use hermesvm)
