# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if(HERMES_ENABLE_LIBFUZZER)
    set(HERMES_ENABLE_EH ON)

    add_hermes_tool(fuzzer-jsi-entry
      fuzzer-jsi-entry.cpp
      ${ALL_HEADER_FILES}
    )

    target_compile_options(fuzzer-jsi-entry PRIVATE "-fsanitize=fuzzer-no-link")
    append(${HERMES_FUZZING_FLAG} CMAKE_EXE_LINKER_FLAGS)

    target_include_directories(fuzzer-jsi-entry PRIVATE ${HERMES_SOURCE_DIR}/API)
    target_link_libraries(fuzzer-jsi-entry
      PRIVATE
        hermesapi
    )
endif()
