set(LLVM_LINK_COMPONENTS support)

add_llvm_library(hermesParser
        JSLexer.cpp
        JSONParser.cpp
        JSParser.cpp
        LINK_LIBS hermesSupport hermesAST dtoa
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

