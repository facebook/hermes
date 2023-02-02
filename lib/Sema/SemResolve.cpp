/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/SemResolve.h"

#include "SemanticResolver.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Support/PerfSection.h"

namespace hermes {
namespace sema {

class ASTPrinter {
  llvh::raw_ostream &os_;
  SemContext &semCtx_;
  SemContextDumper &semDumper_;
  unsigned depth_ = 0;

 public:
  ASTPrinter(
      llvh::raw_ostream &os,
      SemContext &semCtx,
      SemContextDumper &semDumper)
      : os_(os), semCtx_(semCtx), semDumper_(semDumper) {}

  void run(ESTree::Node *root) {
    ESTree::ESTreeVisit(*this, root);
    os_ << "\n";
  }

  bool shouldVisit(ESTree::Node *V) {
    return true;
  }

  void enter(ESTree::Node *V) {
    ++depth_;
    os_ << llvh::left_justify("", (depth_ - 1) * 4);
    os_ << V->getNodeName();
    printNodeType(V);
    printScopeRef(V);
    os_ << '\n';
  }
  void enter(ESTree::IdentifierNode *V) {
    ++depth_;
    os_ << llvh::left_justify("", (depth_ - 1) * 4);
    os_ << "Id '" << V->_name->str() << '\'';

    sema::Decl *declD = semCtx_.getDeclarationDecl(V);
    sema::Decl *exprD = semCtx_.getExpressionDecl(V);
    if (declD || exprD) {
      os_ << " [";
      if (!declD || declD == exprD) {
        os_ << "D:E:";
        semDumper_.printDeclRef(os_, exprD);
      } else if (exprD) {
        os_ << "D:";
        semDumper_.printDeclRef(os_, declD, false);
        os_ << " E:";
        semDumper_.printDeclRef(os_, exprD, true);
      } else {
        // The only remaining case.
        assert(declD && !exprD);
        os_ << "D:";
        semDumper_.printDeclRef(os_, declD);
      }
      os_ << ']';
    }
    if (V->isUnresolvable())
      os_ << " UNR";
    printNodeType(V);
    os_ << '\n';
  }
  void leave(ESTree::Node *V) {
    --depth_;
  }

 private:
  void printScopeRef(ESTree::Node *n) {
    if (auto *sd = ESTree::getDecoration<ESTree::ScopeDecorationBase>(n)) {
      if (sd->getScope()) {
        os_ << " ";
        semDumper_.printScopeRef(os_, sd->getScope());
      }
    }
  }

  void printNodeType(ESTree::Node *n) {}
};

bool resolveAST(
    Context &astContext,
    SemContext &semCtx,
    ESTree::ProgramNode *root,
    const DeclarationFileListTy &ambientDecls) {
  PerfSection validation("Resolving JavaScript global AST");
  // Resolve the entire AST.
  SemanticResolver resolver{astContext, semCtx, ambientDecls, true};
  return resolver.run(root);
}

bool resolveCommonJSAST(
    Context &astContext,
    SemContext &semCtx,
    ESTree::FunctionExpressionNode *root) {
  PerfSection validation("Resolving JavaScript CommonJS Module AST");
  // Resolve the entire AST.
  SemanticResolver resolver{astContext, semCtx, {}, true};
  return resolver.runCommonJSModule(root);
}

void semDump(llvh::raw_ostream &os, SemContext &semCtx, ESTree::Node *root) {
  SemContextDumper semDumper;
  semDumper.printSemContext(os, semCtx);
  os << '\n';
  ASTPrinter ap(os, semCtx, semDumper);
  ap.run(root);
}

bool resolveASTForParser(
    Context &astContext,
    SemContext &semCtx,
    ESTree::Node *root) {
  SemanticResolver resolver{astContext, semCtx, {}, false};
  return resolver.run(llvh::cast<ESTree::ProgramNode>(root));
}

} // namespace sema
} // namespace hermes
