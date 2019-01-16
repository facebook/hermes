add_llvm_library(hermesFlowParser
    FlowParser.cpp
    LINK_LIBS
    hermesSupport
    ${LIBFLOWPARSER}
)
