/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSParserImpl.h"

#include "llvh/Support/SaveAndRestore.h"

using llvh::cast;
using llvh::dyn_cast;
using llvh::isa;

namespace hermes {
namespace parser {
namespace detail {

#if HERMES_PARSE_FLOW

Optional<ESTree::Node *> JSParserImpl::parseFlowDeclaration() {
  assert(checkDeclaration());
  SMLoc start = tok_->getStartLoc();

  if (check(TokenKind::rw_enum)) {
    auto optEnum = parseEnumDeclaration();
    if (!optEnum)
      return None;
    return *optEnum;
  }

  TypeAliasKind kind = TypeAliasKind::None;
  if (checkAndEat(declareIdent_))
    kind = TypeAliasKind::Declare;
  else if (checkAndEat(opaqueIdent_))
    kind = TypeAliasKind::Opaque;

  if (kind == TypeAliasKind::Declare &&
      !checkN(typeIdent_, interfaceIdent_, TokenKind::rw_interface)) {
    error(tok_->getSourceRange(), "invalid token in type declaration");
    return None;
  }
  if (kind == TypeAliasKind::Opaque && !check(typeIdent_)) {
    error(tok_->getSourceRange(), "invalid token in opaque type declaration");
    return None;
  }

  if (checkAndEat(typeIdent_)) {
    auto optType = parseTypeAlias(start, kind);
    if (!optType)
      return None;
    return *optType;
  }

  if (checkN(interfaceIdent_, TokenKind::rw_interface)) {
    auto optType = parseInterfaceDeclaration(
        kind == TypeAliasKind::Declare ? Optional<SMLoc>(start) : None);
    if (!optType)
      return None;
    return *optType;
  }

  assert(
      kind == TypeAliasKind::None &&
      "checkDeclaration() returned true without 'type' or 'interface'");
  return None;
}

Optional<ESTree::Node *> JSParserImpl::parseDeclare(
    SMLoc start,
    AllowDeclareExportType allowDeclareExportType) {
  if (checkAndEat(typeIdent_)) {
    return parseTypeAlias(start, TypeAliasKind::Declare);
  }
  if (checkAndEat(opaqueIdent_)) {
    if (!check(typeIdent_)) {
      error(tok_->getStartLoc(), "'type' required in opaque type declaration");
      return None;
    }
    advance(JSLexer::GrammarContext::Flow);
    return parseTypeAlias(start, TypeAliasKind::DeclareOpaque);
  }
  if (checkN(TokenKind::rw_interface, interfaceIdent_)) {
    return parseInterfaceDeclaration(start);
  }
  if (check(TokenKind::rw_class)) {
    return parseDeclareClass(start);
  }
  if (check(TokenKind::rw_function)) {
    return parseDeclareFunction(start);
  }
  if (check(moduleIdent_)) {
    return parseDeclareModule(start);
  }
  if (checkAndEat(TokenKind::rw_var)) {
    auto optIdent = parseBindingIdentifier(Param{});
    if (!optIdent) {
      errorExpected(
          TokenKind::identifier,
          "in var declaration",
          "start of declaration",
          start);
      return None;
    }
    if (!eatSemi())
      return None;
    return setLocation(
        start,
        getPrevTokenEndLoc(),
        new (context_) ESTree::DeclareVariableNode(*optIdent));
  }

  if (!check(TokenKind::rw_export)) {
    errorExpected(
        {TokenKind::rw_export,
         TokenKind::rw_interface,
         TokenKind::rw_function,
         TokenKind::rw_class,
         TokenKind::rw_var},
        "in declared type",
        "start of declare",
        start);
    return None;
  }

  return parseDeclareExport(start, allowDeclareExportType);
}

Optional<ESTree::Node *> JSParserImpl::parseTypeAlias(
    SMLoc start,
    TypeAliasKind kind) {
  if (!need(
          TokenKind::identifier, "in type alias", "start of type alias", start))
    return None;

  ESTree::Node *id = setLocation(
      tok_,
      tok_,
      new (context_)
          ESTree::IdentifierNode(tok_->getIdentifier(), nullptr, false));
  advance(JSLexer::GrammarContext::Flow);

  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optTypeParams = parseTypeParams();
    if (!optTypeParams)
      return None;
    typeParams = *optTypeParams;
  }

  ESTree::Node *supertype = nullptr;
  if ((kind == TypeAliasKind::Opaque || kind == TypeAliasKind::DeclareOpaque) &&
      checkAndEat(TokenKind::colon, JSLexer::GrammarContext::Flow)) {
    auto optSuper = parseTypeAnnotation();
    if (!optSuper)
      return None;
    supertype = *optSuper;
  }

  ESTree::Node *right = nullptr;
  if (kind != TypeAliasKind::DeclareOpaque) {
    if (!eat(
            TokenKind::equal,
            JSLexer::GrammarContext::Flow,
            "in type alias",
            "start of type alias",
            start))
      return None;

    auto optRight = parseTypeAnnotation();
    if (!optRight)
      return None;
    right = *optRight;
  }

  eatSemi(true);

  if (kind == TypeAliasKind::DeclareOpaque) {
    return setLocation(
        start,
        getPrevTokenEndLoc(),
        new (context_)
            ESTree::DeclareOpaqueTypeNode(id, typeParams, right, supertype));
  }
  if (kind == TypeAliasKind::Declare) {
    return setLocation(
        start,
        getPrevTokenEndLoc(),
        new (context_) ESTree::DeclareTypeAliasNode(id, typeParams, right));
  }
  if (kind == TypeAliasKind::Opaque) {
    return setLocation(
        start,
        getPrevTokenEndLoc(),
        new (context_)
            ESTree::OpaqueTypeNode(id, typeParams, right, supertype));
  }
  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::TypeAliasNode(id, typeParams, right));
}

Optional<ESTree::Node *> JSParserImpl::parseInterfaceDeclaration(
    Optional<SMLoc> declareStart) {
  assert(checkN(TokenKind::rw_interface, interfaceIdent_));
  SMLoc start = advance(JSLexer::GrammarContext::Flow).Start;

  if (!need(
          TokenKind::identifier,
          "in interface declaration",
          "start of interface",
          start))
    return None;

  auto *id = setLocation(
      tok_,
      tok_,
      new (context_)
          ESTree::IdentifierNode(tok_->getIdentifier(), nullptr, false));
  advance(JSLexer::GrammarContext::Flow);

  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optParams = parseTypeParams();
    if (!optParams)
      return None;
    typeParams = *optParams;
  }

  ESTree::NodeList extends{};

  auto optBody = parseInterfaceTail(start, extends);
  if (!optBody)
    return None;

  if (declareStart.hasValue()) {
    return setLocation(
        *declareStart,
        *optBody,
        new (context_) ESTree::DeclareInterfaceNode(
            id, typeParams, std::move(extends), *optBody));
  }
  return setLocation(
      start,
      *optBody,
      new (context_) ESTree::InterfaceDeclarationNode(
          id, typeParams, std::move(extends), *optBody));
}

Optional<ESTree::Node *> JSParserImpl::parseInterfaceTail(
    SMLoc start,
    ESTree::NodeList &extends) {
  if (checkAndEat(TokenKind::rw_extends)) {
    do {
      if (!need(
              TokenKind::identifier,
              "in extends clause",
              "location of interface",
              start))
        return None;
      if (!parseInterfaceExtends(start, extends))
        return None;
    } while (checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow));
  }

  if (!need(TokenKind::l_brace, "in interface", "location of interface", start))
    return None;

  return parseObjectTypeAnnotation(
      AllowProtoProperty::No, AllowStaticProperty::No, AllowSpreadProperty::No);
}

bool JSParserImpl::parseInterfaceExtends(
    SMLoc start,
    ESTree::NodeList &extends) {
  assert(check(TokenKind::identifier));
  auto optGeneric = parseGenericType();
  if (!optGeneric)
    return false;
  ESTree::GenericTypeAnnotationNode *generic = *optGeneric;
  extends.push_back(*setLocation(
      generic,
      generic,
      new (context_) ESTree::InterfaceExtendsNode(
          generic->_id, generic->_typeParameters)));
  return true;
}

Optional<ESTree::Node *> JSParserImpl::parseDeclareFunction(SMLoc start) {
  assert(check(TokenKind::rw_function));
  advance(JSLexer::GrammarContext::Flow);

  if (!need(
          TokenKind::identifier,
          "in declare function type",
          "location of declare",
          start))
    return None;

  UniqueString *id = tok_->getIdentifier();
  SMLoc idStart = advance(JSLexer::GrammarContext::Flow).Start;

  SMLoc funcStart = tok_->getStartLoc();

  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optParams = parseTypeParams();
    if (!optParams)
      return None;
    typeParams = *optParams;
  }

  if (!need(
          TokenKind::l_paren,
          "in declare function type",
          "location of declare",
          start))
    return None;

  ESTree::NodeList params{};
  ESTree::Node *thisConstraint = nullptr;
  auto optRest = parseFunctionTypeAnnotationParams(params, thisConstraint);
  if (!optRest)
    return None;

  if (!eat(
          TokenKind::colon,
          JSLexer::GrammarContext::Flow,
          "in declare function type",
          "location of declare",
          start))
    return None;

  auto optReturn = parseTypeAnnotation();
  if (!optReturn)
    return None;
  ESTree::Node *returnType = *optReturn;
  SMLoc funcEnd = getPrevTokenEndLoc();

  ESTree::Node *predicate = nullptr;
  if (check(checksIdent_)) {
    auto optPred = parsePredicate();
    if (!optPred)
      return None;
    predicate = *optPred;
  }

  if (!eatSemi())
    return None;

  auto *func = setLocation(
      funcStart,
      funcEnd,
      new (context_) ESTree::TypeAnnotationNode(setLocation(
          funcStart,
          funcEnd,
          new (context_) ESTree::FunctionTypeAnnotationNode(
              std::move(params),
              thisConstraint,
              returnType,
              *optRest,
              typeParams))));
  auto *ident = setLocation(
      idStart, func, new (context_) ESTree::IdentifierNode(id, func, false));
  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::DeclareFunctionNode(ident, predicate));
}

