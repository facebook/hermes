/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/SemContext.h"

namespace hermes {
namespace sema {

#ifndef NDEBUG
static llvh::FormattedString ind(unsigned level) {
  return llvh::left_justify("", level * 4);
}
#endif

void Decl::dump(unsigned level) const {
#ifndef NDEBUG
  llvh::outs() << ind(level) << "Decl '" << name << "' ";
  const char *s;
#define CASE(x) \
  case Kind::x: \
    s = #x;     \
    break;
  switch (kind) {
    CASE(Let)
    CASE(Const)
    CASE(Class)
    CASE(Import)
    CASE(ES5Catch)
    CASE(FunctionExprName)
    CASE(ScopedFunction)
    CASE(Var)
    CASE(Parameter)
    CASE(GlobalProperty)
    CASE(UndeclaredGlobalProperty)
  }
  llvh::outs() << s;
#undef CASE
#define CASE(x)    \
  case Special::x: \
    s = #x;        \
    break;
  switch (special) {
    CASE(NotSpecial)
    CASE(Arguments)
    CASE(Eval)
  }
#undef CASE
  llvh::outs() << "\n";
#endif
}

void LexicalScope::dump(const SemContext *sd, unsigned int level) const {
#ifndef NDEBUG
  llvh::outs() << ind(level) << "Scope " << llvh::format("%p", this) << "\n";
  for (const auto &decl : decls) {
    decl->dump(level + 1);
  }
  for (const auto *fd : hoistedFunctions) {
    llvh::outs() << ind(level + 1) << "hoistedFunction "
                 << llvh::cast<ESTree::IdentifierNode>(fd->_id)->_name->str()
                 << "\n";
  }
#endif
}

void FunctionInfo::dump(const SemContext *sd, unsigned level) const {
#ifndef NDEBUG
  llvh::outs() << ind(level) << "Func\n";
  std::map<const LexicalScope *, llvh::SmallVector<const LexicalScope *, 2>>
      children;

  for (const auto *sc : scopes) {
    if (sc == scopes[0])
      continue;
    children[sc->parentScope].push_back(sc);
  }

  unsigned processedCount = 0;
  std::function<void(const LexicalScope *, unsigned)> dumpScope =
      [&dumpScope, &children, &processedCount, sd](
          const LexicalScope *sc, unsigned level) {
        sc->dump(sd, level);
        ++processedCount;
        auto it = children.find(sc);
        if (it == children.end())
          return;
        for (auto *childScope : it->second)
          dumpScope(childScope, level + 1);
      };

  dumpScope(scopes[0], level + 1);
  assert(processedCount == scopes.size() && "not all scopes were visited");
#endif
}

void SemContext::dump() const {
#ifndef NDEBUG
  llvh::outs() << "SemContext\n";
  std::map<const FunctionInfo *, llvh::SmallVector<const FunctionInfo *, 2>>
      children;

  for (const auto &F : functions_) {
    if (&F == &functions_[0])
      continue;
    children[F.parentFunction].push_back(&F);
  }

  unsigned processedCount = 0;
  std::function<void(const FunctionInfo *, unsigned)> dumpFunction =
      [&dumpFunction, &children, &processedCount, this](
          const FunctionInfo *F, unsigned level) {
        F->dump(this, level);
        ++processedCount;
        auto it = children.find(F);
        if (it == children.end())
          return;
        for (auto *childFunc : it->second)
          dumpFunction(childFunc, level + 1);
      };

  dumpFunction(&functions_[0], 0);
  assert(processedCount == functions_.size() && "not all scopes were visited");
#endif
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
    bool strict) {
  functions_.emplace_back(funcNode, parentFunction, parentScope, strict);
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

} // namespace sema
} // namespace hermes
