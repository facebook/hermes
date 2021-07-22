# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if(HERMES_ENABLE_LIBFUZZER AND HERMES_ENABLE_LPMFUZZERS)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(HERMES_ENABLE_EH ON)

    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/tools/fuzzers/fuzzer-json/json.pb.cc)
        message(FATAL_ERROR "Proto file is not compiled, run tools/fuzzers/fuzzer-json/compile_proto.sh")
    endif()

    add_hermes_tool(fuzzer-json
        fuzzer-json.cpp
        json.pb.cc
        ${ALL_HEADER_FILES}
    )

    target_compile_options(fuzzer-json PRIVATE "-fsanitize=fuzzer-no-link")
    append(${HERMES_FUZZING_FLAG} CMAKE_EXE_LINKER_FLAGS)
    target_include_directories(fuzzer-json PRIVATE
        ${FUZZING_UTILS_DIR}/libprotobuf-mutator
        ${FUZZING_UTILS_DIR}/LPM/external.protobuf/include
        ${HERMES_SOURCE_DIR}/API
    )
    target_link_libraries(fuzzer-json
        PRIVATE
        hermesapi
        hermesSupport
        ${FUZZING_UTILS_DIR}/LPM/src/libfuzzer/libprotobuf-mutator-libfuzzer.a
        ${FUZZING_UTILS_DIR}/LPM/src/libprotobuf-mutator.a
        ${FUZZING_UTILS_DIR}/LPM/external.protobuf/lib/libprotobuf.a
    )
endif()