Optional<ESTree::Node *> JSParserImpl::parseDeclareModule(SMLoc start) {
  assert(check(moduleIdent_));
  advance(JSLexer::GrammarContext::Flow);

  if (checkAndEat(TokenKind::period, JSLexer::GrammarContext::Flow)) {
    if (!checkAndEat(exportsIdent_, JSLexer::GrammarContext::Flow)) {
      error(tok_->getSourceRange(), "expected module.exports declaration");
      return None;
    }

    SMLoc annotStart = tok_->getStartLoc();
    if (!eat(
            TokenKind::colon,
            JSLexer::GrammarContext::Flow,
            "in module.exports declaration",
            "start of declaration",
            start))
      return None;
    auto optType = parseTypeAnnotation(annotStart);
    if (!optType)
      return None;
    eatSemi(true);
    return setLocation(
        start,
        getPrevTokenEndLoc(),
        new (context_) ESTree::DeclareModuleExportsNode(*optType));
  }

  // declare module Identifier {[opt]
  //                ^
  ESTree::Node *id = nullptr;
  if (check(TokenKind::string_literal)) {
    id = setLocation(
        tok_,
        tok_,
        new (context_) ESTree::StringLiteralNode(tok_->getStringLiteral()));
  } else {
    if (!need(
            TokenKind::identifier,
            "in module declaration",
            "start of declaration",
            start))
      return None;
    id = setLocation(
        tok_,
        tok_,
        new (context_)
            ESTree::IdentifierNode(tok_->getIdentifier(), nullptr, false));
  }
  advance(JSLexer::GrammarContext::Flow);

  // declare module Identifier {
  //                           ^
  SMLoc bodyStart = tok_->getStartLoc();
  if (!eat(
          TokenKind::l_brace,
          JSLexer::GrammarContext::Flow,
          "in module declaration",
          "start of declaration",
          start))
    return None;

  UniqueString *kind = nullptr;
  ESTree::NodeList declarations{};

  while (!check(TokenKind::r_brace)) {
    if (check(TokenKind::rw_import)) {
      // 'import' can be bare without a 'declare' before it.
      auto optImport = parseImportDeclaration();
      if (!optImport)
        return None;
      ESTree::ImportDeclarationNode *import = *optImport;
      if (import->_importKind == valueIdent_) {
        error(
            import->getSourceRange(),
            "imports within a `declare module` body must always be "
            "`import type` or `import typeof`");
      }
      declarations.push_back(*import);
      continue;
    }
    if (!check(declareIdent_)) {
      error(
          tok_->getSourceRange(),
          "expected 'declare' in module declaration body");
      return None;
    }
    SMLoc declarationStart = advance(JSLexer::GrammarContext::Flow).Start;
    auto optDecl = parseDeclare(declarationStart, AllowDeclareExportType::Yes);
    if (!optDecl)
      return None;
    ESTree::Node *decl = *optDecl;
    switch (decl->getKind()) {
      case ESTree::NodeKind::DeclareModuleExports:
        if (kind != nullptr && kind != commonJSIdent_) {
          error(
              decl->getSourceRange(),
              "cannot use CommonJS export in ES module");
        }
        kind = commonJSIdent_;
        break;
      case ESTree::NodeKind::DeclareExportDeclaration:
        if (llvh::dyn_cast_or_null<ESTree::InterfaceDeclarationNode>(
                llvh::cast<ESTree::DeclareExportDeclarationNode>(decl)
                    ->_declaration)) {
          // declare export interface can show up in either module kind.
          // Ignore it.
          break;
        }
        if (llvh::dyn_cast_or_null<ESTree::TypeAliasNode>(
                llvh::cast<ESTree::DeclareExportDeclarationNode>(decl)
                    ->_declaration)) {
          // declare export type can show up in either module kind.
          // Ignore it.
          break;
        }
        if (kind != nullptr && kind != esIdent_) {
          error(
              decl->getSourceRange(),
              "cannot use ESM export in CommonJS module");
        }
        kind = esIdent_;
        break;
      case ESTree::NodeKind::DeclareExportAllDeclaration:
        if (kind != nullptr && kind != esIdent_) {
          error(
              decl->getSourceRange(),
              "cannot use ESM export in CommonJS module");
        }
        kind = esIdent_;
        break;
      default:
        break;
    }
    declarations.push_back(*decl);
  }

  SMLoc bodyEnd = advance(JSLexer::GrammarContext::Flow).End;

  ESTree::Node *body = setLocation(
      bodyStart,
      bodyEnd,
      new (context_) ESTree::BlockStatementNode(std::move(declarations)));

  if (kind == nullptr) {
    // Default to CommonJS if we weren't able to figure it out based on
    // declarations themselves.
    kind = commonJSIdent_;
  }

  return setLocation(
      start, body, new (context_) ESTree::DeclareModuleNode(id, body, kind));
}

Optional<ESTree::Node *> JSParserImpl::parseDeclareClass(SMLoc start) {
  assert(check(TokenKind::rw_class));
  advance(JSLexer::GrammarContext::Flow);

  // NOTE: Class definition is always strict mode code.
  SaveStrictMode saveStrictMode{this};
  setStrictMode(true);

  if (!need(
          TokenKind::identifier,
          "in class declaration",
          "start of declaration",
          start))
    return None;

  auto *id = setLocation(
      tok_,
      tok_,
      new (context_)
          ESTree::IdentifierNode(tok_->getIdentifier(), nullptr, false));
  advance(JSLexer::GrammarContext::Flow);

  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optParams = parseTypeParams();
    if (!optParams)
      return None;
    typeParams = *optParams;
  }

  ESTree::NodeList extends{};
  if (checkAndEat(TokenKind::rw_extends)) {
    if (!need(
            TokenKind::identifier,
            "in class 'extends'",
            "start of declaration",
            start))
      return None;
    if (!parseInterfaceExtends(start, extends))
      return None;
  }

  ESTree::NodeList mixins{};
  if (checkAndEat(mixinsIdent_)) {
    do {
      if (!need(
              TokenKind::identifier,
              "in class 'mixins'",
              "start of declaration",
              start))
        return None;
      if (!parseInterfaceExtends(start, mixins))
        return None;
    } while (checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow));
  }

  ESTree::NodeList implements{};
  if (checkAndEat(TokenKind::rw_implements)) {
    do {
      if (!need(
              TokenKind::identifier,
              "in class 'implements'",
              "start of declaration",
              start))
        return None;
      auto optImpl = parseClassImplements();
      if (!optImpl)
        return None;
      implements.push_back(**optImpl);
    } while (checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow));
  }

  if (!need(
          TokenKind::l_brace,
          "in declared class",
          "start of declaration",
          start))
    return None;

  auto optBody = parseObjectTypeAnnotation(
      AllowProtoProperty::Yes,
      AllowStaticProperty::Yes,
      AllowSpreadProperty::No);
  if (!optBody)
    return None;

  return setLocation(
      start,
      *optBody,
      new (context_) ESTree::DeclareClassNode(
          id,
          typeParams,
          std::move(extends),
          std::move(implements),
          std::move(mixins),
          *optBody));
}

Optional<ESTree::Node *> JSParserImpl::parseExportTypeDeclaration(
    SMLoc startLoc) {
  assert(check(typeIdent_));
  SMLoc typeIdentLoc = advance().Start;

  if (checkAndEat(TokenKind::star)) {
    // export type * FromClause;
    //               ^
    auto optFromClause = parseFromClause();
    if (!optFromClause) {
      return None;
    }
    if (!eatSemi()) {
      return None;
    }
    return setLocation(
        startLoc,
        getPrevTokenEndLoc(),
        new (context_)
            ESTree::ExportAllDeclarationNode(*optFromClause, typeIdent_));
  }

  if (check(TokenKind::l_brace)) {
    ESTree::NodeList specifiers{};
    llvh::SmallVector<SMRange, 2> invalids{};

    auto optExportClause = parseExportClause(specifiers, invalids);
    if (!optExportClause) {
      return None;
    }

    ESTree::Node *source = nullptr;
    if (check(fromIdent_)) {
      // export ExportClause FromClause ;
      auto optFromClause = parseFromClause();
      if (!optFromClause) {
        return None;
      }
      source = *optFromClause;
    } else {
      // export ExportClause ;
      // ES9.0 15.2.3.1
      // When there is no FromClause, any ranges added to invalids are
      // actually invalid, and should be reported as errors.
      for (const SMRange &range : invalids) {
        error(range, "Invalid exported name");
      }
    }

    if (!eatSemi()) {
      return None;
    }

    return setLocation(
        startLoc,
        getPrevTokenEndLoc(),
        new (context_) ESTree::ExportNamedDeclarationNode(
            nullptr, std::move(specifiers), source, typeIdent_));
  }

  if (check(TokenKind::identifier)) {
    auto optAlias = parseTypeAlias(typeIdentLoc, TypeAliasKind::None);
    if (!optAlias)
      return None;
    return setLocation(
        startLoc,
        *optAlias,
        new (context_) ESTree::ExportNamedDeclarationNode(
            *optAlias, {}, nullptr, typeIdent_));
  }

  errorExpected(
      {TokenKind::star, TokenKind::l_brace, TokenKind::identifier},
      "in export type declaration",
      "start of export",
      startLoc);
  return None;
}

