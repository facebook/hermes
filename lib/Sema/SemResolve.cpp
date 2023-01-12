/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/SemResolve.h"

#include "FlowChecker.h"
#include "FlowTypesDumper.h"
#include "SemanticResolver.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Support/PerfSection.h"

namespace hermes {
namespace sema {

class AnnotateDecl final {
  const flow::FlowContext &flowContext_;
  flow::FlowTypesDumper &flowDumper_;

 public:
  AnnotateDecl(
      const flow::FlowContext &flowContext,
      flow::FlowTypesDumper &flowDumper)
      : flowContext_(flowContext), flowDumper_(flowDumper) {}

  void annotateDecl(llvh::raw_ostream &os, const Decl *decl) {
    if (flow::Type *type = flowContext_.findDeclType(decl)) {
      os << " : ";
      flowDumper_.printTypeRef(os, type);
    }
  };
};

class ASTPrinter {
  llvh::raw_ostream &os_;
  SemContextDumper &semDumper_;
  flow::FlowTypesDumper *flowDumper_;
  const flow::FlowContext *flowContext_;
  unsigned depth_ = 0;

 public:
  ASTPrinter(
      llvh::raw_ostream &os,
      SemContextDumper &semDumper,
      flow::FlowTypesDumper *flowDumper = nullptr,
      const flow::FlowContext *flowContext = nullptr)
      : os_(os),
        semDumper_(semDumper),
        flowDumper_(flowDumper),
        flowContext_(flowContext) {}

  void run(ESTree::Node *root) {
    ESTree::ESTreeVisit(*this, root);
    os_ << "\n";
  }

  bool shouldVisit(ESTree::TypeAnnotationNode *V) {
    return false;
  }
  bool shouldVisit(ESTree::Node *V) {
    return true;
  }

  void enter(ESTree::Node *V) {
    ++depth_;
    os_ << llvh::left_justify("", (depth_ - 1) * 4);
    os_ << V->getNodeName();
    printNodeType(V);
    os_ << '\n';
  }
  void enter(ESTree::IdentifierNode *V) {
    ++depth_;
    os_ << llvh::left_justify("", (depth_ - 1) * 4);
    os_ << "Id '" << V->_name->str() << '\'';
    if (V->getDecl()) {
      os_ << " [";
      semDumper_.printDeclRef(os_, V->getDecl());
      os_ << ']';
    }
    printNodeType(V);
    os_ << '\n';
  }
  void leave(ESTree::Node *V) {
    --depth_;
  }

 private:
  void printNodeType(ESTree::Node *n) {
    if (!flowDumper_)
      return;
    if (auto *type = nodeType(n)) {
      os_ << " : ";
      flowDumper_->printTypeRef(os_, type);
    }
  }

  flow::Type *nodeType(ESTree::Node *n) const {
    return flowContext_ ? flowContext_->findNodeType(n) : nullptr;
  }
};

bool resolveAST(
    Context &astContext,
    SemContext &semCtx,
    flow::FlowContext *flowContext,
    ESTree::Node *root,
    const DeclarationFileListTy &ambientDecls) {
  PerfSection validation("Resolving JavaScript function AST");
  // Resolve the entire AST.
  DeclCollectorMapTy declCollectorMap{};
  SemanticResolver resolver{
      astContext, semCtx, ambientDecls, &declCollectorMap, true};
  if (!resolver.run(root))
    return false;

  if (flowContext) {
    flow::FlowChecker checker(astContext, *flowContext, declCollectorMap, true);
    if (!checker.run(llvh::cast<ESTree::ProgramNode>(root)))
      return false;
  }

  return true;
}

void semDump(
    llvh::raw_ostream &os,
    SemContext &semCtx,
    flow::FlowContext *flowContext,
    ESTree::Node *root) {
  if (!flowContext) {
    SemContextDumper semDumper;
    os << '\n';
    semDumper.printSemContext(os, semCtx);
    ASTPrinter ap(os, semDumper);
    ap.run(root);
  } else {
    flow::FlowTypesDumper flowDumper{};
    flowDumper.printAllTypes(os, *flowContext);
    os << '\n';

    AnnotateDecl annotateDecl(*flowContext, flowDumper);
    SemContextDumper semDumper(
        [&annotateDecl](llvh::raw_ostream &os, const Decl *decl) {
          annotateDecl.annotateDecl(os, decl);
        });

    semDumper.printSemContext(os, semCtx);
    os << '\n';
    ASTPrinter ap(os, semDumper, &flowDumper, flowContext);
    ap.run(root);
  }
}

bool resolveASTForParser(
    Context &astContext,
    SemContext &semCtx,
    ESTree::Node *root) {
  SemanticResolver resolver{astContext, semCtx, {}, nullptr, false};
  return resolver.run(root);
}

} // namespace sema
} // namespace hermes
