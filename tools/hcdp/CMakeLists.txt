# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if(NOT HERMES_ENABLE_DEBUGGER)
  # Build stub executable when debugger is disabled
  add_hermes_tool(hcdp hcdp.cpp)
else()

  set(NO_EH_RTTI_SOURCES
    JSONHelpers.cpp
    )

  if (NOT HERMES_ENABLE_EH AND NOT HERMES_ENABLE_RTTI)
    if (GCC_COMPATIBLE)
      set_property(SOURCE ${NO_EH_RTTI_SOURCES} APPEND_STRING
        PROPERTY COMPILE_FLAGS "-fno-exceptions -fno-rtti")
    elseif (MSVC)
      set_property(SOURCE ${NO_EH_RTTI_SOURCES} APPEND_STRING
        PROPERTY COMPILE_FLAGS "/EHs-c- /GR-")
    endif ()
  endif ()

  set(HERMES_ENABLE_RTTI ON)
  set(HERMES_ENABLE_EH ON)

  add_hermes_tool(hcdp
    hcdp.cpp
    IPC.cpp
    ${NO_EH_RTTI_SOURCES}
    ${ALL_HEADER_FILES}
    )

  target_link_libraries(hcdp hermesvm_a)
endif()

install(TARGETS hcdp
  RUNTIME DESTINATION bin
)