Optional<ESTree::Node *> JSParserImpl::parseDeclareExport(
    SMLoc start,
    AllowDeclareExportType allowDeclareExportType) {
  assert(check(TokenKind::rw_export));
  advance(JSLexer::GrammarContext::Flow);
  SMLoc declareStart = tok_->getStartLoc();

  if (checkAndEat(TokenKind::rw_default, JSLexer::GrammarContext::Flow)) {
    declareStart = tok_->getStartLoc();
    if (check(TokenKind::rw_function)) {
      auto optFunc = parseDeclareFunction(declareStart);
      if (!optFunc)
        return None;
      return setLocation(
          start,
          *optFunc,
          new (context_) ESTree::DeclareExportDeclarationNode(
              *optFunc, {}, nullptr, true));
    }
    if (check(TokenKind::rw_class)) {
      auto optClass = parseDeclareClass(declareStart);
      if (!optClass)
        return None;
      return setLocation(
          start,
          *optClass,
          new (context_) ESTree::DeclareExportDeclarationNode(
              *optClass, {}, nullptr, true));
    }
    auto optType = parseTypeAnnotation();
    if (!optType)
      return None;
    if (!eatSemi())
      return None;
    return setLocation(
        start,
        getPrevTokenEndLoc(),
        new (context_)
            ESTree::DeclareExportDeclarationNode(*optType, {}, nullptr, true));
  }

  if (check(TokenKind::rw_function)) {
    auto optFunc = parseDeclareFunction(declareStart);
    if (!optFunc)
      return None;
    return setLocation(
        start,
        *optFunc,
        new (context_)
            ESTree::DeclareExportDeclarationNode(*optFunc, {}, nullptr, false));
  }

  if (check(TokenKind::rw_class)) {
    auto optClass = parseDeclareClass(declareStart);
    if (!optClass)
      return None;
    return setLocation(
        start,
        *optClass,
        new (context_) ESTree::DeclareExportDeclarationNode(
            *optClass, {}, nullptr, false));
  }

  if (check(TokenKind::rw_var)) {
    SMLoc varStart = advance(JSLexer::GrammarContext::Flow).Start;
    auto optIdent = parseBindingIdentifier(Param{});
    if (!optIdent) {
      errorExpected(
          TokenKind::identifier,
          "in var declaration",
          "start of declaration",
          start);
      return None;
    }
    if (!eatSemi())
      return None;

    SMLoc end = getPrevTokenEndLoc();
    return setLocation(
        start,
        end,
        new (context_) ESTree::DeclareExportDeclarationNode(
            setLocation(
                varStart,
                end,
                new (context_) ESTree::DeclareVariableNode(*optIdent)),
            {},
            nullptr,
            false));
  }

  if (checkAndEat(opaqueIdent_, JSLexer::GrammarContext::Flow)) {
    if (!check(typeIdent_)) {
      error(tok_->getStartLoc(), "'type' required in opaque type declaration");
      return None;
    }
    advance(JSLexer::GrammarContext::Flow);
    auto optType = parseTypeAlias(declareStart, TypeAliasKind::DeclareOpaque);
    if (!optType)
      return None;
    return setLocation(
        start,
        *optType,
        new (context_)
            ESTree::DeclareExportDeclarationNode(*optType, {}, nullptr, false));
  }

  if (allowDeclareExportType == AllowDeclareExportType::Yes &&
      check(typeIdent_)) {
    advance(JSLexer::GrammarContext::Flow);
    auto optType = parseTypeAlias(declareStart, TypeAliasKind::None);
    if (!optType)
      return None;
    return setLocation(
        start,
        *optType,
        new (context_)
            ESTree::DeclareExportDeclarationNode(*optType, {}, nullptr, false));
  }

  if (checkN(TokenKind::rw_interface, interfaceIdent_)) {
    auto optInterface = parseInterfaceDeclaration();
    if (!optInterface)
      return None;
    return setLocation(
        start,
        *optInterface,
        new (context_) ESTree::DeclareExportDeclarationNode(
            *optInterface, {}, nullptr, false));
  }

  if (checkAndEat(TokenKind::star, JSLexer::GrammarContext::Flow)) {
    // declare export * from 'foo';
    //                  ^
    if (!check(fromIdent_)) {
      error(
          tok_->getStartLoc(), "expected 'from' clause in export declaration");
      return None;
    }
    auto optSource = parseFromClause();
    if (!optSource)
      return None;
    if (!eatSemi())
      return None;
    return setLocation(
        start,
        getPrevTokenEndLoc(),
        new (context_) ESTree::DeclareExportAllDeclarationNode(*optSource));
  }

  if (!need(
          TokenKind::l_brace, "in export specifier", "start of declare", start))
    return None;

  ESTree::NodeList specifiers{};
  llvh::SmallVector<SMRange, 2> invalids{};
  if (!parseExportClause(specifiers, invalids))
    return None;

  ESTree::Node *source = nullptr;
  if (check(fromIdent_)) {
    auto optSource = parseFromClause();
    if (!optSource)
      return None;
    source = *optSource;
  }

  if (!eatSemi())
    return None;

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::DeclareExportDeclarationNode(
          nullptr, std::move(specifiers), source, false));
}

Optional<ESTree::Node *> JSParserImpl::parseTypeAnnotation(
    Optional<SMLoc> wrappedStart,
    AllowAnonFunctionType allowAnonFunctionType) {
  llvh::SaveAndRestore<bool> saveParam(
      allowAnonFunctionType_,
      allowAnonFunctionType == AllowAnonFunctionType::Yes);
  auto optType = parseUnionTypeAnnotation();
  if (!optType)
    return None;
  if (wrappedStart) {
    return setLocation(
        *wrappedStart,
        getPrevTokenEndLoc(),
        new (context_) ESTree::TypeAnnotationNode(*optType));
  }
  return *optType;
}

Optional<ESTree::Node *> JSParserImpl::parseUnionTypeAnnotation() {
  SMLoc start = tok_->getStartLoc();
  checkAndEat(TokenKind::pipe, JSLexer::GrammarContext::Flow);

  auto optFirst = parseIntersectionTypeAnnotation();
  if (!optFirst)
    return None;

  if (!check(TokenKind::pipe)) {
    // Done with the union, move on.
    return *optFirst;
  }

  ESTree::NodeList types{};
  types.push_back(**optFirst);

  while (checkAndEat(TokenKind::pipe, JSLexer::GrammarContext::Flow)) {
    auto optInt = parseIntersectionTypeAnnotation();
    if (!optInt)
      return None;
    types.push_back(**optInt);
  }

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::UnionTypeAnnotationNode(std::move(types)));
}

Optional<ESTree::Node *> JSParserImpl::parseIntersectionTypeAnnotation() {
  SMLoc start = tok_->getStartLoc();
  checkAndEat(TokenKind::amp, JSLexer::GrammarContext::Flow);

  auto optFirst = parseAnonFunctionWithoutParensTypeAnnotation();
  if (!optFirst)
    return None;

  if (!check(TokenKind::amp)) {
    // Done with the union, move on.
    return *optFirst;
  }

  ESTree::NodeList types{};
  types.push_back(**optFirst);

  while (checkAndEat(TokenKind::amp, JSLexer::GrammarContext::Flow)) {
    auto optInt = parseAnonFunctionWithoutParensTypeAnnotation();
    if (!optInt)
      return None;
    types.push_back(**optInt);
  }

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::IntersectionTypeAnnotationNode(std::move(types)));
}

Optional<ESTree::Node *>
JSParserImpl::parseAnonFunctionWithoutParensTypeAnnotation() {
  SMLoc start = tok_->getStartLoc();
  auto optParam = parsePrefixTypeAnnotation();
  if (!optParam)
    return None;

  if (allowAnonFunctionType_ && check(TokenKind::equalgreater)) {
    // ParamType => ReturnType
    //           ^
    ESTree::NodeList params{};
    // "Reparse" the param into a FunctionTypeParam so it can be used for
    // parseFunctionTypeAnnotationWithParams.
    params.push_back(*setLocation(
        *optParam,
        *optParam,
        new (context_) ESTree::FunctionTypeParamNode(
            /* name */ nullptr, *optParam, /* optional */ false)));
    ESTree::Node *rest = nullptr;
    ESTree::Node *typeParams = nullptr;
    return parseFunctionTypeAnnotationWithParams(
        start, std::move(params), nullptr, rest, typeParams);
  }

  return *optParam;
}

Optional<ESTree::Node *> JSParserImpl::parsePrefixTypeAnnotation() {
  SMLoc start = tok_->getStartLoc();
  if (checkAndEat(TokenKind::question, JSLexer::GrammarContext::Flow)) {
    auto optPrefix = parsePrefixTypeAnnotation();
    if (!optPrefix)
      return None;
    return setLocation(
        start,
        getPrevTokenEndLoc(),
        new (context_) ESTree::NullableTypeAnnotationNode(*optPrefix));
  }
  return parsePostfixTypeAnnotation();
}

Optional<ESTree::Node *> JSParserImpl::parsePostfixTypeAnnotation() {
  SMLoc start = tok_->getStartLoc();
  auto optPrimary = parsePrimaryTypeAnnotation();
  if (!optPrimary)
    return None;

  ESTree::Node *result = *optPrimary;

  // Parse any [] after the primary type.
  while (!lexer_.isNewLineBeforeCurrentToken() &&
         checkAndEat(TokenKind::l_square, JSLexer::GrammarContext::Flow)) {
    if (!need(TokenKind::r_square, "in array type annotation", nullptr, {}))
      return None;
    result = setLocation(
        start,
        advance(JSLexer::GrammarContext::Flow).End,
        new (context_) ESTree::ArrayTypeAnnotationNode(result));
  }

  return result;
}

