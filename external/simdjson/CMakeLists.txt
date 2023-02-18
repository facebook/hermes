set(SRC src/simdjson.cpp)
add_hermes_library(simdjson STATIC ${SRC})
target_compile_definitions(simdjson PUBLIC SIMDJSON_EXCEPTIONS=OFF)
