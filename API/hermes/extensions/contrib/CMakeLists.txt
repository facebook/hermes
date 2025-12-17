# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This file exports the list of contrib extension C++ sources to the parent.
# JS files are globbed directly by the parent CMakeLists.txt.

set(CONTRIB_EXTENSIONS_CPP_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/ContribExtensions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ContribDummy.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/TextDecoder.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextDecoderUtils.cpp

  PARENT_SCOPE
)
