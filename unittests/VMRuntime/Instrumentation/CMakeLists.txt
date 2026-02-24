# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(ISources
  ApproxIntegralTest.cpp
  ProcessStatsTest.cpp
)

add_hermes_unittest(HermesInstrumentationTests
  ${ISources}
)

target_link_libraries(HermesInstrumentationTests
  hermesInstrumentation
  )
