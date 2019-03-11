add_llvm_library(hermesSourceMap
    SourceMap.cpp
    SourceMapGenerator.cpp
    SourceMapParser.cpp
    LINK_LIBS hermesParser
)
