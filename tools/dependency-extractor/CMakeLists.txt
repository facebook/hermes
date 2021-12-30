# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_LINK_COMPONENTS LLVHSupport)

add_hermes_tool(dependency-extractor
  dependency-extractor.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(dependency-extractor
  hermesAST
  hermesDependencyExtractor
  hermesParser
)

add_hermes_tool(emdependency-extractor
  emdependency-extractor.cpp
  ${ALL_HEADER_FILES}
  )

target_link_libraries(emdependency-extractor
  hermesAST
  hermesDependencyExtractor
  hermesParser
)

if (EMSCRIPTEN)
  target_link_options(emdependency-extractor PRIVATE "SHELL: -s NODERAWFS=0 -s WASM=1")
  target_link_options(emdependency-extractor PRIVATE "SHELL: -s ALLOW_MEMORY_GROWTH=1")
  target_link_options(emdependency-extractor PRIVATE
    "SHELL: -s EXTRA_EXPORTED_RUNTIME_METHODS=[cwrap,ccall]")
  target_link_options(emdependency-extractor PRIVATE "SHELL: -s MODULARIZE=1")
  target_link_options(emdependency-extractor PRIVATE "SHELL: -s EXPORT_NAME=createDependencyExtractor")
  target_link_options(emdependency-extractor PRIVATE
    "SHELL: -s BINARYEN_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 -s WASM_ASYNC_COMPILATION=0")
endif ()
