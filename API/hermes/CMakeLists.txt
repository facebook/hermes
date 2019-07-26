# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

set(LLVM_OPTIONAL_SOURCES
  hermes.cpp
  hermes_tracing.cpp
  DebuggerAPI.cpp
  SynthTrace.cpp
  TraceInterpreter.cpp
  TracingRuntime.cpp
  CompileJS.cpp
  )

set(api_sources
  hermes.cpp
  DebuggerAPI.cpp
  hermes_tracing.cpp
  )

if (HERMESVM_SYNTH_REPLAY)
  list(APPEND api_sources
    SynthTrace.cpp
    TraceInterpreter.cpp
    TracingRuntime.cpp
  )
endif()

add_llvm_library(hermesapi
        ${api_sources}
        LINK_LIBS jsi hermesVMRuntime)

set(hermesapi_compile_flags "")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  string(APPEND hermesapi_compile_flags " -Wno-non-virtual-dtor")
endif()

set_property(TARGET hermesapi PROPERTY
  CXX_STANDARD 14)
set_property(TARGET hermesapi APPEND_STRING PROPERTY
  COMPILE_FLAGS ${hermesapi_compile_flags})
set_property(TARGET hermesapi APPEND_STRING PROPERTY
  COMPILE_DEFINITIONS "JSI_CREATE_SHARED_LIBRARY")

set(LLVM_LINK_COMPONENTS
  Core
  Support
  )

# Disable EH and RTTI for compileJS
set(LLVM_REQUIRES_EH OFF)
set(LLVM_REQUIRES_RTTI OFF)

add_llvm_library(compileJS STATIC CompileJS.cpp)

# Restore EH and RTTI (Note: At the time of writing, there is no usage of
# add_llvm_library either after this line in this file or in a sub directory.
# However, the values are restored anyways for the sake of sanity.)
set(LLVM_REQUIRES_EH ON)
set(LLVM_REQUIRES_RTTI ON)

add_library(libhermes SHARED ${api_sources})
target_link_libraries(libhermes
  jsi
  hermesVMRuntime
  ${CORE_FOUNDATION}
)
set_target_properties(libhermes PROPERTIES
  # Declare our dependency on std::make_unique, and export all API symbols
  COMPILE_FLAGS "-std=c++14 -fvisibility=default"
  # The linker doesn't support -flto -Os, so make sure it's linked with -O3.
  # This only affects linking, not compilation, so without -flto it's a no-op.
  # In other words, if the compiler flags are -O0 -fno-lto, then the resulting
  # library will be -O0 in spite of setting -O3 here.
  LINK_FLAGS "-O3 ${OPTIONAL_GC_SECTIONS} ${OPTIONAL_JNI_ONLOAD}"
  # Avoid becoming liblibhermes (and there's already a target called 'hermes')
  OUTPUT_NAME hermes
  )
hermes_link_icu(libhermes)


install(TARGETS libhermes hermesapi
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)