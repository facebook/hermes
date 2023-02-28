/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_FLOWCHECKER_H
#define HERMES_SEMA_FLOWCHECKER_H

#include "DeclCollector.h"

#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/AST/Context.h"
#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/Sema/FlowContext.h"
#include "hermes/Sema/Keywords.h"
#include "hermes/Sema/SemContext.h"

namespace hermes {
namespace flow {

/// Class the performs all resolution.
/// Reports errors if validation fails.
class FlowChecker {
  /// AST context.
  Context &astContext_;

  /// A copy of Context::getSM() for easier access.
  SourceErrorManager &sm_;

  /// Buffer all generated messages and print them sorted in the end.
  SourceErrorManager::SaveAndBufferMessages bufferMessages_;

  sema::SemContext &semContext_;

  /// Flow types are stored here.
  FlowContext &flowContext_;

  /// Map of DeclCollector instances associated with every function.
  const sema::DeclCollectorMapTy &declCollectorMap_;

  /// Keywords we will be checking for.
  sema::Keywords &kw_;

  struct TypeDecl {
    /// nullptr is used to indicate a forward declaration. This is a temporary
    /// state while declaring all types in a scope.
    Type *type;
    sema::LexicalScope *scope;
    ESTree::Node *astNode;
    explicit TypeDecl(
        Type *type,
        sema::LexicalScope *scope,
        ESTree::Node *astNode)
        : type(type), scope(scope), astNode(astNode) {}
  };

  /// The scoped binding table mapping from string to binding.
  using TypeBindingTableTy = hermes::ScopedHashTable<UniqueString *, TypeDecl>;
  using TypeBindingTableScopeTy =
      hermes::ScopedHashTableScope<UniqueString *, TypeDecl>;

  /// The currently lexically visible names of types.
  TypeBindingTableTy bindingTable_{};

  /// Information about the current function.
  class FunctionContext;

  /// The current function context.
  FunctionContext *curFunctionContext_ = nullptr;

  /// Information about the current class.
  class ClassContext;

  /// The current class context.
  ClassContext *curClassContext_ = nullptr;

  /// Types associated with declarations.
  llvh::DenseMap<const sema::Decl *, Type *> &declTypes_;
  /// The AST node for every declaration. Used for reporting re-declarations.
  llvh::DenseMap<sema::Decl *, ESTree::Node *> declNodes_{};

  /// True if we are preparing the AST to be compiled by Hermes, including
  /// erroring on features which we parse but don't compile and transforming
  /// the AST. False if we just want to validate the AST.
  bool const compile_;

 public:
  explicit FlowChecker(
      Context &astContext,
      sema::SemContext &semContext,
      FlowContext &flowContext,
      sema::DeclCollectorMapTy &declCollectorMap,
      bool compile);

  /// Run semantic resolution and store the result in \c semCtx_.
  /// \param rootNode the top-level program/JS module node to run resolution on.
  /// \return false on error.
  bool run(ESTree::ProgramNode *rootNode);

  /// This method implements the first part of the stack overflow protection
  /// protocol defined by RecursiveVisitor. We don't need to do anything because
  /// the AST depth has been checked by prior passes.
  bool incRecursionDepth(ESTree::Node *) {
    return true;
  }

  /// This is the second part of the protocol defined by RecursiveVisitor.
  void decRecursionDepth() {}

  /// Default case for all ignored nodes, we still want to visit their children.
  void visit(ESTree::Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::ProgramNode *node);

  void visit(ESTree::FunctionDeclarationNode *node);
  void visit(ESTree::FunctionExpressionNode *node);
  void visit(ESTree::ArrowFunctionExpressionNode *node);

  void visit(ESTree::ClassExpressionNode *node);
  void visit(ESTree::ClassDeclarationNode *node);
  void visit(ESTree::MethodDefinitionNode *node);

  void visit(ESTree::TypeAnnotationNode *node);
  void visit(ESTree::IdentifierNode *identifierNode);

  /// Used in an expression context.
  class ExprVisitor;