Optional<ESTree::Node *> JSParserImpl::parsePrimaryTypeAnnotation() {
  SMLoc start = tok_->getStartLoc();
  switch (tok_->getKind()) {
    case TokenKind::star:
      return setLocation(
          start,
          advance(JSLexer::GrammarContext::Flow).End,
          new (context_) ESTree::ExistsTypeAnnotationNode());
    case TokenKind::less:
      return parseFunctionTypeAnnotation();
    case TokenKind::l_paren:
      return parseFunctionOrGroupTypeAnnotation();
    case TokenKind::l_brace:
    case TokenKind::l_bracepipe:
      return parseObjectTypeAnnotation(
          AllowProtoProperty::No,
          AllowStaticProperty::No,
          AllowSpreadProperty::Yes);
    case TokenKind::rw_interface: {
      advance(JSLexer::GrammarContext::Flow);
      ESTree::NodeList extends{};
      auto optBody = parseInterfaceTail(start, extends);
      if (!optBody)
        return None;
      return setLocation(
          start,
          *optBody,
          new (context_) ESTree::InterfaceTypeAnnotationNode(
              std::move(extends), *optBody));
    }
    case TokenKind::rw_typeof: {
      advance(JSLexer::GrammarContext::Flow);
      auto optPrimary = parsePrimaryTypeAnnotation();
      if (!optPrimary)
        return None;
      return setLocation(
          start,
          getPrevTokenEndLoc(),
          new (context_) ESTree::TypeofTypeAnnotationNode(*optPrimary));
    }

    case TokenKind::l_square:
      return parseTupleTypeAnnotation();
    case TokenKind::rw_static:
    case TokenKind::rw_this:
    case TokenKind::identifier:
      if (tok_->getResWordOrIdentifier() == anyIdent_) {
        return setLocation(
            start,
            advance(JSLexer::GrammarContext::Flow).End,
            new (context_) ESTree::AnyTypeAnnotationNode());
      }
      if (tok_->getResWordOrIdentifier() == mixedIdent_) {
        return setLocation(
            start,
            advance(JSLexer::GrammarContext::Flow).End,
            new (context_) ESTree::MixedTypeAnnotationNode());
      }
      if (tok_->getResWordOrIdentifier() == emptyIdent_) {
        return setLocation(
            start,
            advance(JSLexer::GrammarContext::Flow).End,
            new (context_) ESTree::EmptyTypeAnnotationNode());
      }
      if (tok_->getResWordOrIdentifier() == booleanIdent_ ||
          tok_->getResWordOrIdentifier() == boolIdent_) {
        return setLocation(
            start,
            advance(JSLexer::GrammarContext::Flow).End,
            new (context_) ESTree::BooleanTypeAnnotationNode());
      }
      if (tok_->getResWordOrIdentifier() == numberIdent_) {
        return setLocation(
            start,
            advance(JSLexer::GrammarContext::Flow).End,
            new (context_) ESTree::NumberTypeAnnotationNode());
      }
      if (tok_->getResWordOrIdentifier() == symbolIdent_) {
        return setLocation(
            start,
            advance(JSLexer::GrammarContext::Flow).End,
            new (context_) ESTree::SymbolTypeAnnotationNode());
      }
      if (tok_->getResWordOrIdentifier() == stringIdent_) {
        return setLocation(
            start,
            advance(JSLexer::GrammarContext::Flow).End,
            new (context_) ESTree::StringTypeAnnotationNode());
      }
      if (tok_->getResWordOrIdentifier() == interfaceIdent_) {
        advance(JSLexer::GrammarContext::Flow);
        ESTree::NodeList extends{};
        auto optBody = parseInterfaceTail(start, extends);
        if (!optBody)
          return None;
        return setLocation(
            start,
            *optBody,
            new (context_) ESTree::InterfaceTypeAnnotationNode(
                std::move(extends), *optBody));
      }

      {
        auto optGeneric = parseGenericType();
        if (!optGeneric)
          return None;
        return *optGeneric;
      }

    case TokenKind::rw_null:
      return setLocation(
          start,
          advance(JSLexer::GrammarContext::Flow).End,
          new (context_) ESTree::NullLiteralTypeAnnotationNode());

    case TokenKind::rw_void:
      return setLocation(
          start,
          advance(JSLexer::GrammarContext::Flow).End,
          new (context_) ESTree::VoidTypeAnnotationNode());

    case TokenKind::string_literal: {
      UniqueString *str = tok_->getStringLiteral();
      return setLocation(
          start,
          advance(JSLexer::GrammarContext::Flow).End,
          new (context_) ESTree::StringLiteralTypeAnnotationNode(str));
    }

    case TokenKind::numeric_literal: {
      double value = tok_->getNumericLiteral();
      UniqueString *raw = lexer_.getStringLiteral(tok_->inputStr());
      return setLocation(
          start,
          advance(JSLexer::GrammarContext::Flow).End,
          new (context_) ESTree::NumberLiteralTypeAnnotationNode(value, raw));
    }

    case TokenKind::minus: {
      advance(JSLexer::GrammarContext::Flow);
      if (!need(
              TokenKind::numeric_literal,
              "in type annotation",
              "start of annotation",
              start))
        return None;
      // Negate the literal.
      double value = -tok_->getNumericLiteral();
      UniqueString *raw = lexer_.getStringLiteral(StringRef(
          start.getPointer(),
          tok_->getEndLoc().getPointer() - start.getPointer()));
      return setLocation(
          start,
          advance(JSLexer::GrammarContext::Flow).End,
          new (context_) ESTree::NumberLiteralTypeAnnotationNode(value, raw));
    }

    case TokenKind::rw_true:
    case TokenKind::rw_false: {
      bool value = check(TokenKind::rw_true);
      auto *raw = tok_->getResWordIdentifier();
      return setLocation(
          start,
          advance(JSLexer::GrammarContext::Flow).End,
          new (context_) ESTree::BooleanLiteralTypeAnnotationNode(value, raw));
    }
    default:
      if (tok_->isResWord()) {
        auto optGeneric = parseGenericType();
        if (!optGeneric)
          return None;
        return *optGeneric;
      }
      error(tok_->getStartLoc(), "unexpected token in type annotation");
      return None;
  }
}

Optional<ESTree::Node *> JSParserImpl::parseTupleTypeAnnotation() {
  assert(check(TokenKind::l_square));
  SMLoc start = advance(JSLexer::GrammarContext::Flow).Start;

  ESTree::NodeList types{};

  while (!check(TokenKind::r_square)) {
    auto optType = parseTypeAnnotation();
    if (!optType)
      return None;
    types.push_back(**optType);

    if (!checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow))
      break;
  }

  if (!need(
          TokenKind::r_square,
          "at end of tuple type annotation",
          "start of tuple",
          start))
    return None;

  return setLocation(
      start,
      advance(JSLexer::GrammarContext::Flow).End,
      new (context_) ESTree::TupleTypeAnnotationNode(std::move(types)));
}

Optional<ESTree::Node *> JSParserImpl::parseFunctionTypeAnnotation() {
  SMLoc start = tok_->getStartLoc();

  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optParams = parseTypeParams();
    if (!optParams)
      return None;
    typeParams = *optParams;
  }

  if (!need(
          TokenKind::l_paren,
          "in function type annotation",
          "start of annotation",
          start))
    return None;

  ESTree::NodeList params{};
  ESTree::Node *thisConstraint = nullptr;
  auto optRest = parseFunctionTypeAnnotationParams(params, thisConstraint);
  if (!optRest)
    return None;
  ESTree::Node *rest = *optRest;

  if (!need(
          TokenKind::equalgreater,
          "in function type annotation",
          "start of annotation",
          start))
    return None;

  return parseFunctionTypeAnnotationWithParams(
      start, std::move(params), thisConstraint, rest, typeParams);
}

Optional<ESTree::Node *> JSParserImpl::parseFunctionTypeAnnotationWithParams(
    SMLoc start,
    ESTree::NodeList &&params,
    ESTree::Node *thisConstraint,
    ESTree::Node *rest,
    ESTree::Node *typeParams) {
  assert(check(TokenKind::equalgreater));
  advance(JSLexer::GrammarContext::Flow);

  auto optReturnType = parseTypeAnnotation();
  if (!optReturnType)
    return None;

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::FunctionTypeAnnotationNode(
          std::move(params), thisConstraint, *optReturnType, rest, typeParams));
}

