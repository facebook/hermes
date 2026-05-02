# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
  ConsoleHost.cpp
)

add_hermes_library(hermesConsoleHost ${source_files}
  LINK_OBJLIBS hermesPublic hermesapi
)

if(HERMES_ENABLE_NAPI)
  target_link_libraries(hermesConsoleHost_obj hermesNapi)
  # Allow ConsoleHost to include hermes_napi_impl.h (the internal NAPI header).
  target_include_directories(hermesConsoleHost_obj PRIVATE
    ${PROJECT_SOURCE_DIR}/API/napi
  )
  target_compile_definitions(hermesConsoleHost_obj PRIVATE HERMES_ENABLE_NAPI)
endif()
