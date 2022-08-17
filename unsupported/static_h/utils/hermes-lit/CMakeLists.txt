# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# If we have a multi-configuration build system, we create a copy of llvm-lit in
# every directory.
if (NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
  foreach (BUILD_MODE ${CMAKE_CONFIGURATION_TYPES})
    string(REPLACE ${CMAKE_CFG_INTDIR} ${BUILD_MODE} bi ${HERMES_LIT_PATH})
    configure_file(
      hermes-lit.in
      ${bi}
    )
  endforeach ()
else ()
  set(BUILD_MODE "${CMAKE_BUILD_MODE}")
  configure_file(
    hermes-lit.in
    ${HERMES_LIT_PATH}
  )
endif ()