Optional<ESTree::Node *> JSParserImpl::parseFunctionOrGroupTypeAnnotation() {
  assert(check(TokenKind::l_paren));
  // This is either
  // ( Type )
  // ^
  // or
  // ( ParamList ) => Type
  // ^
  // so we use a similar approach to arrow function parameters by keeping track
  // and reparsing in certain cases.
  SMLoc start = advance(JSLexer::GrammarContext::Flow).Start;

  bool isFunction = false;
  ESTree::Node *type = nullptr;
  ESTree::Node *rest = nullptr;
  ESTree::NodeList params{};
  ESTree::Node *thisConstraint = nullptr;

  if (check(TokenKind::rw_this)) {
    isFunction = true;
    SMLoc thisStart = advance(JSLexer::GrammarContext::Flow).Start;
    if (checkAndEat(TokenKind::colon, JSLexer::GrammarContext::Flow)) {
      auto optType = parseTypeAnnotation();
      if (!optType)
        return None;
      ESTree::Node *typeAnnotation = *optType;

      thisConstraint = setLocation(
          thisStart,
          getPrevTokenEndLoc(),
          new (context_) ESTree::FunctionTypeParamNode(
              /* name */ nullptr, typeAnnotation, /* optional */ false));
    } else {
      params.push_back(*setLocation(
          thisStart,
          getPrevTokenEndLoc(),
          new (context_) ESTree::FunctionTypeParamNode(
              nullptr,
              setLocation(
                  thisStart,
                  getPrevTokenEndLoc(),
                  new (context_) ESTree::GenericTypeAnnotationNode(
                      setLocation(
                          thisStart,
                          getPrevTokenEndLoc(),
                          new (context_) ESTree::IdentifierNode(
                              thisIdent_, nullptr, false)),
                      {})),
              false)));
    }
    checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow);
  }

  if (allowAnonFunctionType_ &&
      checkAndEat(TokenKind::dotdotdot, JSLexer::GrammarContext::Flow)) {
    isFunction = true;
    // Must be parameters, and this must be the last one.
    auto optParam = parseFunctionTypeAnnotationParam();
    if (!optParam)
      return None;
    // Rest param must be the last param.
    rest = *optParam;
  } else if (check(TokenKind::r_paren)) {
    isFunction = true;
    // ( )
    //   ^
    // No parameters, but this must be an empty param list.
  } else {
    // Not sure yet whether this is a param or simply a type.
    auto optParam = parseFunctionTypeAnnotationParam();
    if (!optParam)
      return None;
    ESTree::FunctionTypeParamNode *param = *optParam;
    type = param->_typeAnnotation;
    if (param->_name || param->_optional) {
      // Must be a param if it has a name.or if it was optional.
      isFunction = true;
    }
    params.push_back(*param);
  }

  // If isFunction was already forced by something previously then we
  // have no choice but to attempt to parse as a function type annotation.
  if ((isFunction || allowAnonFunctionType_) &&
      checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow)) {
    isFunction = true;
    while (!check(TokenKind::r_paren)) {
      bool isRest = !rest &&
          checkAndEat(TokenKind::dotdotdot, JSLexer::GrammarContext::Flow);

      auto optParam = parseFunctionTypeAnnotationParam();
      if (!optParam)
        return None;
      if (isRest) {
        rest = *optParam;
        checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow);
        break;
      } else {
        params.push_back(**optParam);
      }

      if (!checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow))
        break;
    }
  }

  if (!eat(
          TokenKind::r_paren,
          JSLexer::GrammarContext::Flow,
          "at end of function annotation parameters",
          "start of parameters",
          start))
    return None;

  if (isFunction) {
    if (!eat(
            TokenKind::equalgreater,
            JSLexer::GrammarContext::Flow,
            "in function type annotation",
            "start of function",
            start))
      return None;
  } else if (allowAnonFunctionType_) {
    if (checkAndEat(TokenKind::equalgreater, JSLexer::GrammarContext::Flow)) {
      isFunction = true;
    }
  }

  if (!isFunction) {
    return type;
  }

  auto optReturnType = parseTypeAnnotation(
      None,
      allowAnonFunctionType_ ? AllowAnonFunctionType::Yes
                             : AllowAnonFunctionType::No);
  if (!optReturnType)
    return None;

  ESTree::Node *typeParams = nullptr;
  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::FunctionTypeAnnotationNode(
          std::move(params), thisConstraint, *optReturnType, rest, typeParams));
}

Optional<ESTree::Node *> JSParserImpl::parseObjectTypeAnnotation(
    AllowProtoProperty allowProtoProperty,
    AllowStaticProperty allowStaticProperty,
    AllowSpreadProperty allowSpreadProperty) {
  assert(check(TokenKind::l_brace, TokenKind::l_bracepipe));
  bool exact = check(TokenKind::l_bracepipe);
  SMLoc start = advance(JSLexer::GrammarContext::Flow).Start;

  ESTree::NodeList properties{};
  ESTree::NodeList indexers{};
  ESTree::NodeList callProperties{};
  ESTree::NodeList internalSlots{};
  bool inexact = false;

  if (!parseObjectTypeProperties(
          allowProtoProperty,
          allowStaticProperty,
          allowSpreadProperty,
          properties,
          indexers,
          callProperties,
          internalSlots,
          inexact))
    return None;

  if (exact && inexact) {
    // Doesn't prevent parsing from continuing, but it is an error.
    error(
        start,
        "Explicit inexact syntax cannot appear inside an explicit exact object type");
  }

  SMLoc end = tok_->getEndLoc();
  if (!eat(
          exact ? TokenKind::piper_brace : TokenKind::r_brace,
          JSLexer::GrammarContext::Flow,
          "at end of exact object type annotation",
          "start of object",
          start))
    return None;

  return setLocation(
      start,
      end,
      new (context_) ESTree::ObjectTypeAnnotationNode(
          std::move(properties),
          std::move(indexers),
          std::move(callProperties),
          std::move(internalSlots),
          inexact,
          exact));
}

bool JSParserImpl::parseObjectTypeProperties(
    AllowProtoProperty allowProtoProperty,
    AllowStaticProperty allowStaticProperty,
    AllowSpreadProperty allowSpreadProperty,
    ESTree::NodeList &properties,
    ESTree::NodeList &indexers,
    ESTree::NodeList &callProperties,
    ESTree::NodeList &internalSlots,
    bool &inexact) {
  while (!check(TokenKind::r_brace, TokenKind::piper_brace)) {
    SMLoc start = tok_->getStartLoc();
    if (check(TokenKind::dotdotdot)) {
      // Spread property or explicit '...' for inexact.
      advance(JSLexer::GrammarContext::Flow);
      if (check(TokenKind::comma, TokenKind::semi)) {
        inexact = true;
        advance(JSLexer::GrammarContext::Flow);
        // Explicit '...' must be the last element in the type annotation.
        return true;
      } else if (check(TokenKind::r_brace, TokenKind::piper_brace)) {
        inexact = true;
        return true;
      } else {
        if (allowSpreadProperty == AllowSpreadProperty::No) {
          error(
              start, "Spreading a type is only allowed inside an object type");
        }
        auto optType = parseTypeAnnotation();
        if (!optType)
          return false;
        properties.push_back(*setLocation(
            start,
            getPrevTokenEndLoc(),
            new (context_) ESTree::ObjectTypeSpreadPropertyNode(*optType)));
      }
    } else {
      if (!parsePropertyTypeAnnotation(
              allowProtoProperty,
              allowStaticProperty,
              properties,
              indexers,
              callProperties,
              internalSlots))
        return false;
    }

    if (check(TokenKind::comma, TokenKind::semi)) {
      advance(JSLexer::GrammarContext::Flow);
    } else if (check(TokenKind::r_brace, TokenKind::piper_brace)) {
      return true;
    } else {
      errorExpected(
          {TokenKind::comma,
           TokenKind::semi,
           TokenKind::r_brace,
           TokenKind::piper_brace},
          "after property",
          "start of property",
          start);
      return false;
    }
  }

  return true;
}

