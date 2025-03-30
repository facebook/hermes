# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH_RTTI ON)

add_subdirectory(node_api)

add_library(hermesNodeApi STATIC hermes_node_api.cpp)
target_link_libraries(hermesNodeApi PRIVATE
  hermesVMRuntime
  nodeApi
  hermesParser)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
  # Same as above, but for windows. Note that there is no equivalent of -fvisibility=default.
  set(compile_flags "")

  # Enable exception
  set(compile_flags "${compile_flags} /EHsc")

  # Enable RTTI
  set(compile_flags "${compile_flags} /GR")

  # Generate PDBs
  set(compile_flags "${compile_flags} /Zi")

  set(link_flags_debug "/DEBUG")
  # Use /OPT:NOICF because hermes associates function pointer with its name.
  # The optimization that merges functions with the same body breaks that code.
  # See getFunctionNameImpl in lib\VM\JSNativeFunctions.cpp.
  set(link_flags_release "/DEBUG;/DEBUGTYPE:CV$<COMMA>FIXUP;/OPT:REF;/OPT:NOICF;/INCREMENTAL:NO")
  if(CMAKE_VS_PLATFORM_NAME MATCHES "^(x64|x86|Win32)$")
    # CETCOMPAT is not supported by ARM or ARM64
    list(APPEND link_flags_release "/CETCOMPAT")
  endif()
endif()

set_target_properties(hermesNodeApi PROPERTIES
  COMPILE_FLAGS "${compile_flags}"
  LINK_OPTIONS "$<IF:$<CONFIG:Debug>,${link_flags_debug},${link_flags_release}>"
)
