# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_llvm_tool(fuzzer
  fuzzer.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(fuzzer
  hermesAST
  hermesHBCBackend
  hermesBackend
  hermesOptimizer
  hermesFrontend
  hermesParser
  hermesSupport
  hermesVMRuntime
  dtoa
)

# link libFuzzer.a from the libfuzzer path.
target_link_libraries(fuzzer ${LIBFUZZER_PATH})
