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
  string(APPEND hermesapi_compile_falgs " -Wno-covered-switch-default")
endif()

set_target_properties(hermesapi PROPERTIES
  CXX_STANDARD 14
  COMPILE_FLAGS "${hermesapi_compile_flags}"
  COMPILE_DEFINITIONS "JSI_CREATE_SHARED_LIBRARY"
  )

set(LLVM_LINK_COMPONENTS
  Core
  Support
  )

add_llvm_library(compileJS STATIC CompileJS.cpp)

set_target_properties(compileJS PROPERTIES
  COMPILE_FLAGS "-fno-rtti"
  )

add_library(libhermes SHARED ${api_sources})
target_link_libraries(libhermes
  hermesapi
  ${CORE_FOUNDATION}
)
set_target_properties(libhermes PROPERTIES
  # Declare our dependency on std::make_unique
  COMPILE_FLAGS "-std=c++14"
  # Avoid becoming liblibhermes (and there's already a target called 'hermes')
  OUTPUT_NAME hermes
  )
hermes_link_icu(libhermes)
