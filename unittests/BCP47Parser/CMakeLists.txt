# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if(HERMES_ENABLE_INTL)
  add_hermes_unittest(BCP47ParserTests
    BCP47ParserTest.cpp
    )
  target_link_libraries(BCP47ParserTests
    hermesBCP47Parser
    )
endif()
