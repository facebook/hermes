/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
 * This file exposes a C API for use by Rust. It cannot actually be used from C
 * as is, because the function declarations themselves require C++ types.
 * Addressing that would require non-trivial effort, which for now we have
 * chosen not to do, given that we don't have a use case for C, just Rust. For
 * that reason we are not exposing a header file - there is no possible
 * consumer for that file.
 */

#include "hermes/Parser/JSParser.h"
#include "hermes/Support/SimpleDiagHandler.h"

// As explained above, we don't have a header with prototypes for this file.
#if defined(__GNUC__) && defined(__clang__)
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif

using llvh::cast;
using llvh::dyn_cast;
using llvh::isa;

using namespace hermes;
using namespace hermes::ESTree;

// While we return almost all attributes unmodified by value, that doesn't work
// for NodeList (since it is the head of a circular list) and we need to return
// a pointer to it.
// So some templating to adjust the return type of the getter depending on the
// type of the AST attribute.
namespace {
template <class T>
struct ReturnType {
  using Type = T;
  static inline Type get(T x) {
    return x;
  }
};

template <>
struct ReturnType<NodeList> {
  using Type = const NodeList *;
  static inline Type get(const NodeList &x) {
    return &x;
  }
};
} // namespace

#define ESTREE_FIRST(NAME, ...)
#define ESTREE_NODE_0_ARGS(NAME, ...)

// NOTE: -fomit-frame-pointer helps here

#define ESTREE_NODE_1_ARGS(NAME, BASE, ARG0TY, ARG0NM, ARG0OPT)     \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }

#define ESTREE_NODE_2_ARGS(                                         \
    NAME, BASE, ARG0TY, ARG0NM, ARG0OPT, ARG1TY, ARG1NM, ARG1OPT)   \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }

#define ESTREE_NODE_3_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }

#define ESTREE_NODE_4_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }

#define ESTREE_NODE_5_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT,                                                        \
    ARG4TY,                                                         \
    ARG4NM,                                                         \
    ARG4OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG4TY>::Type hermes_get_##NAME##_##ARG4NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG4TY>::get(node->_##ARG4NM);                \
  }

#define ESTREE_NODE_6_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT,                                                        \
    ARG4TY,                                                         \
    ARG4NM,                                                         \
    ARG4OPT,                                                        \
    ARG5TY,                                                         \
    ARG5NM,                                                         \
    ARG5OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG4TY>::Type hermes_get_##NAME##_##ARG4NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG4TY>::get(node->_##ARG4NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG5TY>::Type hermes_get_##NAME##_##ARG5NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG5TY>::get(node->_##ARG5NM);                \
  }

#define ESTREE_NODE_7_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT,                                                        \
    ARG4TY,                                                         \
    ARG4NM,                                                         \
    ARG4OPT,                                                        \
    ARG5TY,                                                         \
    ARG5NM,                                                         \
    ARG5OPT,                                                        \
    ARG6TY,                                                         \
    ARG6NM,                                                         \
    ARG6OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG4TY>::Type hermes_get_##NAME##_##ARG4NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG4TY>::get(node->_##ARG4NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG5TY>::Type hermes_get_##NAME##_##ARG5NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG5TY>::get(node->_##ARG5NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG6TY>::Type hermes_get_##NAME##_##ARG6NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG6TY>::get(node->_##ARG6NM);                \
  }

#define ESTREE_NODE_8_ARGS(                                         \
    NAME,                                                           \
    BASE,                                                           \
    ARG0TY,                                                         \
    ARG0NM,                                                         \
    ARG0OPT,                                                        \
    ARG1TY,                                                         \
    ARG1NM,                                                         \
    ARG1OPT,                                                        \
    ARG2TY,                                                         \
    ARG2NM,                                                         \
    ARG2OPT,                                                        \
    ARG3TY,                                                         \
    ARG3NM,                                                         \
    ARG3OPT,                                                        \
    ARG4TY,                                                         \
    ARG4NM,                                                         \
    ARG4OPT,                                                        \
    ARG5TY,                                                         \
    ARG5NM,                                                         \
    ARG5OPT,                                                        \
    ARG6TY,                                                         \
    ARG6NM,                                                         \
    ARG6OPT,                                                        \
    ARG7TY,                                                         \
    ARG7NM,                                                         \
    ARG7OPT)                                                        \
  extern "C" ReturnType<ARG0TY>::Type hermes_get_##NAME##_##ARG0NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG0TY>::get(node->_##ARG0NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG1TY>::Type hermes_get_##NAME##_##ARG1NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG1TY>::get(node->_##ARG1NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG2TY>::Type hermes_get_##NAME##_##ARG2NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG2TY>::get(node->_##ARG2NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG3TY>::Type hermes_get_##NAME##_##ARG3NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG3TY>::get(node->_##ARG3NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG4TY>::Type hermes_get_##NAME##_##ARG4NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG4TY>::get(node->_##ARG4NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG5TY>::Type hermes_get_##NAME##_##ARG5NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG5TY>::get(node->_##ARG5NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG6TY>::Type hermes_get_##NAME##_##ARG6NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG6TY>::get(node->_##ARG6NM);                \
  }                                                                 \
  extern "C" ReturnType<ARG7TY>::Type hermes_get_##NAME##_##ARG7NM( \
      const NAME##Node *node) {                                     \
    return ReturnType<ARG7TY>::get(node->_##ARG7NM);                \
  }

#include "hermes/AST/ESTree.def"

namespace {

/// This object contains the entire parser state.
struct Parsed {
  std::shared_ptr<Context> context;
  std::string error;
  ESTree::ProgramNode *ast = nullptr;

  explicit Parsed(std::string const &error) : error(error) {}
  explicit Parsed(
      std::shared_ptr<Context> const &context,
      ESTree::ProgramNode *ast)
      : context(context), ast(ast) {}
};

} // namespace

/// source is the zero terminated input. source[len-1] must be \0.
extern "C" Parsed *hermes_parser_parse(const char *source, size_t len) {
  if (len == 0 || source[len - 1] != 0) {
    return new Parsed("Input is not zero terminated");
  }

  CodeGenerationSettings codeGenOpts{};
  OptimizationSettings optSettings;
  auto context = std::make_shared<Context>(codeGenOpts, optSettings);
  SimpleDiagHandlerRAII outputManager{context->getSourceErrorManager()};

  parser::JSParser parser(*context, StringRef(source, len - 1));
  auto ast = parser.parse();

  if (outputManager.haveErrors()) {
    return new Parsed(outputManager.getErrorString());
  } else if (!ast) {
    // Just in case.
    return new Parsed("Internal error");
  }

  return new Parsed(context, *ast);
}

extern "C" void hermes_parser_free(Parsed *parsed) {
  delete parsed;
}

extern "C" const char *hermes_parser_get_error(Parsed *parsed) {
  return parsed->ast ? nullptr : parsed->error.c_str();
}

extern "C" ESTree::ProgramNode *hermes_parser_get_ast(Parsed *parsed) {
  return parsed->ast;
}

extern "C" const char *hermes_get_node_name(ESTree::Node *n) {
  return n->getNodeName().data();
}
