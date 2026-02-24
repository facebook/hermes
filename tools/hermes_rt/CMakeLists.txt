# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
#
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.

# hermes_rt — lightweight JS test runner that exercises hermes.dll via the
# public C API.  Used by lit tests to verify DLL behaviour, especially Intl
# provider switching.

add_executable(hermes_rt hermes_rt.cpp)
target_link_libraries(hermes_rt PRIVATE libshared)
target_link_options(hermes_rt PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

# Output to bin/ alongside other hermes tools so lit can find it.
set_property(TARGET hermes_rt PROPERTY RUNTIME_OUTPUT_DIRECTORY
  "${HERMES_TOOLS_OUTPUT_DIR}")

# hermes_icu.h lives under include/hermes/Platform/Intl/
target_include_directories(hermes_rt PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

# Copy hermes.dll next to hermes_rt.exe so it can be found at runtime.
# Use a standalone custom command (not POST_BUILD) so the copy re-runs
# whenever hermes.dll is rebuilt, even if hermes_rt.exe itself is up to date.
add_custom_command(
  OUTPUT "${HERMES_TOOLS_OUTPUT_DIR}/hermes.dll"
  VERBATIM
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:libshared>
    "${HERMES_TOOLS_OUTPUT_DIR}"
  DEPENDS libshared
  COMMENT "Copying hermes.dll to bin/"
)
add_custom_target(hermes_rt_copy_dll ALL
  DEPENDS "${HERMES_TOOLS_OUTPUT_DIR}/hermes.dll"
)
add_dependencies(hermes_rt hermes_rt_copy_dll)

# Copy hermes-icu.dll next to hermes_rt.exe so the bundled ICU provider is
# available.
if(TARGET hermes-icu)
  add_custom_command(
    OUTPUT "${HERMES_TOOLS_OUTPUT_DIR}/hermes-icu.dll"
    VERBATIM
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:hermes-icu>
      "${HERMES_TOOLS_OUTPUT_DIR}"
    DEPENDS hermes-icu
    COMMENT "Copying hermes-icu.dll to bin/"
  )
  add_custom_target(hermes_rt_copy_icu ALL
    DEPENDS "${HERMES_TOOLS_OUTPUT_DIR}/hermes-icu.dll"
  )
  add_dependencies(hermes_rt hermes_rt_copy_icu)
endif()
