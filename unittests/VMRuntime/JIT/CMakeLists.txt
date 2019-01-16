set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

set(JITSources
    ExecHeapTest.cpp
    DisassemblerTest.cpp
    DiscoverBBTest.cpp
    PoolHeapTest.cpp
    x86_64_EmitterTest.cpp
)

add_hermes_unittest(JITTests
  ${JITSources}
  )

target_link_libraries(JITTests
  hermesVMRuntime
  ${CORE_FOUNDATION}
 )

hermes_link_icu(JITTests)