bool JSParserImpl::parsePropertyTypeAnnotation(
    AllowProtoProperty allowProtoProperty,
    AllowStaticProperty allowStaticProperty,
    ESTree::NodeList &properties,
    ESTree::NodeList &indexers,
    ESTree::NodeList &callProperties,
    ESTree::NodeList &internalSlots) {
  SMRange startRange = tok_->getSourceRange();
  SMLoc start = startRange.Start;

  ESTree::Node *variance = nullptr;
  bool isStatic = false;
  bool proto = false;

  if (check(protoIdent_)) {
    proto = true;
    advance(JSLexer::GrammarContext::Flow);
  }

  if (!proto && (check(TokenKind::rw_static) || check(staticIdent_))) {
    isStatic = true;
    advance(JSLexer::GrammarContext::Flow);
  }

  if (check(TokenKind::plus, TokenKind::minus)) {
    variance = setLocation(
        tok_,
        tok_,
        new (context_) ESTree::VarianceNode(
            check(TokenKind::plus) ? plusIdent_ : minusIdent_));
    advance(JSLexer::GrammarContext::Flow);
  }

  if (checkAndEat(TokenKind::l_square, JSLexer::GrammarContext::Flow)) {
    if (checkAndEat(TokenKind::l_square, JSLexer::GrammarContext::Flow)) {
      if (variance) {
        error(variance->getSourceRange(), "Unexpected variance sigil");
      }
      if (proto) {
        error(startRange, "invalid 'proto' modifier");
      }
      if (isStatic && allowStaticProperty == AllowStaticProperty::No) {
        error(startRange, "invalid 'static' modifier");
      }
      // Internal slot
      if (!check(TokenKind::identifier) && !tok_->isResWord()) {
        errorExpected(
            TokenKind::identifier,
            "in internal slot",
            "start of internal slot",
            start);
        return false;
      }
      ESTree::IdentifierNode *id = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::IdentifierNode(
              tok_->getResWordOrIdentifier(), nullptr, false));
      advance(JSLexer::GrammarContext::Flow);

      if (!eat(
              TokenKind::r_square,
              JSLexer::GrammarContext::Flow,
              "at end of internal slot",
              "start of internal slot",
              start))
        return false;
      if (!eat(
              TokenKind::r_square,
              JSLexer::GrammarContext::Flow,
              "at end of internal slot",
              "start of internal slot",
              start))
        return false;

      bool optional = false;
      bool method = false;
      ESTree::Node *value = nullptr;

      if (check(TokenKind::less, TokenKind::l_paren)) {
        // Type params and method.
        method = true;
        ESTree::Node *typeParams = nullptr;
        if (check(TokenKind::less)) {
          auto optParams = parseTypeParams();
          if (!optParams)
            return false;
          typeParams = *optParams;
        }
        auto optMethodish = parseMethodishTypeAnnotation(start, typeParams);
        if (!optMethodish)
          return false;
        value = *optMethodish;
      } else {
        // Standard type annotation.
        optional =
            checkAndEat(TokenKind::question, JSLexer::GrammarContext::Flow);
        if (!eat(
                TokenKind::colon,
                JSLexer::GrammarContext::Flow,
                "in type annotation",
                "start of annotation",
                start))
          return false;
        auto optValue = parseTypeAnnotation();
        if (!optValue)
          return false;
        value = *optValue;
      }

      assert(value && "value must be set by a branch");
      internalSlots.push_back(*setLocation(
          start,
          getPrevTokenEndLoc(),
          new (context_) ESTree::ObjectTypeInternalSlotNode(
              id, value, optional, isStatic, method)));
    } else {
      // Indexer
      auto optIndexer = parseTypeIndexerProperty(start, variance, isStatic);
      if (!optIndexer)
        return false;
      if (proto) {
        error(startRange, "invalid 'proto' modifier");
      }
      if (isStatic && allowStaticProperty == AllowStaticProperty::No) {
        error(startRange, "invalid 'static' modifier");
      }
      indexers.push_back(**optIndexer);
    }
    return true;
  }

  ESTree::Node *key = nullptr;

  if (check(TokenKind::less, TokenKind::l_paren)) {
    if ((isStatic && allowStaticProperty == AllowStaticProperty::No) ||
        (proto && allowProtoProperty == AllowProtoProperty::No)) {
      key = setLocation(
          startRange,
          startRange,
          new (context_) ESTree::IdentifierNode(
              isStatic ? staticIdent_ : protoIdent_, nullptr, false));
      isStatic = false;
      proto = false;
      if (variance) {
        error(variance->getSourceRange(), "Unexpected variance sigil");
      }
      auto optProp = parseMethodTypeProperty(start, isStatic, key);
      if (!optProp)
        return false;
      properties.push_back(**optProp);
      return true;
    }
    if (variance != nullptr) {
      error(
          variance->getSourceRange(),
          "call property must not specify variance");
    }
    if (proto) {
      error(startRange, "invalid 'proto' modifier");
    }
    auto optCall = parseTypeCallProperty(start, isStatic);
    if (!optCall)
      return false;
    callProperties.push_back(**optCall);
    return true;
  }

  if ((isStatic || proto) && check(TokenKind::colon, TokenKind::question)) {
    if (variance) {
      error(variance->getSourceRange(), "Unexpected variance sigil");
    }
    key = setLocation(
        startRange,
        startRange,
        new (context_) ESTree::IdentifierNode(
            isStatic ? staticIdent_ : protoIdent_, nullptr, false));
    isStatic = false;
    proto = false;
    auto optProp = parseTypeProperty(start, variance, isStatic, proto, key);
    if (!optProp)
      return false;
    properties.push_back(**optProp);
    return true;
  }

  auto optKey = parsePropertyName();
  if (!optKey)
    return false;
  key = *optKey;

  if (check(TokenKind::less, TokenKind::l_paren)) {
    if (variance) {
      error(variance->getSourceRange(), "Unexpected variance sigil");
    }
    if (proto) {
      error(startRange, "invalid 'proto' modifier");
    }
    if (isStatic && allowStaticProperty == AllowStaticProperty::No) {
      error(startRange, "invalid 'static' modifier");
    }
    auto optProp = parseMethodTypeProperty(start, isStatic, key);
    if (!optProp)
      return false;
    properties.push_back(**optProp);
    return true;
  }

  if (check(TokenKind::colon, TokenKind::question)) {
    if (proto && allowProtoProperty == AllowProtoProperty::No) {
      error(startRange, "invalid 'proto' modifier");
    }
    if (isStatic && allowStaticProperty == AllowStaticProperty::No) {
      error(startRange, "invalid 'static' modifier");
    }
    auto optProp = parseTypeProperty(start, variance, isStatic, proto, key);
    if (!optProp)
      return false;
    properties.push_back(**optProp);
    return true;
  }

  if (auto *ident = dyn_cast<ESTree::IdentifierNode>(key)) {
    if (ident->_name == getIdent_ || ident->_name == setIdent_) {
      if (variance != nullptr) {
        error(
            variance->getSourceRange(),
            "accessor property must not specify variance");
      }
      if (proto) {
        error(startRange, "invalid 'proto' modifier");
      }
      if (isStatic && allowStaticProperty == AllowStaticProperty::No) {
        error(startRange, "invalid 'static' modifier");
      }
      auto optKey = parsePropertyName();
      if (!optKey)
        return false;
      key = *optKey;
      auto optGetSet = parseGetOrSetTypeProperty(
          start, isStatic, ident->_name == getIdent_, key);
      if (!optGetSet)
        return false;
      properties.push_back(**optGetSet);
      return true;
    }
  }

  errorExpected(
      {TokenKind::colon, TokenKind::question},
      "in property type annotation",
      "start of properties",
      start);
  return false;
}

Optional<ESTree::Node *> JSParserImpl::parseTypeProperty(
    SMLoc start,
    ESTree::Node *variance,
    bool isStatic,
    bool proto,
    ESTree::Node *key) {
  assert(check(TokenKind::colon, TokenKind::question));

  bool optional =
      checkAndEat(TokenKind::question, JSLexer::GrammarContext::Flow);
  if (!eat(
          TokenKind::colon,
          JSLexer::GrammarContext::Flow,
          "in type property",
          "start of property",
          start))
    return None;

  auto optValue = parseTypeAnnotation();
  if (!optValue)
    return None;
  ESTree::Node *value = *optValue;

  bool method = false;
  UniqueString *kind = initIdent_;

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::ObjectTypePropertyNode(
          key, value, method, optional, isStatic, proto, variance, kind));
}

Optional<ESTree::Node *> JSParserImpl::parseMethodTypeProperty(
    SMLoc start,
    bool isStatic,
    ESTree::Node *key) {
  assert(check(TokenKind::less, TokenKind::l_paren));

  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optTypeParams = parseTypeParams();
    if (!optTypeParams)
      return None;
    typeParams = *optTypeParams;
  }

  auto optValue = parseMethodishTypeAnnotation(start, typeParams);
  if (!optValue)
    return None;
  ESTree::Node *value = *optValue;

  bool method = true;
  bool optional = false;
  bool proto = false;
  UniqueString *kind = initIdent_;

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::ObjectTypePropertyNode(
          key,
          value,
          method,
          optional,
          isStatic,
          proto,
          /* variance */ nullptr,
          kind));
}

Optional<ESTree::Node *> JSParserImpl::parseGetOrSetTypeProperty(
    SMLoc start,
    bool isStatic,
    bool isGetter,
    ESTree::Node *key) {
  auto optValue = parseMethodishTypeAnnotation(start, nullptr);
  if (!optValue)
    return None;

  ESTree::FunctionTypeAnnotationNode *value = *optValue;
  bool method = false;
  bool optional = false;
  bool proto = false;
  ESTree::Node *variance = nullptr;
  UniqueString *kind = isGetter ? getIdent_ : setIdent_;

  // Check the number of parameters, but we can continue parsing anyway.
  if (isGetter) {
    if (value->_params.size() != 0) {
      error(value->getSourceRange(), "Getter must have 0 parameters");
    }
  } else {
    if (value->_params.size() != 1) {
      error(value->getSourceRange(), "Setter must have 1 parameter");
    }
  }

  if (value->_this) {
    error(
        value->_this->getSourceRange(),
        "Accessors must not have 'this' annotations");
  }

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::ObjectTypePropertyNode(
          key, value, method, optional, isStatic, proto, variance, kind));
}

Optional<ESTree::Node *> JSParserImpl::parseTypeIndexerProperty(
    SMLoc start,
    ESTree::Node *variance,
    bool isStatic) {
  // We can either have
  // [ Identifier : TypeAnnotation ]
  //   ^
  // or
  // [ TypeAnnotation ]
  //   ^
  // Because we cannot differentiate without looking ahead for the `:`,
  // we call `parseTypeAnnotation`, check if we have the `:`, and then
  // pull the Identifier out of the GenericTypeAnnotation which should
  // have been emitted, and run it again.
  auto optLeft = parseTypeAnnotation();
  if (!optLeft)
    return None;

  ESTree::IdentifierNode *id = nullptr;
  ESTree::Node *key = nullptr;

  if (checkAndEat(TokenKind::colon, JSLexer::GrammarContext::Flow)) {
    auto optId = reparseTypeAnnotationAsIdentifier(*optLeft);
    if (!optId)
      return None;
    id = *optId;
    auto optKey = parseTypeAnnotation();
    if (!optKey)
      return None;
    key = *optKey;
  } else {
    key = *optLeft;
  }

  if (!eat(
          TokenKind::r_square,
          JSLexer::GrammarContext::Flow,
          "in indexer",
          "start of indexer",
          start))
    return None;

  if (!eat(
          TokenKind::colon,
          JSLexer::GrammarContext::Flow,
          "in indexer",
          "start of indexer",
          start))
    return None;

  auto optValue = parseTypeAnnotation();
  if (!optValue)
    return None;
  ESTree::Node *value = *optValue;

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_)
          ESTree::ObjectTypeIndexerNode(id, key, value, isStatic, variance));
}

Optional<ESTree::Node *> JSParserImpl::parseTypeCallProperty(
    SMLoc start,
    bool isStatic) {
  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optTypeParams = parseTypeParams();
    if (!optTypeParams)
      return None;
    typeParams = *optTypeParams;
  }
  auto optValue = parseMethodishTypeAnnotation(start, typeParams);
  if (!optValue)
    return None;
  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::ObjectTypeCallPropertyNode(*optValue, isStatic));
}

