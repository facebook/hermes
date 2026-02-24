# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# TODO: We would like to enable this, to demonstrate that we get narrowing
# conversion errors in some cases, but now when using the HermesSafeMath primitive
# for doing so.  But that will have to wait until we silence the errors in llvh.
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wshorten-64-to-32")

if(STATIC_ASSERT_TESTS)
  add_hermes_library(hermesSafeMathStaticAssertTest
    HermesSafeMathStaticAssertTest.cpp
  )
endif()
