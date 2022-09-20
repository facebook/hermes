# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(CMAKE_C_STANDARD 11)

add_executable(sh_debug_skel
  sh_debug_skel.c
  ${ALL_HEADER_FILES}
  )

target_link_libraries(sh_debug_skel hermesvm)
