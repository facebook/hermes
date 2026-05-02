# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

find_library(LIBREADLINE_FOUND readline)
find_library(LIBTINFO_FOUND tinfo)

if(LIBREADLINE_FOUND)
  set(LIBREADLINE ${LIBREADLINE_FOUND})
endif()
if(LIBTINFO_FOUND)
  list(APPEND LIBREADLINE ${LIBTINFO_FOUND})
endif()
set(CMAKE_REQUIRED_LIBRARIES ${LIBREADLINE})
if(LIBREADLINE_FOUND)
  check_function_exists(readline HAVE_LIBREADLINE)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_hermes_tool(hermes
  hermes.cpp
  repl.cpp
  LINK_OBJLIBS hermesCompilerDriver_obj hermesConsoleHost_obj
  LINK_LIBS hermesvm_a ${LIBREADLINE}
  )

# NAPI module loading support: dlopen'd addons need to resolve NAPI API
# symbols (napi_create_string_utf8, etc.) from the host binary. Since the
# hermes binary doesn't directly call most NAPI functions, the linker
# would normally discard them from hermesvm_a. Force-loading the NAPI
# static library ensures all NAPI symbols are present. Additionally,
# -export_dynamic / -rdynamic makes them visible to dlopen'd libraries.
if(HERMES_ENABLE_NAPI)
  if(APPLE)
    target_link_libraries(hermes -force_load hermesNapi)
    target_link_options(hermes PRIVATE -Wl,-export_dynamic)
  elseif(MSVC)
    target_link_libraries(hermes hermesNapi)
    target_link_options(hermes PRIVATE /WHOLEARCHIVE:hermesNapi)
  else()
    target_link_libraries(hermes
      -Wl,--whole-archive
      hermesNapi
      -Wl,--no-whole-archive
    )
    target_link_options(hermes PRIVATE -rdynamic)
  endif()
endif()

if(HAVE_LIBREADLINE)
  target_compile_definitions(hermes PRIVATE HAVE_LIBREADLINE)
  set_target_properties(hermes PROPERTIES UNITY_BUILD false)
endif()

install(TARGETS hermes
  RUNTIME DESTINATION bin
)
