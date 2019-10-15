# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

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

