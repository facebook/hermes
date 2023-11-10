# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(RAYTRACER_O raytracer${CMAKE_C_OUTPUT_EXTENSION})
add_custom_command(OUTPUT ${RAYTRACER_O}
        COMMAND ${SHERMES} $<$<CONFIG:Debug>:-g3> -typed -c
        ffi_helpers.js asciiz.js raytracer.js
        -Wc,-I.
        -o ${CMAKE_CURRENT_BINARY_DIR}/${RAYTRACER_O}
        DEPENDS
        ${SHERMES}
        ffi_helpers.js asciiz.js raytracer.js
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(raytracer ${CMAKE_CURRENT_BINARY_DIR}/${RAYTRACER_O})
set_target_properties(raytracer PROPERTIES LINKER_LANGUAGE CXX)
target_link_directories(raytracer PRIVATE ${HERMES_BUILD}/lib)
target_link_libraries(raytracer hermesvm)

set(RAYTRACER_GUI_O raytracer-gui${CMAKE_C_OUTPUT_EXTENSION})
add_custom_command(OUTPUT ${RAYTRACER_GUI_O}
        COMMAND ${SHERMES} $<$<CONFIG:Debug>:-g3> -typed --exported-unit=demo -c
            ffi_helpers.js asciiz.js sapp.js js_externs.js gui_prelude.js raytracer.js raytracer-gui.js
        -Wc,-I.
        -o ${CMAKE_CURRENT_BINARY_DIR}/${RAYTRACER_GUI_O}
        DEPENDS
            ${SHERMES}
            ffi_helpers.js asciiz.js sapp.js js_externs.js gui_prelude.js raytracer.js raytracer-gui.js
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(raytracer-gui ${CMAKE_CURRENT_BINARY_DIR}/${RAYTRACER_GUI_O} js_externs_cwrap.c)
set_target_properties(raytracer-gui PROPERTIES LINKER_LANGUAGE CXX)
target_link_directories(raytracer-gui PRIVATE ${HERMES_BUILD}/lib)
target_link_libraries(raytracer-gui shgui hermesvm)

add_executable(rtcpp raytracer.cpp)
target_include_directories(rtcpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/)

add_executable(rtcpp-gui raytracer.cpp)
target_compile_definitions(rtcpp-gui PRIVATE -DUSE_GUI=1)
target_link_libraries(rtcpp-gui gui)
