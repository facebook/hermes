# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(BLOBS)
foreach(file IN ITEMS ship enemy background)
    list(APPEND BLOBS blob_${file}.c)
    add_custom_command(
            OUTPUT blob_${file}.c
            COMMAND echo "\\#define NAME \\\"assets/${file}.png\\\"" > blob_${file}.c
            COMMAND echo "static const unsigned char blob[] = {" >> blob_${file}.c
            COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/xxd.py ${CMAKE_CURRENT_SOURCE_DIR}/assets/${file}.png >>
                blob_${file}.c
            COMMAND echo "\}\;" >> blob_${file}.c
            COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/register_blob.inc >> blob_${file}.c

            DEPENDS register_blob.inc assets/${file}.png
            COMMENT "Generating C array from ${file}.png"
    )
endforeach()

add_executable(imdemo-cpp imdemo-cpp.cpp ${BLOBS})
target_link_libraries(imdemo-cpp gui)