  /// Invoke the expression visitor.
  void visitExpression(ESTree::Node *node, ESTree::Node *parent);

  void visit(ESTree::ExpressionStatementNode *node);
  void visit(ESTree::IfStatementNode *node);
  void visit(ESTree::ForStatementNode *node);
  void visit(ESTree::ReturnStatementNode *node);
  void visit(ESTree::BlockStatementNode *node);
  void visit(ESTree::VariableDeclarationNode *node);

 private:
  /// A RAII object automatic scopes.
  class ScopeRAII {
   public:
    /// The binding table scope.
    TypeBindingTableScopeTy bindingScope;

    /// Create a binding scope and push a semantic scope.
    /// \param scopeNode is the AST node with which to associate the scope.
    explicit ScopeRAII(FlowChecker &checker)
        : bindingScope(checker.bindingTable_) {}

    /// Pops the created scope if it was pushed.
    ~ScopeRAII() = default;
  };

  void visitFunctionLike(
      ESTree::FunctionLikeNode *node,
      ESTree::Node *body,
      ESTree::NodeList &params);

  /// Resolve and declare all types named in a scope.
  class DeclareScopeTypes;

  /// Parse all sema declarations type annotations and associate them
  /// with the declarations.
  class AnnotateScopeDecls;

  /// Declare all named types in the specified scope and annotate the
  /// declarations with them.
  void resolveScopeTypesAndAnnotate(
      ESTree::Node *scopeNode,
      sema::LexicalScope *scope);

  /// Record the declaration's type and declaring AST node, while checking for
  /// and reporting re-declarations.
  /// \return true if there was no error.
  bool recordDecl(
      sema::Decl *decl,
      Type *type,
      ESTree::IdentifierNode *id,
      ESTree::Node *astDeclNode);

  /// Return the type associated with a sema::Decl. The association must
  /// exist.
  Type *getDeclType(sema::Decl *decl) const {
    return flowContext_.getDeclType(decl);
  }

  /// Associate a type with an expression node. Do nothing if the specified
  /// type is nullptr.
  void setNodeType(ESTree::Node *node, Type *type) {
    flowContext_.setNodeType(node, type);
  }

  /// Return the optional type associated with a node or "any" if no type.
  Type *getNodeTypeOrAny(ESTree::Node *node) const {
    return flowContext_.getNodeTypeOrAny(node);
  }

  /// Stores information about a "forward" type declaration, which can then
  /// be used to complete the declaration.
  struct ForwardDecl {
    ESTree::Node *astNode;
    Type *type;
    ForwardDecl(ESTree::Node *astNode, Type *type)
        : astNode(astNode), type(type) {}
  };

  /// \param optReturnTypeAnnotation is nullptr or ESTree::TypeAnnotationNode
  /// \param defaultReturnType optional return type if the return annotation
  ///     is missing. nullptr here is a shortcut for "any".
  FunctionType *parseFunctionType(
      ESTree::NodeList &params,
      ESTree::Node *optReturnTypeAnnotation,
      bool isAsync,
      bool isGenerator,
      Type *defaultReturnType = nullptr);

  /// Parse an optional type annotation. If it is nullptr, return any, otherwise
  /// parse the inner annotation (which cannot be null).
  /// \param defaultType optional type to use if the annotation is missing.
  ///     nullptr here is a shortcut for "any".
  Type *parseOptionalTypeAnnotation(
      ESTree::Node *optAnnotation,
      Type *defaultType = nullptr);

  /// Parse a type annotation into a type.
  ///
  /// \param node the type annotation AST (ESTree::TypeAnnotationNode).
  /// \param fwdType if not null, there is an associated forward-declared type,
  ///     that should now be completed.
  /// \param forwardDecls if not null, parsing should end at a complex type
  ///     which should then be recorded in `forwardDecls` so it can be completed
  ///     later.
  Type *parseTypeAnnotation(
      ESTree::Node *node,
      Type *fwdType,
      llvh::SmallVectorImpl<ForwardDecl> *forwardDecls);

