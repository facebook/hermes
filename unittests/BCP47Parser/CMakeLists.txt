# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(BCP47ParserSources
  BCP47Parser.cpp
  )

add_hermes_unittest(BCP47ParserTests
  ${BCP47ParserSources}
  )

target_link_libraries(BCP47ParserTests
  hermesBCP47Parser
  )
