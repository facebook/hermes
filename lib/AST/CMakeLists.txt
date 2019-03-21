add_llvm_library(hermesAST
    ASTBuilder.cpp
    ESTree.cpp
    ESTreeJSONDumper.cpp
    RecursiveVisitor.h
    SemValidate.cpp
    SemanticValidator.cpp SemanticValidator.h
    CommonJS.cpp
    LINK_LIBS
    hermesSupport
)
