# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_OPTIONAL_SOURCES
  hermes.cpp
  hermes_tracing.cpp
  hermes_tracing_compat.cpp
  DebuggerAPI.cpp
  SynthTrace.cpp
  SynthTraceParser.cpp
  TraceInterpreter.cpp
  TracingRuntime.cpp
  CompileJS.cpp
  )

set(api_sources
  hermes.cpp
  DebuggerAPI.cpp
  )

if (HERMESVM_API_TRACE)
  list(APPEND api_sources
    hermes_tracing.cpp
    SynthTrace.cpp
    SynthTraceParser.cpp
    TraceInterpreter.cpp
    TracingRuntime.cpp
    )
else()
  list(APPEND api_sources hermes_tracing_compat.cpp)
endif()

add_hermes_library(hermesapi
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

set(HERMES_LINK_COMPONENTS LLVHSupport)

# Disable EH and RTTI for compileJS
set(HERMES_ENABLE_EH OFF)
set(HERMES_ENABLE_RTTI OFF)

add_hermes_library(compileJS STATIC CompileJS.cpp)

# Restore EH and RTTI (Note: At the time of writing, there is no usage of
# add_hermes_library either after this line in this file or in a sub directory.
# However, the values are restored anyways for the sake of sanity.)
set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)

add_library(libhermes SHARED ${api_sources})
target_link_libraries(libhermes
  jsi
  hermesVMRuntime
  ${CORE_FOUNDATION}
)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  # Export all API symbols
  set(compile_flags "-fvisibility=default")
  # The linker doesn't support -flto -Os, so make sure it's linked with -O3.
  # This only affects linking, not compilation, so without -flto it's a no-op.
  # In other words, if the compiler flags are -O0 -fno-lto, then the resulting
  # library will be -O0 in spite of setting -O3 here.
  set(link_flags "-O3 ${OPTIONAL_GC_SECTIONS} ${OPTIONAL_JNI_ONLOAD}")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
  # Same as above, but for windows. Note that there is no equivalent of -fvisibility=default.
  set(compile_flags "")
  set(link_flags "/O2 ${OPTIONAL_GC_SECTIONS} ${OPTIONAL_JNI_ONLOAD}")
endif()

set_target_properties(libhermes PROPERTIES
  COMPILE_FLAGS "${compile_flags}"
  LINK_FLAGS "${link_flags}"
  INTERFACE_COMPILE_DEFINITIONS "${HERMES_EXPORTED_COMPILE_DEFS}"
  # Declare our dependency on std::make_unique
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED 14
  # Avoid becoming liblibhermes (and there's already a target called 'hermes')
  OUTPUT_NAME hermes
  )
hermes_link_icu(libhermes)