Optional<ESTree::Node *> JSParserImpl::parseTypeParams() {
  assert(check(TokenKind::less));
  SMLoc start = advance(JSLexer::GrammarContext::Flow).Start;

  ESTree::NodeList params{};

  do {
    auto optType = parseTypeParam();
    if (!optType)
      return None;
    params.push_back(**optType);

    if (!checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow))
      break;
  } while (!check(TokenKind::greater));

  SMLoc end = tok_->getEndLoc();
  if (!eat(
          TokenKind::greater,
          JSLexer::GrammarContext::Flow,
          "at end of type parameters",
          "start of type parameters",
          start))
    return None;

  return setLocation(
      start,
      end,
      new (context_) ESTree::TypeParameterDeclarationNode(std::move(params)));
}

Optional<ESTree::Node *> JSParserImpl::parseTypeParam() {
  SMLoc start = tok_->getStartLoc();
  ESTree::Node *variance = nullptr;

  if (check(TokenKind::plus, TokenKind::minus)) {
    variance = setLocation(
        tok_,
        tok_,
        new (context_) ESTree::VarianceNode(
            check(TokenKind::plus) ? plusIdent_ : minusIdent_));
    advance(JSLexer::GrammarContext::Flow);
  }

  if (!need(TokenKind::identifier, "in type parameter", nullptr, {}))
    return None;
  UniqueString *name = tok_->getIdentifier();
  advance(JSLexer::GrammarContext::Flow);

  ESTree::Node *bound = nullptr;
  if (check(TokenKind::colon)) {
    SMLoc boundStart = advance(JSLexer::GrammarContext::Flow).Start;
    auto optType = parseTypeAnnotation();
    if (!optType)
      return None;
    bound = setLocation(
        boundStart,
        getPrevTokenEndLoc(),
        new (context_) ESTree::TypeAnnotationNode(*optType));
  }

  ESTree::Node *initializer = nullptr;
  if (checkAndEat(TokenKind::equal, JSLexer::GrammarContext::Flow)) {
    auto optInit = parseTypeAnnotation();
    if (!optInit)
      return None;
    initializer = *optInit;
  }

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_)
          ESTree::TypeParameterNode(name, bound, variance, initializer));
}

Optional<ESTree::Node *> JSParserImpl::parseTypeArgs() {
  assert(check(TokenKind::less));
  SMLoc start = advance(JSLexer::GrammarContext::Flow).Start;

  ESTree::NodeList params{};

  while (!check(TokenKind::greater)) {
    auto optType = parseTypeAnnotation();
    if (!optType)
      return None;
    params.push_back(**optType);

    if (!checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow))
      break;
  }

  SMLoc end = tok_->getEndLoc();
  if (!eat(
          TokenKind::greater,
          JSLexer::GrammarContext::Flow,
          "at end of type parameters",
          "start of type parameters",
          start))
    return None;

  return setLocation(
      start,
      end,
      new (context_) ESTree::TypeParameterInstantiationNode(std::move(params)));
}

Optional<ESTree::FunctionTypeAnnotationNode *>
JSParserImpl::parseMethodishTypeAnnotation(
    SMLoc start,
    ESTree::Node *typeParams) {
  ESTree::NodeList params{};
  ESTree::Node *thisConstraint = nullptr;

  if (!need(TokenKind::l_paren, "at start of parameters", nullptr, {}))
    return None;
  auto optRest = parseFunctionTypeAnnotationParams(params, thisConstraint);
  if (!optRest)
    return None;

  if (!eat(
          TokenKind::colon,
          JSLexer::GrammarContext::Flow,
          "in function type annotation",
          "start of annotation",
          start))
    return None;

  auto optReturn = parseTypeAnnotation();
  if (!optReturn)
    return None;

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::FunctionTypeAnnotationNode(
          std::move(params), thisConstraint, *optReturn, *optRest, typeParams));
}

Optional<ESTree::FunctionTypeParamNode *>
JSParserImpl::parseFunctionTypeAnnotationParams(
    ESTree::NodeList &params,
    ESTree::NodePtr &thisConstraint) {
  assert(check(TokenKind::l_paren));
  SMLoc start = advance(JSLexer::GrammarContext::Flow).Start;

  ESTree::FunctionTypeParamNode *rest = nullptr;
  thisConstraint = nullptr;

  if (check(TokenKind::rw_this)) {
    SMLoc thisStart = advance(JSLexer::GrammarContext::Flow).Start;
    if (checkAndEat(TokenKind::colon, JSLexer::GrammarContext::Flow)) {
      auto optType = parseTypeAnnotation();
      if (!optType)
        return None;
      ESTree::Node *typeAnnotation = *optType;

      thisConstraint = setLocation(
          thisStart,
          getPrevTokenEndLoc(),
          new (context_) ESTree::FunctionTypeParamNode(
              /* name */ nullptr, typeAnnotation, /* optional */ false));
    } else {
      params.push_back(*setLocation(
          thisStart,
          getPrevTokenEndLoc(),
          new (context_) ESTree::FunctionTypeParamNode(
              nullptr,
              setLocation(
                  thisStart,
                  getPrevTokenEndLoc(),
                  new (context_) ESTree::GenericTypeAnnotationNode(
                      setLocation(
                          thisStart,
                          getPrevTokenEndLoc(),
                          new (context_) ESTree::IdentifierNode(
                              thisIdent_, nullptr, false)),
                      {})),
              false)));
    }
    checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow);
  }

  while (!check(TokenKind::r_paren)) {
    bool isRest =
        checkAndEat(TokenKind::dotdotdot, JSLexer::GrammarContext::Flow);

    auto optParam = parseFunctionTypeAnnotationParam();
    if (!optParam)
      return None;

    if (isRest) {
      // Rest param must be the last param.
      rest = *optParam;
      checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow);
      break;
    } else {
      params.push_back(**optParam);
      if (!checkAndEat(TokenKind::comma, JSLexer::GrammarContext::Flow)) {
        break;
      }
    }
  }

  if (!eat(
          TokenKind::r_paren,
          JSLexer::GrammarContext::Flow,
          "at end of function annotation parameters",
          "start of parameters",
          start))
    return None;

  return rest;
}

Optional<ESTree::FunctionTypeParamNode *>
JSParserImpl::parseFunctionTypeAnnotationParam() {
  SMLoc start = tok_->getStartLoc();

  if (check(TokenKind::rw_this)) {
    error(
        tok_->getSourceRange(),
        "'this' constraint must be the first parameter");
  }

  auto optLeft = parseTypeAnnotation();
  if (!optLeft)
    return None;

  ESTree::Node *name = nullptr;
  ESTree::Node *typeAnnotation = nullptr;
  bool optional = false;

  if (check(TokenKind::colon, TokenKind::question)) {
    // The node is actually supposed to be an identifier, not a TypeAnnotation.
    auto optName = reparseTypeAnnotationAsIdentifier(*optLeft);
    if (!optName)
      return None;
    name = *optName;
    optional = checkAndEat(TokenKind::question, JSLexer::GrammarContext::Flow);
    if (!eat(
            TokenKind::colon,
            JSLexer::GrammarContext::Flow,
            "in function parameter type annotation",
            "start of parameter",
            start))
      return None;
    auto optType = parseTypeAnnotation();
    if (!optType)
      return None;
    typeAnnotation = *optType;
  } else {
    typeAnnotation = *optLeft;
  }

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_)
          ESTree::FunctionTypeParamNode(name, typeAnnotation, optional));
}

Optional<ESTree::GenericTypeAnnotationNode *> JSParserImpl::parseGenericType() {
  assert(check(TokenKind::identifier) || tok_->isResWord());
  SMLoc start = tok_->getStartLoc();

  ESTree::Node *id = setLocation(
      tok_,
      tok_,
      new (context_) ESTree::IdentifierNode(
          tok_->getResWordOrIdentifier(), nullptr, false));
  advance(JSLexer::GrammarContext::Flow);

  while (checkAndEat(TokenKind::period, JSLexer::GrammarContext::Flow)) {
    if (!check(TokenKind::identifier) && !tok_->isResWord()) {
      errorExpected(
          TokenKind::identifier,
          "in qualified generic type name",
          "start of type name",
          start);
      return None;
    }
    ESTree::Node *next = setLocation(
        tok_,
        tok_,
        new (context_) ESTree::IdentifierNode(
            tok_->getResWordOrIdentifier(), nullptr, false));
    advance(JSLexer::GrammarContext::Flow);
    id = setLocation(
        id, next, new (context_) ESTree::QualifiedTypeIdentifierNode(id, next));
  }

  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optTypeParams = parseTypeArgs();
    if (!optTypeParams)
      return None;
    typeParams = *optTypeParams;
  }

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::GenericTypeAnnotationNode(id, typeParams));
}

Optional<ESTree::ClassImplementsNode *> JSParserImpl::parseClassImplements() {
  assert(check(TokenKind::identifier));
  SMLoc start = tok_->getStartLoc();

  ESTree::Node *id = setLocation(
      tok_,
      tok_,
      new (context_)
          ESTree::IdentifierNode(tok_->getIdentifier(), nullptr, false));
  advance(JSLexer::GrammarContext::Flow);

  ESTree::Node *typeParams = nullptr;
  if (check(TokenKind::less)) {
    auto optTypeParams = parseTypeArgs();
    if (!optTypeParams)
      return None;
    typeParams = *optTypeParams;
  }

  return setLocation(
      start,
      getPrevTokenEndLoc(),
      new (context_) ESTree::ClassImplementsNode(id, typeParams));
}

