# Copyright (c) Microsoft Corporation.
# Licensed under the MIT license.

include(CMakePrintHelpers)

# Set the boost source directory to the external folder
set(BOOST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/boost2)

set(boostComponents
  algorithm
  asio
  assert
  bind
  concept_check
  config
  container_hash
  core
  detail
  function
  integer
  iterator
  move
  mpl
  preprocessor
  range
  static_assert
  throw_exception
  type_index
  type_traits
  utility
  variant
)

# Check if all boost components exist in external directory
foreach(component ${boostComponents})
  if(NOT EXISTS ${BOOST_SOURCE_DIR}/${component})
    message(FATAL_ERROR 
      "Boost ${component} source not found at ${BOOST_SOURCE_DIR}/${component}. "
      "Please ensure boost 1.76.0 components are extracted to external/boost2/")
  endif()
  
  list(APPEND boostIncludes ${BOOST_SOURCE_DIR}/${component}/include)
endforeach()

set(BOOST_INCLUDES ${boostIncludes} PARENT_SCOPE)
