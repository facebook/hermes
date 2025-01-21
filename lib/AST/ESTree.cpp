/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/ESTree.h"

#include "llvh/Support/raw_ostream.h"

using llvh::dyn_cast;
using llvh::isa;

namespace hermes {
namespace ESTree {

NodeList &getParams(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return cast<ProgramNode>(node)->dummyParamList;
    case NodeKind::FunctionExpression:
      return cast<FunctionExpressionNode>(node)->_params;
    case NodeKind::ArrowFunctionExpression:
      return cast<ArrowFunctionExpressionNode>(node)->_params;
    case NodeKind::FunctionDeclaration:
      return cast<FunctionDeclarationNode>(node)->_params;
#if HERMES_PARSE_FLOW
    case NodeKind::ComponentDeclaration:
      return cast<ComponentDeclarationNode>(node)->_params;
    case NodeKind::HookDeclaration:
      return cast<HookDeclarationNode>(node)->_params;
#endif
  }
}

Node *getTypeParameters(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return nullptr;
    case NodeKind::FunctionExpression:
      return cast<FunctionExpressionNode>(node)->_typeParameters;
    case NodeKind::ArrowFunctionExpression:
      return cast<ArrowFunctionExpressionNode>(node)->_typeParameters;
    case NodeKind::FunctionDeclaration:
      return cast<FunctionDeclarationNode>(node)->_typeParameters;
#if HERMES_PARSE_FLOW
    case NodeKind::ComponentDeclaration:
      return cast<ComponentDeclarationNode>(node)->_typeParameters;
#endif
  }
}

BlockStatementNode *getBlockStatement(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return nullptr;
    case NodeKind::FunctionExpression:
      return cast<BlockStatementNode>(
          cast<FunctionExpressionNode>(node)->_body);
    case NodeKind::FunctionDeclaration:
      return cast<BlockStatementNode>(
          cast<FunctionDeclarationNode>(node)->_body);
    case NodeKind::ArrowFunctionExpression:
      return dyn_cast<BlockStatementNode>(
          cast<ArrowFunctionExpressionNode>(node)->_body);
#if HERMES_PARSE_FLOW
    case NodeKind::ComponentDeclaration:
      return cast<BlockStatementNode>(
          cast<ComponentDeclarationNode>(node)->_body);
    case NodeKind::HookDeclaration:
      return cast<BlockStatementNode>(cast<HookDeclarationNode>(node)->_body);
#endif
  }
}

Node *getIdentifier(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return nullptr;
    case NodeKind::FunctionExpression:
      return cast<FunctionExpressionNode>(node)->_id;
    case NodeKind::ArrowFunctionExpression:
      return cast<ArrowFunctionExpressionNode>(node)->_id;
    case NodeKind::FunctionDeclaration:
      return cast<FunctionDeclarationNode>(node)->_id;
  }
}

Node *getReturnType(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return nullptr;
    case NodeKind::FunctionExpression:
      return cast<FunctionExpressionNode>(node)->_returnType;
    case NodeKind::ArrowFunctionExpression:
      return cast<ArrowFunctionExpressionNode>(node)->_returnType;
    case NodeKind::FunctionDeclaration:
      return cast<FunctionDeclarationNode>(node)->_returnType;
  }
}

Node *getObject(MemberExpressionLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::MemberExpression:
      return cast<MemberExpressionNode>(node)->_object;
    case NodeKind::OptionalMemberExpression:
      return cast<OptionalMemberExpressionNode>(node)->_object;
    default:
      break;
  }
  llvm_unreachable("invalid MemberExpressionLikeNode");
}

Node *getProperty(MemberExpressionLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::MemberExpression:
      return cast<MemberExpressionNode>(node)->_property;
    case NodeKind::OptionalMemberExpression:
      return cast<OptionalMemberExpressionNode>(node)->_property;
    default:
      break;
  }
  llvm_unreachable("invalid MemberExpressionLikeNode");
}

