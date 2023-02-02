/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/SemContext.h"

namespace hermes {
namespace sema {

static llvh::FormattedString ind(unsigned level) {
  return llvh::left_justify("", level * 4);
}

SemContext::SemContext() = default;

SemContext::~SemContext() = default;

FunctionInfo *SemContext::newFunction(
    ESTree::FunctionLikeNode *funcNode,
    FunctionInfo *parentFunction,
    LexicalScope *parentScope,
    bool strict,
    SourceVisibility sourceVisibility) {
  functions_.emplace_back(
      funcNode, parentFunction, parentScope, strict, sourceVisibility);
  return &functions_.back();
}

LexicalScope *SemContext::newScope(
    FunctionInfo *parentFunction,
    LexicalScope *parentScope) {
  scopes_.emplace_back(parentFunction, parentScope);
  LexicalScope *res = &scopes_.back();
  parentFunction->scopes.push_back(res);
  return res;
}

Decl *SemContext::newDeclInScope(
    UniqueString *name,
    Decl::Kind kind,
    LexicalScope *scope,
    Decl::Special special) {
  decls_.emplace_back(Identifier::getFromPointer(name), kind, special, scope);
  auto res = &decls_.back();
  scope->decls.push_back(res);
  return res;
}

Decl *SemContext::newGlobal(hermes::UniqueString *name, Decl::Kind kind) {
  assert(Decl::isKindGlobal(kind) && "invalid global declaration kind");
  return newDeclInScope(name, kind, getGlobalScope());
}

Decl *SemContext::funcArgumentsDecl(
    FunctionInfo *func,
    UniqueString *argumentsName) {
  // Find the closest non-arrow ancestor.
  FunctionInfo *argumentsFunc = func;
  while (argumentsFunc->arrow) {
    if (argumentsFunc->parentFunction) {
      argumentsFunc = argumentsFunc->parentFunction;
    } else {
      break;
    }
  }

  // 'arguments' already exists, avoid redeclaring.
  if (argumentsFunc->argumentsDecl)
    return *argumentsFunc->argumentsDecl;

  Decl *decl;
  if (argumentsFunc == getGlobalFunction()) {
    // `arguments` must simply be treated as a global property in top level
    // contexts.
    decl = newDeclInScope(
        argumentsName,
        Decl::Kind::UndeclaredGlobalProperty,
        argumentsFunc->scopes.front());
  } else {
    // Otherwise, regular function-level "arguments" declaration.
    decl = newDeclInScope(
        argumentsName,
        Decl::Kind::Var,
        argumentsFunc->scopes.front(),
        Decl::Special::Arguments);
  }

  // Store it for future use.
  argumentsFunc->argumentsDecl = decl;

  return decl;
}

Decl *SemContext::getDeclarationDecl(ESTree::IdentifierNode *node) {
  using ID = ESTree::IdentifierDecoration;
  if (node->declState_ & ID::BitHaveDecl) {
    return (Decl *)node->decl_;
  } else if (node->declState_ & ID::BitSideDecl) {
    auto it = sideIdentifierDeclarationDecl_.find(node);
    assert(
        it != sideIdentifierDeclarationDecl_.end() &&
        "IdentifierNode with BitSideDecl must be in the side table");
    return it->second;
  } else {
    return nullptr;
  }
}

void SemContext::setDeclarationDecl(ESTree::IdentifierNode *node, Decl *decl) {
  using ID = ESTree::IdentifierDecoration;

  // Are we setting a "declaration decl" or erasing one?
  if (LLVM_LIKELY(decl)) {
    // We are setting a new "declaration decl". Update the state
    // correspondingly.
    switch (node->declState_) {
      // We have an existing "expression decl". Depending on whether the
      // "declaration decl" has the same value, either just set its bit, or
      // record it in the side table.
      case ID::BitHaveExpr:
        if (node->decl_ == decl) {
          node->declState_ = ID::BitHaveExpr | ID::BitHaveDecl;
        } else {
          node->declState_ = ID::BitHaveExpr | ID::BitSideDecl;
          sideIdentifierDeclarationDecl_[node] = decl;
        }
        break;

      // We have both an existing and "expression decl" and a "declaration
      // decl", which is in the side table. We have been asked to update the
      // "declaration decl". If the new value happens to be the same as the
      // "expression decl", we no longer need the side table, otherwise we just
      // update the value in the side table.
      case ID::BitHaveExpr | ID::BitSideDecl:
        if (node->decl_ == decl) {
          node->declState_ = ID::BitHaveExpr | ID::BitHaveDecl;
          bool erased = sideIdentifierDeclarationDecl_.erase(node);
          (void)erased;
          assert(
              erased &&
              "IdentifierNode with BitSideDecl must be in side table");
        } else {
          sideIdentifierDeclarationDecl_[node] = decl;
        }
        break;

      // We don't have an "expression decl", so we just update the "declaration
      // decl" and set the bit to know it is there.
      default:
        assert(
            (node->declState_ == 0 || node->declState_ == ID::BitHaveDecl) &&
            "Invalid declState");
        node->decl_ = decl;
        node->declState_ = ID::BitHaveDecl;
        break;
    }
  } else {
    // We are "unsetting" a "declaration decl". Update the state for that.
    switch (node->declState_) {
      // We have a "declaration decl" and an "expression decl" sharing the
      // same value. Just unset the bit for the "declaration decl".
      case ID::BitHaveDecl | ID::BitHaveExpr:
        node->declState_ = ID::BitHaveExpr;
        break;

      // We have only a "declaration decl". Unset the bit and clear the value.
      case ID::BitHaveDecl:
        node->declState_ = 0;
        node->decl_ = nullptr;
        break;

      // We have a "declaration decl" in the side table and an expression decl.
      // Remove the former from the side table and clear its bit.
      case ID::BitSideDecl | ID::BitHaveExpr: {
        node->declState_ = ID::BitHaveExpr;
        bool erased = sideIdentifierDeclarationDecl_.erase(node);
        (void)erased;
        assert(
            erased && "IdentifierNode with BitSideDecl must be in side table");
      } break;

      // We don't have a "declaration decl", so do nothing.
      default:
        assert(
            (node->declState_ == 0 || node->declState_ == ID::BitHaveExpr) &&
            "Invalid declState");
        break;
    }
  }
}

void SemContext::setExpressionDecl(ESTree::IdentifierNode *node, Decl *decl) {
  using ID = ESTree::IdentifierDecoration;

  // Are we setting an "expression decl" or erasing one?
  if (LLVM_LIKELY(decl)) {
    // We are setting a new "expression decl". Update the state
    // correspondingly.
    assert(
        !node->isUnresolvable() &&
        "Attempt to set decl for unresolvable identifier");

    switch (node->declState_) {
      // We already have a "declaration decl" and possibly an "expression decl".
      // Depending on whether the new "expression decl" has the same value as
      // the existing "declaration decl" or not, we have to move the existing
      // "declaration decl" into the side table.
      case ID::BitHaveDecl:
      case ID::BitHaveDecl | ID::BitHaveExpr:
        if (decl == node->decl_) {
          node->declState_ = ID::BitHaveExpr | ID::BitHaveDecl;
        } else {
          node->declState_ = ID::BitHaveExpr | ID::BitSideDecl;
          sideIdentifierDeclarationDecl_[node] = (Decl *)node->decl_;
          node->decl_ = decl;
        }
        break;

      // We have an existing "expression decl" and a different "declaration
      // decl" stored in the side table. If the new "expression decl" matches
      // the "declaration decl", then we need to remove the declaration decl"
      // from the side table.
      case ID::BitSideDecl | ID::BitHaveExpr: {
        node->decl_ = decl;
        auto it = sideIdentifierDeclarationDecl_.find(node);
        assert(
            it != sideIdentifierDeclarationDecl_.end() &&
            "IdentifierNode with BitSideDecl must be in side table");
        if (decl == it->second) {
          node->declState_ = ID::BitHaveDecl | ID::BitHaveExpr;
          sideIdentifierDeclarationDecl_.erase(it);
        }
      } break;

      // Just update the value and set the bit.
      default:
        assert(node->declState_ == 0 || node->declState_ == ID::BitHaveExpr);
        node->decl_ = decl;
        node->declState_ = ID::BitHaveExpr;
        break;
    }
  } else {
    // We are "unsetting" an "expression decl". Update the state for that.
    switch (node->declState_) {
      // Unset the existing "expression decl" and clear the pointer.
      case ID::BitHaveExpr:
        node->declState_ = 0;
        node->decl_ = nullptr;
        break;

      // We have both an "expression decl" and a "declaration decl", sharing
      // a value. Clear the "have expression bit".
      case ID::BitHaveExpr | ID::BitHaveDecl:
        node->declState_ = ID::BitHaveDecl;
        break;

      // We have "expression decl" and a "declaration decl" with a different
      // value in a side table. Move the "declaration decl" out of the side
      // table.
      case ID::BitHaveExpr | ID::BitSideDecl: {
        auto it = sideIdentifierDeclarationDecl_.find(node);
        assert(
            it != sideIdentifierDeclarationDecl_.end() &&
            "IdentifierNode with BitSideDecl must be in side table");
        node->decl_ = it->second;
        node->declState_ = ID::BitHaveDecl;
      } break;

      default:
        assert(
            (node->declState_ == 0 || node->declState_ == ID::BitHaveDecl) &&
            "Invalid declState");
        break;
    }
  }
}

void SemContextDumper::printSemContext(
    llvh::raw_ostream &os,
    const SemContext &semCtx) {
  os << "SemContext\n";
  std::map<const FunctionInfo *, llvh::SmallVector<const FunctionInfo *, 2>>
      children;

  for (const auto &F : semCtx.functions_) {
    if (&F == &semCtx.functions_[0])
      continue;
    children[F.parentFunction].push_back(&F);
  }

  unsigned processedCount = 0;
  std::function<void(const FunctionInfo *, unsigned)> dumpFunction =
      [&dumpFunction, &children, &processedCount, &os, this](
          const FunctionInfo *F, unsigned level) {
        printFunction(os, *F, level);
        ++processedCount;
        auto it = children.find(F);
        if (it == children.end())
          return;
        for (auto *childFunc : it->second)
          dumpFunction(childFunc, level + 1);
      };

  dumpFunction(&semCtx.functions_[0], 0);
  assert(
      processedCount == semCtx.functions_.size() &&
      "not all scopes were visited");
}

void SemContextDumper::printFunction(
    llvh::raw_ostream &os,
    const FunctionInfo &f,
    unsigned level) {
  os << ind(level) << "Func " << (f.strict ? "strict" : "loose") << "\n";
  std::map<const LexicalScope *, llvh::SmallVector<const LexicalScope *, 2>>
      children;

  for (const auto *sc : f.scopes) {
    if (sc == f.scopes[0])
      continue;
    children[sc->parentScope].push_back(sc);
  }

  unsigned processedCount = 0;
  std::function<void(const LexicalScope *, unsigned)> dumpScope =
      [&dumpScope, &children, &processedCount, &os, this](
          const LexicalScope *sc, unsigned level) {
        printScope(os, sc, level);
        ++processedCount;
        auto it = children.find(sc);
        if (it == children.end())
          return;
        for (auto *childScope : it->second)
          dumpScope(childScope, level + 1);
      };

  dumpScope(f.scopes[0], level + 1);
  assert(processedCount == f.scopes.size() && "not all scopes were visited");
}

void SemContextDumper::printScope(
    llvh::raw_ostream &os,
    const LexicalScope *s,
    unsigned int level) {
  os << ind(level) << "Scope %s." << scopeNumbers_.getNumber(s) << '\n';
  for (const auto decl : s->decls) {
    os << ind(level + 1);
    printDecl(os, decl);
    os << '\n';
  }
  for (const auto *fd : s->hoistedFunctions) {
    os << ind(level + 1) << "hoistedFunction "
       << llvh::cast<ESTree::IdentifierNode>(fd->_id)->_name->str() << "\n";
  }
}

void SemContextDumper::printScopeRef(
    llvh::raw_ostream &os,
    const LexicalScope *s) {
  os << "Scope %s." << scopeNumbers_.getNumber(s);
}

void SemContextDumper::printDecl(llvh::raw_ostream &os, const Decl *d) {
  os << "Decl %d." << declNumbers_.getNumber(d) << " '" << d->name << "' ";
  const char *s;
#define CASE(x)       \
  case Decl::Kind::x: \
    s = #x;           \
    break;
  switch (d->kind) {
    CASE(Let)
    CASE(Const)
    CASE(Class)
    CASE(Import)
    CASE(ES5Catch)
    CASE(FunctionExprName)
    CASE(ClassExprName)
    CASE(ScopedFunction)
    CASE(Var)
    CASE(Parameter)
    CASE(GlobalProperty)
    CASE(UndeclaredGlobalProperty)
  }
  os << s;
#undef CASE
  if (d->special != Decl::Special::NotSpecial) {
#define CASE(x)          \
  case Decl::Special::x: \
    s = #x;              \
    break;

    switch (d->special) {
      CASE(Arguments)
      CASE(Eval)
      case Decl::Special::NotSpecial:
        break;
    }
    os << ' ' << s;

#undef CASE
  }

  if (annotateDecl_)
    annotateDecl_(os, d);
}

void SemContextDumper::printDeclRef(
    llvh::raw_ostream &os,
    const Decl *d,
    bool printName) {
  os << "%d." << declNumbers_.getNumber(d);
  if (printName && d->name.isValid())
    os << " '" << d->name << '\'';
}

size_t SemContextDumper::PtrNumberingImpl::getNumberImpl(const void *ptr) {
  auto [it, inserted] = numbers_.try_emplace(ptr, nextNumber_);
  if (inserted)
    ++nextNumber_;
  return it->second;
}

} // namespace sema
} // namespace hermes