  UnionType *parseUnionTypeAnnotation(ESTree::UnionTypeAnnotationNode *node);
  UnionType *parseNullableTypeAnnotation(
      ESTree::NullableTypeAnnotationNode *node);
  ArrayType *parseArrayTypeAnnotation(
      ESTree::ArrayTypeAnnotationNode *node,
      Type *fwdType,
      llvh::SmallVectorImpl<ForwardDecl> *forwardDecls);
  Type *parseGenericTypeAnnotation(ESTree::GenericTypeAnnotationNode *node);

  /// Parse a class type into an already created (but empty) class.
  void parseClassType(
      ESTree::Node *superClass,
      ESTree::Node *body,
      ClassType *classType);

  /// Result indicating whether a type can flow into another type. If it can,
  /// additionally indicates whether a checked cast is needed.
  struct CanFlowResult {
    bool canFlow = false;
    /// When the type can flow, this field indicates whether a checked cast is
    /// needed.
    bool needCheckedCast = false;
  };

  /// Return true if type \p a can "flow" into type \p b.
  /// TODO: generate message explaining why not.
  static CanFlowResult canAFlowIntoB(Type *a, Type *b);
  static CanFlowResult canAFlowIntoB(ClassType *a, ClassType *b);
  static CanFlowResult canAFlowIntoB(FunctionType *a, FunctionType *b);

  /// If \c canFlow.needCheckedCast is set and \c compile_ is set, allocate an
  /// implicit checked cast node from the specified \p argument to
  /// the specified type \p toType and return it. Otherwise return the argument.
  ESTree::Node *implicitCheckedCast(
      ESTree::Node *argument,
      Type *toType,
      CanFlowResult canFlow);

  /// Return the non-null expression sema::Decl associated with the identifier.
  sema::Decl *getDecl(ESTree::IdentifierNode *id) {
    return semContext_.getExpressionDecl(id);
  }
};

class FlowChecker::FunctionContext {
  FlowChecker &outer_;
  /// The previous context to be restored on destruction. nullptr if this is the
  /// global function.
  FunctionContext *const prevContext_;

 public:
  /// The DeclCollector associated with this function.
  const sema::DeclCollector *const declCollector;

  /// The external signature of the current function. If nullptr, this is the
  /// global function.
  FunctionType *const functionType;

  /// The type of the "this" parameter. If nullptr, this is a global function
  /// with an implicit "this" paramater. Depending on the compilation mode,
  /// usages may be invalid.
  ///
  /// Note that the type of "this" is not necessarily the same as
  /// FunctionType::thisParamType, because of arrow functions.
  Type *const thisParamType;

  FunctionContext(const FunctionContext &) = delete;
  void operator=(const FunctionContext &) = delete;

  /// \param declCollectorNode the AST node with an associated DeclCollector.
  FunctionContext(
      FlowChecker &outer,
      ESTree::FunctionLikeNode *declCollectorNode,
      FunctionType *functionType,
      Type *thisParamType)
      : outer_(outer),
        prevContext_(outer.curFunctionContext_),
        declCollector(
            outer.declCollectorMap_.find(declCollectorNode)->second.get()),
        functionType(functionType),
        thisParamType(thisParamType) {
    outer.curFunctionContext_ = this;
  }

  ~FunctionContext() {
    outer_.curFunctionContext_ = prevContext_;
  }
};

// TODO: this is probably not needed. We can directly visit the class members,
//       passing in the required info.
class FlowChecker::ClassContext {
  FlowChecker &outer_;

  /// The previous context, restored on destruction.
  ClassContext *const prevContext_;

 public:
  ClassType *const classType;

  ClassContext(const ClassContext &) = delete;
  void operator=(const ClassContext &) = delete;

  ClassContext(FlowChecker &outer, ClassType *const classType)
      : outer_(outer),
        prevContext_(outer.curClassContext_),
        classType(classType) {
    outer.curClassContext_ = this;
  }

  ~ClassContext() {
    outer_.curClassContext_ = prevContext_;
  }
};

} // namespace flow
} // namespace hermes

#endif
