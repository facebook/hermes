# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
#
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.

set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)

# Windows-specific configuration for hermesvm targets (defined in ../hermes)
if(MSVC)
  # Enable exceptions and RTTI for hermesvm and hermesvmlean
  target_compile_options(hermesvm PRIVATE /EHsc /GR)
  target_compile_options(hermesvmlean PRIVATE /EHsc /GR)
  
  # Keep hermesvm.lib name on Windows (don't rename to hermes.lib) 
  # to avoid conflict with hermes.dll from libshared
  set_target_properties(hermesvm PROPERTIES OUTPUT_NAME hermesvm)
endif()

add_subdirectory(inspector)

add_library(hermesRuntimeApi INTERFACE
  hermes_api.h
  js_runtime_api.h
)

target_include_directories(hermesRuntimeApi INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(libshared SHARED
  hermes_api.cpp
  MurmurHash.cpp
  MurmurHash.h
  ScriptStore.h
)

target_link_libraries(libshared
  PUBLIC
    hermesRuntimeApi
    jsi
    nodeApi
  PRIVATE
    hermesinspector
    hermesNodeApi
    hermesvm_a
)
target_link_options(libshared PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

if(WIN32)
  configure_file(version.rc.in version.rc @ONLY)
  target_sources(libshared PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
endif()

# Export the required header directory
target_include_directories(libshared PUBLIC
  ..
  ../../public
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${HERMES_JSI_DIR}
  ${REACT_NATIVE_SOURCE}/ReactCommon
  ${REACT_NATIVE_SOURCE}/ReactCommon/runtimeexecutor
  ${REACT_NATIVE_SOURCE_72}/ReactCommon)

target_link_options(libshared PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

if(MSVC)
  # Enable exceptions and RTTI (required for shared library)
  target_compile_options(libshared PRIVATE /EHsc /GR)
  
  # Temporary avoid the auto-vectorization optimization since VS 17.14.0 produces incorrect code.
  target_compile_options(libshared PRIVATE $<$<CONFIG:Release>:/O1>)
endif()

set_target_properties(libshared PROPERTIES
  # To make sure that the resulting DLL name is hermes.dll
  OUTPUT_NAME hermes
)