NodeBoolean getComputed(MemberExpressionLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::MemberExpression:
      return cast<MemberExpressionNode>(node)->_computed;
    case NodeKind::OptionalMemberExpression:
      return cast<OptionalMemberExpressionNode>(node)->_computed;
    default:
      break;
  }
  llvm_unreachable("invalid MemberExpressionLikeNode");
}

Node *getCallee(CallExpressionLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::CallExpression:
      return cast<CallExpressionNode>(node)->_callee;
    case NodeKind::OptionalCallExpression:
      return cast<OptionalCallExpressionNode>(node)->_callee;
    default:
      break;
  }
  llvm_unreachable("invalid CallExpressionLikeNode");
}

NodeList &getArguments(CallExpressionLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::CallExpression:
      return cast<CallExpressionNode>(node)->_arguments;
    case NodeKind::OptionalCallExpression:
      return cast<OptionalCallExpressionNode>(node)->_arguments;
    default:
      break;
  }
  llvm_unreachable("invalid CallExpressionLikeNode");
}

bool hasSimpleParams(FunctionLikeNode *node) {
  for (Node &param : getParams(node)) {
    if (isa<PatternNode>(param))
      return false;
#if HERMES_PARSE_FLOW
    if (isa<ComponentParameterNode>(param) &&
        isa<PatternNode>(cast<ComponentParameterNode>(&param)->_local))
      return false;
#endif
  }
  return true;
}

bool isGenerator(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return false;
    case NodeKind::FunctionExpression:
      return cast<FunctionExpressionNode>(node)->_generator;
    case NodeKind::ArrowFunctionExpression:
      return false;
    case NodeKind::FunctionDeclaration:
      return cast<FunctionDeclarationNode>(node)->_generator;
#if HERMES_PARSE_FLOW
    case NodeKind::ComponentDeclaration:
      return false;
    case NodeKind::HookDeclaration:
      return false;
#endif
  }
}

bool isAsync(FunctionLikeNode *node) {
  switch (node->getKind()) {
    default:
      assert(
          node->getKind() == NodeKind::Program && "invalid FunctionLikeNode");
      return false;
    case NodeKind::FunctionExpression:
      return cast<FunctionExpressionNode>(node)->_async;
    case NodeKind::ArrowFunctionExpression:
      return cast<ArrowFunctionExpressionNode>(node)->_async;
    case NodeKind::FunctionDeclaration:
      return cast<FunctionDeclarationNode>(node)->_async;
#if HERMES_PARSE_FLOW
    case NodeKind::ComponentDeclaration:
      return false;
    case NodeKind::HookDeclaration:
      return false;
#endif
  }
}

Node *getSuperClass(ClassLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::ClassExpression:
      return cast<ClassExpressionNode>(node)->_superClass;
    case NodeKind::ClassDeclaration:
      return cast<ClassDeclarationNode>(node)->_superClass;
    default:
      break;
  }
  llvm_unreachable("invalid ClassLikeNode");
}

IdentifierNode *getClassID(ClassLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::ClassExpression:
      return llvh::dyn_cast_or_null<IdentifierNode>(
          cast<ClassExpressionNode>(node)->_id);
    case NodeKind::ClassDeclaration:
      return dyn_cast<IdentifierNode>(cast<ClassDeclarationNode>(node)->_id);
    default:
      break;
  }
  llvm_unreachable("invalid ClassLikeNode");
}

ClassBodyNode *getClassBody(ClassLikeNode *node) {
  switch (node->getKind()) {
    case NodeKind::ClassExpression:
      return cast<ClassBodyNode>(cast<ClassExpressionNode>(node)->_body);
    case NodeKind::ClassDeclaration:
      return cast<ClassBodyNode>(cast<ClassDeclarationNode>(node)->_body);
    default:
      break;
  }
  llvm_unreachable("invalid ClassLikeNode");
}

} // namespace ESTree
} // namespace hermes
