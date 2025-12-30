# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH ON)
set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)

add_hermes_tool(hermes-jsi
  jsi.cpp
  ${ALL_HEADER_FILES}
  LINK_OBJLIBS hermesvm_a timerStats
  )

# Additional targets to test linking against the other library variants.
add_hermes_tool(hermes-jsi-lean
  jsi.cpp
  ${ALL_HEADER_FILES}
  LINK_OBJLIBS hermesvmlean_a timerStats
  )

add_hermes_tool(hermes-jsi-shared
  jsi.cpp
  ${ALL_HEADER_FILES}
  )
target_link_libraries(hermes-jsi-shared PRIVATE hermesvm timerStats)

add_hermes_tool(hermes-jsi-lean-shared
  jsi.cpp
  ${ALL_HEADER_FILES}
  )
target_link_libraries(hermes-jsi-lean-shared PRIVATE hermesvmlean timerStats)
