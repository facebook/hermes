add_library(stb stb.c stb_image.h)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
