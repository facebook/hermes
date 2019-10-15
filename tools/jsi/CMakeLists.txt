# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

include_directories(${HERMES_SOURCE_DIR}/API)
include_directories(${HERMES_JSI_DIR})

add_llvm_tool(hermes-jsi
  jsi.cpp
  ${ALL_HEADER_FILES}
  )

set_target_properties(hermes-jsi PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  )

set_property(TARGET hermes-jsi APPEND_STRING PROPERTY
  COMPILE_FLAGS " -fexceptions"
  )

target_link_libraries(hermes-jsi
  hermesapi
  )
