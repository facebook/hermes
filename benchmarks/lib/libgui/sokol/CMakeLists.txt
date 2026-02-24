# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(SOKOL_HEADERS
        sokol_app.h
        sokol_debugtext.h
        sokol_gfx.h
        sokol_glue.h
        sokol_imgui.h
        sokol_log.h
        sokol_time.h)
set(SOKOL_DEFINES)

if (EMSCRIPTEN)
    add_definitions(-DSOKOL_GLES2)
    add_library(sokol OBJECT sokol.c ${SOKOL_HEADERS})
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    add_library(sokol OBJECT sokol.c ${SOKOL_HEADERS})
    target_compile_options(sokol PRIVATE -x objective-c)
    if (1)
        set(SOKOL_DEFINES ${SOKOL_DEFINES} -DSOKOL_METAL)
        target_link_libraries(sokol PUBLIC
                "-framework QuartzCore"
                "-framework Cocoa"
                "-framework MetalKit"
                "-framework Metal")
    else ()
        set(SOKOL_DEFINES ${SOKOL_DEFINES} -DSOKOL_GLCORE33)
        target_link_libraries(sokol PUBLIC
                "-framework QuartzCore"
                "-framework Cocoa"
                "-framework OpenGL")
    endif ()
else ()
    set(SOKOL_DEFINES ${SOKOL_DEFINES} -DSOKOL_GLCORE33)
    add_library(sokol OBJECT sokol.c ${SOKOL_HEADERS})
    if (CMAKE_SYSTEM_NAME STREQUAL Linux)
        target_link_libraries(sokol INTERFACE X11 Xi Xcursor GL dl m)
        target_link_libraries(sokol PUBLIC Threads::Threads)
    endif ()
endif ()
target_link_libraries(sokol PUBLIC cimgui)
target_include_directories(sokol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(sokol PUBLIC ${SOKOL_DEFINES})