Optional<ESTree::Node *> JSParserImpl::parsePredicate() {
  assert(check(checksIdent_));
  SMRange checksRng = advance(JSLexer::GrammarContext::Flow);
  if (checkAndEat(TokenKind::l_paren, JSLexer::GrammarContext::AllowRegExp)) {
    auto optCond = parseConditionalExpression();
    if (!optCond)
      return None;
    SMLoc end = tok_->getEndLoc();
    if (!eat(
            TokenKind::r_paren,
            JSLexer::GrammarContext::Flow,
            "in declared predicate",
            "start of predicate",
            checksRng.Start))
      return None;
    return setLocation(
        checksRng, end, new (context_) ESTree::DeclaredPredicateNode(*optCond));
  }
  return setLocation(
      checksRng, checksRng, new (context_) ESTree::InferredPredicateNode());
}

Optional<ESTree::IdentifierNode *>
JSParserImpl::reparseTypeAnnotationAsIdentifier(ESTree::Node *typeAnnotation) {
  UniqueString *id = nullptr;
  if (isa<ESTree::AnyTypeAnnotationNode>(typeAnnotation)) {
    id = anyIdent_;
  } else if (isa<ESTree::EmptyTypeAnnotationNode>(typeAnnotation)) {
    id = emptyIdent_;
  } else if (isa<ESTree::BooleanTypeAnnotationNode>(typeAnnotation)) {
    id = booleanIdent_;
  } else if (isa<ESTree::NumberTypeAnnotationNode>(typeAnnotation)) {
    id = numberIdent_;
  } else if (isa<ESTree::StringTypeAnnotationNode>(typeAnnotation)) {
    id = stringIdent_;
  } else if (isa<ESTree::SymbolTypeAnnotationNode>(typeAnnotation)) {
    id = symbolIdent_;
  } else if (isa<ESTree::NullLiteralTypeAnnotationNode>(typeAnnotation)) {
    id = nullIdent_;
  } else if (
      auto *generic =
          dyn_cast<ESTree::GenericTypeAnnotationNode>(typeAnnotation)) {
    if (auto *genericId = dyn_cast<ESTree::IdentifierNode>(generic->_id)) {
      id = genericId->_name;
    }
  }

  if (!id) {
    error(typeAnnotation->getSourceRange(), "identifier expected");
  }

  return setLocation(
      typeAnnotation,
      typeAnnotation,
      new (context_) ESTree::IdentifierNode(id, nullptr, false));
}

Optional<ESTree::Node *> JSParserImpl::parseEnumDeclaration() {
  assert(check(TokenKind::rw_enum));
  SMLoc start = advance().Start;

  auto optIdent = parseBindingIdentifier(Param{});
  if (!optIdent) {
    errorExpected(
        TokenKind::identifier,
        "in enum declaration",
        "start of declaration",
        start);
    return None;
  }
  ESTree::Node *id = *optIdent;

  OptValue<EnumKind> optKind = llvh::None;
  Optional<SMLoc> explicitTypeStart = None;
  if (check(ofIdent_)) {
    explicitTypeStart = advance().Start;

    if (checkAndEat(stringIdent_)) {
      optKind = EnumKind::String;
    } else if (checkAndEat(numberIdent_)) {
      optKind = EnumKind::Number;
    } else if (checkAndEat(booleanIdent_)) {
      optKind = EnumKind::Boolean;
    } else if (checkAndEat(symbolIdent_)) {
      optKind = EnumKind::Symbol;
    }
  }

  if (!need(
          TokenKind::l_brace,
          "in enum declaration",
          "start of declaration",
          start))
    return None;

  auto optBody = parseEnumBody(optKind, explicitTypeStart);
  if (!optBody)
    return None;

  return setLocation(
      start,
      *optBody,
      new (context_) ESTree::EnumDeclarationNode(id, *optBody));
}

Optional<ESTree::Node *> JSParserImpl::parseEnumBody(
    OptValue<EnumKind> optKind,
    Optional<SMLoc> explicitTypeStart) {
  assert(check(TokenKind::l_brace));
  SMLoc start = advance().Start;

  ESTree::NodeList members{};
  bool hasUnknownMembers = false;
  while (!check(TokenKind::r_brace)) {
    if (check(TokenKind::dotdotdot)) {
      SMLoc dotdotdotLoc = advance(JSLexer::GrammarContext::Flow).Start;
      if (!check(TokenKind::r_brace)) {
        error(
            dotdotdotLoc,
            "The `...` must come after all enum members. "
            "Move it to the end of the enum body.");
        return None;
      }
      hasUnknownMembers = true;
      break;
    }
    if (!need(
            TokenKind::identifier,
            "in enum declaration",
            "start of declaration",
            start))
      return None;

    auto optMember = parseEnumMember();
    if (!optMember)
      return None;
    ESTree::Node *member = *optMember;
    OptValue<EnumKind> optMemberKind = getMemberEnumKind(member);

    if (optKind.hasValue()) {
      // We've already figured out the type of the enum, so ensure that the
      // new member is compatible with this.
      if (optMemberKind.hasValue()) {
        if (*optKind != *optMemberKind) {
          error(
              member->getSourceRange(),
              llvh::Twine("cannot use ") + enumKindStr(*optMemberKind) +
                  " initializer in " + enumKindStr(*optKind) + " enum");
          sm_.note(start, "start of enum body", Subsystem::Parser);
          return None;
        }
      }
    } else {
      optKind = optMemberKind;
    }

    members.push_back(*member);
    if (!checkAndEat(TokenKind::comma))
      break;
  }

  if (!members.empty()) {
    // Ensure that enum members use initializers consistently.
    // This is vacuously true when `members` is empty, so just make sure
    // all members use initializers iff the first member does.
    bool usesInitializers =
        !isa<ESTree::EnumDefaultedMemberNode>(members.front());
    for (const ESTree::Node &member : members) {
      if (usesInitializers != !isa<ESTree::EnumDefaultedMemberNode>(member)) {
        error(
            member.getSourceRange(),
            "enum members need to consistently either all use initializers, "
            "or use no initializers");
        sm_.note(
            members.front().getSourceRange(),
            "first enum member",
            Subsystem::Parser);
        return None;
      }
    }

    if (!usesInitializers) {
      // It's only legal to use defaulted members for string and symbol enums,
      // because other kinds of enums can't infer values from names.
      if (optKind.hasValue() && *optKind != EnumKind::String &&
          *optKind != EnumKind::Symbol) {
        error(start, "number and boolean enums must use initializers");
        return None;
      }
    }
  }

  SMLoc end = tok_->getEndLoc();
  if (!eat(
          TokenKind::r_brace,
          JSLexer::GrammarContext::AllowRegExp,
          "in enum body",
          "start of body",
          start))
    return None;

  bool hasExplicitType = explicitTypeStart.hasValue();
  if (hasExplicitType) {
    start = *explicitTypeStart;
  }

  if (!optKind.hasValue()) {
    return setLocation(
        start,
        end,
        new (context_) ESTree::EnumStringBodyNode(
            std::move(members), hasExplicitType, hasUnknownMembers));
  }

  // There are different node kinds per enum kind.
  switch (*optKind) {
    case EnumKind::String:
      return setLocation(
          start,
          end,
          new (context_) ESTree::EnumStringBodyNode(
              std::move(members), hasExplicitType, hasUnknownMembers));
    case EnumKind::Number:
      return setLocation(
          start,
          end,
          new (context_) ESTree::EnumNumberBodyNode(
              std::move(members), hasExplicitType, hasUnknownMembers));
    case EnumKind::Boolean:
      return setLocation(
          start,
          end,
          new (context_) ESTree::EnumBooleanBodyNode(
              std::move(members), hasExplicitType, hasUnknownMembers));
    case EnumKind::Symbol:
      assert(
          hasExplicitType && "symbol enums can only be made via explicit type");
      return setLocation(
          start,
          end,
          new (context_) ESTree::EnumSymbolBodyNode(
              std::move(members), hasUnknownMembers));
  }
  llvm_unreachable("No other kind of enum");
}

Optional<ESTree::Node *> JSParserImpl::parseEnumMember() {
  assert(check(TokenKind::identifier));
  ESTree::Node *id = setLocation(
      tok_,
      tok_,
      new (context_)
          ESTree::IdentifierNode(tok_->getIdentifier(), nullptr, false));
  advance();

  ESTree::Node *member = nullptr;
  if (checkAndEat(TokenKind::equal)) {
    // Parse initializer.
    if (check(TokenKind::rw_true, TokenKind::rw_false)) {
      ESTree::Node *init = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::BooleanLiteralNode(check(TokenKind::rw_true)));
      member = setLocation(
          id, tok_, new (context_) ESTree::EnumBooleanMemberNode(id, init));
    } else if (check(TokenKind::string_literal)) {
      ESTree::Node *init = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::StringLiteralNode(tok_->getStringLiteral()));
      member = setLocation(
          id, tok_, new (context_) ESTree::EnumStringMemberNode(id, init));
    } else if (check(TokenKind::numeric_literal)) {
      ESTree::Node *init = setLocation(
          tok_,
          tok_,
          new (context_) ESTree::NumericLiteralNode(tok_->getNumericLiteral()));
      member = setLocation(
          id, tok_, new (context_) ESTree::EnumNumberMemberNode(id, init));
    } else {
      errorExpected(
          {TokenKind::rw_true,
           TokenKind::rw_false,
           TokenKind::string_literal,
           TokenKind::numeric_literal},
          "in enum member initializer",
          "start of enum member",
          id->getStartLoc());
      return None;
    }
    advance();
  } else {
    member =
        setLocation(id, id, new (context_) ESTree::EnumDefaultedMemberNode(id));
  }

  assert(member != nullptr && "member must have been parsed");
  return member;
}

#endif

} // namespace detail
} // namespace parser
} // namespace hermes
