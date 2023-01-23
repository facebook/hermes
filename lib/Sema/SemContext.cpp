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

SemContext::SemContext(Context &ctx) {
  decls_.emplace_back(
      ctx.getIdentifier("eval"),
      Decl::Kind::UndeclaredGlobalProperty,
      Decl::Special::Eval);
  evalDecl_ = &decls_.back();
}

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

void SemContextDumper::printDeclRef(llvh::raw_ostream &os, const Decl *d) {
  os << "%d." << declNumbers_.getNumber(d);
  if (d->name.isValid())
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
