# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE on)

set(no_rtti_no_exception_SRC
  chrome/JSONValueInterfaces.cpp
  chrome/MessageConverters.cpp
  chrome/MessageInterfaces.cpp
  chrome/MessageTypes.cpp
  )

add_hermes_library(hermesInspector_no_rtti_no_exception ${no_rtti_no_exception_SRC} LINK_LIBS jsi hermesapi)

set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)
set(rtti_exception_SRC
        chrome/CallbackOStream.cpp
        chrome/RemoteObjectConverters.cpp
        chrome/RemoteObjectsTable.cpp
        )

add_hermes_library(hermesInspector_rtti_exception
        ${rtti_exception_SRC}
        LINK_LIBS hermesInspector_no_rtti_no_exception hermesapi)

if(HERMES_ENABLE_TOOLS AND NOT WIN32)
  add_subdirectory(chrome/cli)
endif()
