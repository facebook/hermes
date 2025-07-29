/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/AsyncGenerator.h"
#include "hermes/AST/TransformationsBase.h"

namespace hermes {

/**
 * Transforms JS async generators to generators according to babel
 * transformation
 * https://babeljs.io/docs/babel-plugin-transform-async-generator-functions
 *
 * An async generator:
 *
 * async function* userFunction(arguments) {
 *   // body containing await calls
 * }
 *
 * is transformed into:
 *
 * let userFunction = (() => {
 *      let _ref = _wrapAsyncGenerator(function* (arguments) { // body
 * containing await calls }); return function userFunction() { return
 * _ref.apply(this, arguments);
 *      };
 * })();
 *
 * and an await call inside the async generator body:
 *  await 1;
 *
 * is transformed into:
 *  yield _awaitAsyncGenerator(1);
 *
 * where _awaitAsyncGenerator and _wrapAsyncGenerator are helper functions
 * defined in 04-AsyncIterator.js
 *
 */
class AsyncGenerator : public TransformationsBase {
 public:
  AsyncGenerator(Context &context) : TransformationsBase(context) {}

  void visit(ESTree::FunctionDeclarationNode *funcDecl, ESTree::Node **ppNode) {
    if (funcDecl->_async && funcDecl->_generator) {
      recurseFunctionBody(funcDecl->_body, true);
      auto *refFunc = transformAsyncGeneratorFunction(
          funcDecl, funcDecl->_params, funcDecl->_body);

      *ppNode = createTransformedNode<ESTree::FunctionDeclarationNode>(
          funcDecl,
          funcDecl->_id,
          std::move(funcDecl->_params),
          refFunc,
          nullptr,
          nullptr,
          nullptr,
          false,
          false);
    } else {
      recurseFunctionBody(funcDecl->_body, false);
    }
  }

  void visit(ESTree::FunctionExpressionNode *funcExpr, ESTree::Node **ppNode) {
    if (funcExpr->_async && funcExpr->_generator) {
      recurseFunctionBody(funcExpr->_body, true);
      auto *refFunc = transformAsyncGeneratorFunction(
          funcExpr, funcExpr->_params, funcExpr->_body);

      *ppNode = createTransformedNode<ESTree::FunctionExpressionNode>(
          funcExpr,
          nullptr,
          std::move(funcExpr->_params),
          refFunc,
          nullptr,
          nullptr,
          nullptr,
          false,
          false);
    } else {
      recurseFunctionBody(funcExpr->_body, false);
    }
  }

  void visit(ESTree::AwaitExpressionNode *awaitExpr, ESTree::Node **ppNode) {
    if (!insideAsyncGenerator) {
      return;
    }

    auto *awaitCall = makeHermesInternalCall(
        awaitExpr, "_awaitAsyncGenerator", NodeVector{awaitExpr->_argument});
    *ppNode = createTransformedNode<ESTree::YieldExpressionNode>(
        awaitExpr, awaitCall, false);
  }

  void visit(ESTree::YieldExpressionNode *yieldExpr, ESTree::Node **ppNode) {
    if (!insideAsyncGenerator) {
      return;
    }

    auto awaitExpr = llvh::dyn_cast_or_null<ESTree::AwaitExpressionNode>(
        yieldExpr->_argument);
    if (awaitExpr) {
      visit(awaitExpr->_argument);
      *ppNode = createTransformedNode<ESTree::YieldExpressionNode>(
          yieldExpr, awaitExpr->_argument, false);
    }
  }

  void visit(ESTree::Node *node) {
    visitESTreeChildren(*this, node);
  }

 private:
  void recurseFunctionBody(ESTree::Node *body, bool insideAsyncGenerator) {
    // recursively transforms all nested async generators and async expressions
    bool oldInsideAsyncGenerator = this->insideAsyncGenerator;
    this->insideAsyncGenerator = insideAsyncGenerator;
    visitESTreeChildren(*this, body);
    this->insideAsyncGenerator = oldInsideAsyncGenerator;
  }

  ESTree::Node *transformAsyncGeneratorFunction(
      ESTree::Node *funcNode,
      ESTree::NodeList &params,
      ESTree::Node *body) {
    auto *refFunc = createTransformedNode<ESTree::FunctionExpressionNode>(
        funcNode,
        nullptr, // id is null for anonymous function
        std::move(params), // params
        body,
        nullptr, // typeParameters
        nullptr, // returnType
        nullptr, // predicate
        true, // generator
        false); // async

    auto *wrappedRef = isBodyEmpty(body)
        ? refFunc
        : makeHermesInternalCall(
              funcNode, "_wrapAsyncGenerator", NodeVector{refFunc});

    // Create a function body that calls apply on the wrapped function
    auto appliedBody = createTransformedNode<ESTree::BlockStatementNode>(
        funcNode,
        NodeVector{
            createTransformedNode<ESTree::ReturnStatementNode>(
                funcNode,
                createTransformedNode<ESTree::CallExpressionNode>(
                    funcNode,
                    createTransformedNode<ESTree::MemberExpressionNode>(
                        funcNode,
                        wrappedRef,
                        makeIdentifierNode(funcNode, "apply"),
                        false),
                    nullptr,
                    NodeVector{
                        createTransformedNode<ESTree::ThisExpressionNode>(
                            funcNode),
                        makeIdentifierNode(funcNode, "arguments")}
                        .toNodeList()))}
            .toNodeList(),
        /* implicit */ false);

    return appliedBody;
  }

  bool isBodyEmpty(ESTree::Node *body) {
    auto *blockStmt = llvh::dyn_cast<ESTree::BlockStatementNode>(body);
    return blockStmt && blockStmt->_body.empty();
  }

  ESTree::Node *getHermesInternalIdentifier(ESTree::Node *srcNode) override {
    return makeIdentifierNode(srcNode, "HermesAsyncIteratorsInternal");
  };

  bool insideAsyncGenerator = false;
};

ESTree::Node *transformAsyncGenerators(Context &context, ESTree::Node *node) {
  AsyncGenerator transformer(context);
  visitESTreeNode(transformer, node, nullptr);
  return node;
}

} // namespace hermes
