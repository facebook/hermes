# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(DtoaSources
  DtoaTest.cpp
  )

add_hermes_unittest(DtoaTests
  ${DtoaSources}
  )

target_link_libraries(DtoaTests
 dtoa
 )
