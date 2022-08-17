# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(api_sources
  hermes.cpp
  DebuggerAPI.cpp
  )

file(GLOB api_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB api_public_headers ${PROJECT_SOURCE_DIR}/public/hermes/Public/*.h)

add_hermes_library(hermesapi
        ${api_sources}
        LINK_LIBS jsi hermesVMRuntime)
target_include_directories(hermesapi PUBLIC ..)

add_hermes_library(hermesapiLean
        ${api_sources}
        LINK_LIBS jsi hermesVMRuntimeLean)
target_include_directories(hermesapiLean PUBLIC ..)

add_hermes_library(synthTrace hermes_tracing.cpp SynthTrace.cpp TracingRuntime.cpp
  LINK_LIBS hermesapi)

add_hermes_library(timerStats TimerStats.cpp LINK_LIBS jsi hermesSupport)

add_hermes_library(traceInterpreter TraceInterpreter.cpp
  LINK_LIBS hermesapi hermesInstrumentation synthTrace synthTraceParser)

set(HERMES_LINK_COMPONENTS LLVHSupport)
set(HERMES_ENABLE_RTTI OFF)

# synthTraceParser uses exceptions but not rtti
add_hermes_library(synthTraceParser SynthTraceParser.cpp LINK_LIBS hermesSupport hermesParser synthTrace)

set(HERMES_ENABLE_EH OFF)

# compileJS uses neither exceptions nor RTTI
add_hermes_library(compileJS STATIC CompileJS.cpp LINK_LIBS hermesPublic)

# Restore EH and RTTI (Note: At the time of writing, there is no usage of
# add_hermes_library either after this line in this file or in a sub directory.
# However, the values are restored anyways for the sake of sanity.)
set(HERMES_ENABLE_EH ON)
set(HERMES_ENABLE_RTTI ON)

add_library(libhermes SHARED ${api_sources})

# This is configured using a cmake flag instead of a separate target, because
# we need the output to be named "libhermes.so".
set(LIBHERMES_VM_DEP hermesVMRuntime)
if(HERMES_BUILD_LEAN_LIBHERMES)
  set(LIBHERMES_VM_DEP hermesVMRuntimeLean)
endif()

target_link_libraries(libhermes
  PRIVATE
  jsi
  ${LIBHERMES_VM_DEP}
)
target_link_options(libhermes PRIVATE ${HERMES_EXTRA_LINKER_FLAGS})

# Export the required header directory
target_include_directories(libhermes PUBLIC .. ../../public ${HERMES_JSI_DIR})

set_target_properties(libhermes PROPERTIES
  # Avoid becoming liblibhermes (and there's already a target called 'hermes')
  OUTPUT_NAME hermes
)

if(APPLE AND HERMES_BUILD_APPLE_FRAMEWORK)
  set_target_properties(libhermes PROPERTIES
    FRAMEWORK TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
    FRAMEWORK_VERSION ${PROJECT_VERSION_MAJOR}
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_FRAMEWORK_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER dev.hermesengine.${HERMES_APPLE_TARGET_PLATFORM}
  )
  # Install headers into `Headers` while keeping required directory structure
  set_source_files_properties(${api_headers} PROPERTIES
    MACOSX_PACKAGE_LOCATION Headers
  )
  set_source_files_properties(${api_public_headers} PROPERTIES
    MACOSX_PACKAGE_LOCATION Headers/Public
  )
  if(HERMES_ENABLE_BITCODE)
    target_compile_options(libhermes PUBLIC "-fembed-bitcode")
    target_link_options(libhermes PUBLIC "-fembed-bitcode")
  endif()
  # Define the deployment target in the frameworks metadata
  if(HERMES_APPLE_TARGET_PLATFORM MATCHES "iphone")
    add_custom_command(TARGET libhermes POST_BUILD
      COMMAND /usr/libexec/PlistBuddy -c "Add :MinimumOSVersion string ${CMAKE_OSX_DEPLOYMENT_TARGET}" $<TARGET_FILE_DIR:libhermes>/Info.plist
    )
  elseif(HERMES_APPLE_TARGET_PLATFORM MATCHES "catalyst")
    add_custom_command(TARGET libhermes POST_BUILD
      COMMAND /usr/libexec/PlistBuddy -c "Add :LSMinimumSystemVersion string ${CMAKE_OSX_DEPLOYMENT_TARGET}" $<TARGET_FILE_DIR:libhermes>/Resources/Info.plist
    )
  elseif(HERMES_APPLE_TARGET_PLATFORM MATCHES "macos")
    add_custom_command(TARGET libhermes POST_BUILD
      COMMAND /usr/libexec/PlistBuddy -c "Add :LSMinimumSystemVersion string ${CMAKE_OSX_DEPLOYMENT_TARGET}" $<TARGET_FILE_DIR:libhermes>/Resources/Info.plist
    )
  endif()
endif()

install(TARGETS libhermes
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  FRAMEWORK DESTINATION Library/Frameworks/${HERMES_APPLE_TARGET_PLATFORM}
)
# Install headers into `include` while keeping required directory structure
install(DIRECTORY "${PROJECT_SOURCE_DIR}/API/hermes" DESTINATION include
  FILES_MATCHING PATTERN "*.h"
  PATTERN "synthtest" EXCLUDE)

# Create debug symbols (dSYM) bundle for Apple platform dylibs/frameworks
# Largely inspired by https://github.com/llvm/llvm-project/blob/6701993027f8af172d7ba697884459261b00e3c6/llvm/cmake/modules/AddLLVM.cmake#L1934-L1986
if(HERMES_BUILD_APPLE_DSYM)
  if(CMAKE_CXX_FLAGS MATCHES "-flto")
    set(lto_object ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/libhermes-lto.o)
    set_property(TARGET libhermes APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-object_path_lto,${lto_object}")
  endif()

  get_target_property(DSYM_PATH libhermes LOCATION)
  if(HERMES_BUILD_APPLE_FRAMEWORK)
    get_filename_component(DSYM_PATH ${DSYM_PATH} DIRECTORY)
  endif()
  set(DSYM_PATH "${DSYM_PATH}.dSYM")

  if(NOT CMAKE_DSYMUTIL)
    set(CMAKE_DSYMUTIL xcrun dsymutil)
  endif()
  add_custom_command(TARGET libhermes POST_BUILD
    COMMAND ${CMAKE_DSYMUTIL} $<TARGET_FILE:libhermes> --out ${DSYM_PATH}
    BYPRODUCTS ${DSYM_PATH}
  )

  if(HERMES_BUILD_APPLE_FRAMEWORK)
    install(DIRECTORY ${DSYM_PATH} DESTINATION Library/Frameworks/${HERMES_APPLE_TARGET_PLATFORM})
  else()
    install(DIRECTORY ${DSYM_PATH} DESTINATION lib)
  endif()
endif()
