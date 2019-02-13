# We have to use separate flags for the GNU and Clang linkers, but
# we can't check whether GCC or Clang is being used because the NDK
# compiles with clang but links with GNU ld.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GCC" OR ${HERMES_IS_ANDROID})
  # GNU linker
  function(reexport_static_libs target)
    foreach(lib ${ARGN})
      target_link_libraries(${target} "-Wl,--whole-archive" ${lib} "-Wl,--no-whole-archive")
    endforeach()
  endfunction()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Clang linker
  function(reexport_static_libs target)
    foreach(lib ${ARGN})
      target_link_libraries(${target} "-Wl,-force_load,$<TARGET_FILE:${lib}>")
    endforeach()
  endfunction()
else()
  message(WARNING "libhermes will not be useful because there's "
                  "no reexport_static_libs macro defined.")
  function(reexport_static_libs target)
  endfunction()
endif()

set(LLVM_LINK_COMPONENTS
  Analysis
  Core
  Support
  )

add_library(libhermes SHARED hermes.cpp)
SET_TARGET_PROPERTIES(libhermes
  PROPERTIES OUTPUT_NAME hermes)

target_link_libraries(libhermes
  hermesapi
  ${CORE_FOUNDATION}
)

reexport_static_libs(libhermes
  hermesapi
)


hermes_link_icu(libhermes)
