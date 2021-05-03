# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Check LPM installation
if (HERMES_ENABLE_LPMFUZZERS)
    if (EXISTS ${PROJECT_SOURCE_DIR}/tools/fuzzers/utils/LPM)
        message(STATUS "LPM already installed")
        set(FUZZING_UTILS_DIR ${PROJECT_SOURCE_DIR}/tools/fuzzers/utils)
    else()
        message(FATAL_ERROR "LPM is not available, run the
            script in tools/fuzzers/utils/prepare-env.sh to install it")
    endif()
endif()

add_subdirectory(fuzzer-json)
add_subdirectory(fuzzer-jsi-entry)
add_subdirectory(fuzzilli)
