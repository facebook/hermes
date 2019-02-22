set(LLVM_OPTIONAL_SOURCES
  hermes.cpp
  DebuggerAPI.cpp
  SynthTrace.cpp
  TraceInterpreter.cpp
  CompileJS.cpp
  )

set(api_sources
  hermes.cpp
  DebuggerAPI.cpp
  )

if (HERMESVM_SYNTH_REPLAY)
  list(APPEND api_sources
    SynthTrace.cpp
    TraceInterpreter.cpp
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
