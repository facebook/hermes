# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(SourceMapSources
  SourceMapTest.cpp
  )

add_hermes_unittest(HermesSourceMapTests
  ${SourceMapSources}
  )

target_link_libraries(HermesSourceMapTests
 hermesParser
 hermesSourceMap
 )
