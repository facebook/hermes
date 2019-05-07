/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_IRGEN_ESTREEIRGEN_H
#define HERMES_IRGEN_ESTREEIRGEN_H

#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/AST/SemValidate.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IRGen/IRGen.h"

#include "llvm/Support/Debug.h"

// Use this value to enable debug logging from the command line.
#define DEBUG_TYPE "irgen"

using llvm::dbgs;
using llvm::dyn_cast_or_null;

namespace hermes {
namespace irgen {

// Forward declarations
class ESTreeIRGen;

//===----------------------------------------------------------------------===//
// Free standing helpers.

/// Emit an instruction to load a value from a specified location.
/// \param from location to load from, either a Variable or
/// GlobalObjectProperty. \param inhibitThrow  if true, do not throw when
/// loading from mmissing global properties. \return the instruction performing
/// the load.
Instruction *
emitLoad(IRBuilder &builder, Value *from, bool inhibitThrow = false);

/// Emit an instruction to a store a value into the specified location.
/// \param storedValue value to store
/// \param ptr location to store into, either a Variable or
/// GlobalObjectProperty. \return the instruction performing the store.
Instruction *emitStore(IRBuilder &builder, Value *storedValue, Value *ptr);

/// Return the name field from ID nodes.
inline Identifier getNameFieldFromID(const ESTree::Node *ID) {
  return Identifier::getFromPointer(cast<ESTree::IdentifierNode>(ID)->_name);
}

/// \returns true if \p node is known to be a constant expression.
bool isConstantExpr(ESTree::Node *node);

//===----------------------------------------------------------------------===//
// FunctionContext

/// Scoped hash table to represent the JS scope.
using NameTableTy = hermes::ScopedHashTable<Identifier, Value *>;
using NameTableScopeTy = hermes::ScopedHashTableScope<Identifier, Value *>;

/// Holds the target basic block for a break and continue label.
/// It has a 1-to-1 correspondence to SemInfoFunction::GotoLabel.
struct GotoLabel {
  BasicBlock *breakTarget = nullptr;
  BasicBlock *continueTarget = nullptr;
};

/// Holds per-function state, specifically label tables. Should be constructed
/// on the stack. Upon destruction it automatically restores the previous
/// function context.
class FunctionContext {
  /// Pointer to the "outer" object this is associated with.
  ESTreeIRGen *const irGen_;

  /// Semantic info of the funciton we are emitting.
  sem::FunctionInfo *const semInfo_;

  /// The old value which we save and will restore on destruction.
  FunctionContext *oldContext_;

  /// As we descend into a new function, we save the state of the builder
  /// here. It is automatically restored once we are done with the function.
  IRBuilder::SaveRestore builderSaveState_;

 public:
  /// This is the actual function associated with this context.
  Function *const function;

  /// A vector of labels corresponding 1-to-1 to the labels defined in
  /// \c semInfo_.
  llvm::SmallVector<GotoLabel, 2> labels;

  /// A new variable scope that is used throughout the body of the function.
  NameTableScopeTy scope;

  /// The function's entry block terminator, saved so we can append
  /// instructions that must execute before the body of the function.
  TerminatorInst *entryTerminator{};

  /// Stack Register that will hold the return value of the global scope.
  AllocStackInst *globalReturnRegister{nullptr};

  /// A running counter for anonymous closure. We append this number to some
  /// label to create a unique number;
  size_t anonymousLabelCounter{0};

  /// This holds the created arguments and is lazily initialized the first
  /// time we encounter usage of 'arguments'.
  CreateArgumentsInst *createdArguments{};

  /// Parents of arrow functions need to capture their "this" parameter so the
  /// arrow function can use it. Normal functions and constructors store their
  /// "this" in a variable and record it here, if they contain at least one
  /// arrow function. Arrow functions always copy their parent's value.
  Variable *capturedThis{};

  /// Captured value of new target. In ES5 functions and ES6 constructors it
  /// is a Variable with the result of GetNewTargetInst executed at the start
  /// of the function. In ES6 methods it will be initialized to
  /// LiteralUndefined. In arrow functions it is a copy of the parent's
  /// capturedNewTarget.
  Value *capturedNewTarget{};

  /// Optionally captured value of the eagerly created Arguments object. Used
  /// when arrow functions need to access it.
  Variable *capturedArguments{};

  /// Initialize a new function context, while preserving the previous one.
  /// \param irGen the associated ESTreeIRGen object.
  /// \param function the newly created Function IR node.
  /// \param semInfo semantic info obtained from the AST node.
  FunctionContext(
      ESTreeIRGen *irGen,
      Function *function,
      sem::FunctionInfo *semInfo);

  ~FunctionContext();

  /// The previous (outer) function context on the stack.
  FunctionContext *getPreviousContext() const {
    return oldContext_;
  }

  /// \return the associated semantic information, asserting that it is
  /// present.
  sem::FunctionInfo *getSemInfo() {
    assert(semInfo_ && "semInfo is not set");
    return semInfo_;
  }

  /// Generate a unique string that represents a temporary value. The string
  /// \p hint appears in the name.
  Identifier genAnonymousLabelName(StringRef const hint);
};

//===----------------------------------------------------------------------===//
// LReference

/// This is a utility class that's related to the ES5 Reference type without
/// the runtime semantic parts - https://es5.github.io/#x8.7.
class LReference {
 public:
  enum class Kind {
    /// Elision in destructuring pattern.
    Empty,
    /// Reference to a member expression.
    Member,
    /// Reference to a variable or a global property.
    VarOrGlobal,
    /// Invalid reference. Error has been reported, so it does nothing.
    Error,
    /// Destructuring assignment target.
    Destructuring,
  };

  LReference(
      Kind kind,
      ESTreeIRGen *irgen,
      Value *base,
      Value *property,
      SMLoc loadLoc)
      : kind_(kind), irgen_(irgen) {
    base_ = base;
    property_ = property;
    loadLoc_ = loadLoc;
  }

  LReference(ESTreeIRGen *irgen, ESTree::PatternNode *target)
      : kind_(Kind::Destructuring), irgen_(irgen) {
    destructuringTarget_ = target;
  }

  bool isEmpty() const {
    return kind_ == Kind::Empty;
  }

  Value *emitLoad();
  void emitStore(Value *value);

  Variable *castAsVariable() const;
  GlobalObjectProperty *castAsGlobalObjectProperty() const;

 private:
  /// Self explanatory.
  Kind kind_;

  /// The associated instance of ESTreeIRGen.
  ESTreeIRGen *irgen_;

  union {
    struct {
      /// The base of the object, or the variable we load from.
      Value *base_;
      /// The name/value of the field this reference accesses, or null if this
      /// is a variable access.
      Value *property_;
    };

    /// Destructuring assignment target.
    ESTree::PatternNode *destructuringTarget_;
  };

  /// Debug position for loads. Must be outside of the union because it has a
  /// constructor.
  SMLoc loadLoc_;

  /// \return a reference to IRGen's builder.
  IRBuilder &getBuilder();
};

//===----------------------------------------------------------------------===//
// ESTreeIRGen

/// Performs lowering of the JSON ESTree down to Hermes IR.
class ESTreeIRGen {
  friend class FunctionContext;
  friend class LReference;

  using BasicBlockListType = llvm::SmallVector<BasicBlock *, 4>;

  /// The module we are constructing.
  Module *Mod;
  /// The IRBuilder we use to construct the module.
  IRBuilder Builder;
  /// The root of the ESTree.
  ESTree::Node *Root;
  /// This is a list of parsed global property declaration files.
  const DeclarationFileListTy &DeclarationFileList;
  /// This points to the context of the top-level function, which we
  /// occasionally need.
  FunctionContext *topLevelContext{};
  /// This points to the current function's context. It is saved and restored
  /// whenever we enter a nested function.
  FunctionContext *functionContext_{};
  /// This is the scoped hash table that saves the mapping between the declared
  /// names and an instance of Varible or GlobalObjectProperty.
  NameTableTy nameTable_{};

  /// Lexical scope chain from the runtime, used to resolve identifiers in local
  /// eval.
  std::shared_ptr<SerializedScope> lexicalScopeChain;

  /// Identifier representing the string "eval".
  const Identifier identEval_;

  /// Generate a unique string that represents a temporary value. The string \p
  /// hint appears in the name.
  Identifier genAnonymousLabelName(StringRef hint) {
    return curFunction()->genAnonymousLabelName(hint);
  }

  /// Generate a unique string that represents a temporary value. The string \p
  /// hint appears in the name.
  Identifier genAnonymousLabelName(Identifier hint) {
    return genAnonymousLabelName(hint.isValid() ? hint.str() : "anonymous");
  }

 public:
  explicit ESTreeIRGen(
      ESTree::Node *root,
      const DeclarationFileListTy &declFileList,
      Module *M,
      const ScopeChain &scopeChain);

  /// Perform IRGeneration for the whole module.
  void doIt();

  /// Perform IR generation for a given CJS module.
  void doCJSModule(
      Function *topLevelFunction,
      sem::FunctionInfo *semInfo,
      llvm::StringRef filename);

  /// Perform IR generation for a lazy function.
  /// \return the newly allocated generated Function IR.
  Function *doLazyFunction(hbc::LazyCompilationData *lazyData);

  /// Generate a function which immediately throws the specified SyntaxError
  /// message.
  static Function *genSyntaxErrorFunction(
      Module *M,
      Identifier originalName,
      SMRange sourceRange,
      StringRef error);

 private:
  /// @name statements
  /// @{

  /// \brief Perform IRGen for \p body, which can be the body of a function or
  /// the top-level of a program.
  void genBody(ESTree::NodeList &Body);

  /// Generate code for the statement \p Stmt.
  void genStatement(ESTree::Node *stmt);

  /// Wrapper of genExpression. If curFunction()->globalReturnRegister is
  /// set, stores the expression value into it.
  void genExpressionWrapper(ESTree::Node *expr);

  void genVariableDeclaration(ESTree::VariableDeclarationNode *declaration);
  void genVariableDeclarator(
      ESTree::NodeLabel kind,
      ESTree::VariableDeclaratorNode *declarator);

  void genIfStatement(ESTree::IfStatementNode *IfStmt);
  void genReturnStatement(ESTree::ReturnStatementNode *RetStmt);
  void genForInStatement(ESTree::ForInStatementNode *ForInStmt);
  void genForOfStatement(ESTree::ForOfStatementNode *forOfStmt);

  /// Generate IR for for/while/do..while statements.
  /// \p loop the loop statement node
  /// \p init optional init statement for 'for' loops.
  /// \p preTest the expression that guards the loop entrance.
  /// \p postTest the expression that guards the loop.
  /// \p update optional statement for the update phase of 'for' loops.
  /// \o body the body of the loop.
  void genForWhileLoops(
      ESTree::LoopStatementNode *loop,
      ESTree::Node *init,
      ESTree::Node *preTest,
      ESTree::Node *postTest,
      ESTree::Node *update,
      ESTree::Node *body);

  void genSwitchStatement(ESTree::SwitchStatementNode *switchStmt);

  /// Check if all cases values are constant and stores them in
  /// \p casdeLiterals. The default case, if present, is stored as nullptr, to
  ///     maintain the ordering of the cases.
  /// \returns true if all cases are constant.
  bool areAllCasesConstant(
      ESTree::SwitchStatementNode *switchStmt,
      llvm::SmallVectorImpl<Literal *> &caseLiterals);

  /// Generate IR Switch statements when all case tests are constant.
  void genConstSwitchStmt(
      ESTree::SwitchStatementNode *switchStmt,
      llvm::SmallVectorImpl<Literal *> &caseLiterals);
  /// @}

 private:
  /// @name expressions
  /// @{

  /// Generate IR for the expression \p Expr.
  /// \p nameHint is used to provide names for anonymous functions.
  /// We currently provide names for functions that are assigned to a variable,
  /// or functions that are assigned to an object key. These are a subset of
  /// ES6, but not all of it.
  Value *genExpression(ESTree::Node *expr, Identifier nameHint = Identifier{});

  /// Generate an expression and perform a conditional branch depending on
  /// whether the expression evaluates to true or false.
  void genExpressionBranch(
      ESTree::Node *expr,
      BasicBlock *onTrue,
      BasicBlock *onFalse);

  Value *genObjectExpr(ESTree::ObjectExpressionNode *Expr);
  Value *genArrayExpr(ESTree::ArrayExpressionNode *Expr);
  Value *genCallExpr(ESTree::CallExpressionNode *call);

  /// Generate IR for a direct call to eval(). This is invoked from
  /// genCallExpr().
  Value *genCallEvalExpr(ESTree::CallExpressionNode *call);

  Value *genNewExpr(ESTree::NewExpressionNode *N);
  Value *genAssignmentExpr(ESTree::AssignmentExpressionNode *AE);
  Value *genConditionalExpr(ESTree::ConditionalExpressionNode *C);
  Value *genSequenceExpr(ESTree::SequenceExpressionNode *Sq);
  Value *genYieldExpr(ESTree::YieldExpressionNode *Y);
  Value *genUnaryExpression(ESTree::UnaryExpressionNode *U);
  Value *genUpdateExpr(ESTree::UpdateExpressionNode *updateExpr);
  Value *genLogicalExpression(ESTree::LogicalExpressionNode *logical);

  /// Generate IR for the logical expression \p logical and jump to the
  /// corresponding label depending on its value.
  void genLogicalExpressionBranch(
      ESTree::LogicalExpressionNode *logical,
      BasicBlock *onTrue,
      BasicBlock *onFalse);

  /// Generate IR for the identifier expression.
  /// We need to know whether it's after typeof (\p afterTypeOf),
  /// to help decide whether a load can throw.
  Value *genIdentifierExpression(
      ESTree::IdentifierNode *Iden,
      bool afterTypeOf);

  Value *genMetaProperty(ESTree::MetaPropertyNode *MP);

  /// Generate IR for a template literal expression, which in most cases is
  /// translated to a call to HermesInternal.concat().
  Value *genTemplateLiteralExpr(ESTree::TemplateLiteralNode *Expr);

  /// Generate IR for a tagged template expression, which involves converting
  /// a template literal to a template object, then invoking the tag function
  /// with the template object and substitution expressions.
  Value *genTaggedTemplateExpr(ESTree::TaggedTemplateExpressionNode *Expr);

  /// Generate IR for handling resuming a suspended generator.
  /// \param yield the corresponding yield to resume from, nullptr if this is
  ///   the first resume and there is no yield.
  /// \param isReturn the slot indicating whether the user requested a return.
  /// \param nextBB the next BasicBlock to run if the user did not request a
  ///   return.
  Value *genResumeGenerator(
      ESTree::YieldExpressionNode *yield,
      AllocStackInst *isReturn,
      BasicBlock *nextBB);

  /// @}

 private:
  /// @name Exception handling.
  /// @{

  /// Generate IR for try/catch/finally statement.
  void genTryStatement(ESTree::TryStatementNode *tryStmt);

  /// Generate IR "catch (e)".
  CatchInst *prepareCatch(ESTree::NodePtr catchParam);

  /// When we see a control change such as return, break, continue,
  /// we need to make sure to generate code for finally block if
  /// we are under a try/catch.
  /// \param sourceTry  the try statement surrounding the AST node performing
  ///   the goto (break, continue, return)
  /// \param targetTry  the try statement surrounding the AST node of the label.
  void genFinallyBeforeControlChange(
      ESTree::TryStatementNode *sourceTry,
      ESTree::TryStatementNode *targetTry);

  /// @}

 private:
  /// @name functions
  /// @{

  /// Generate IR for function declarations;
  void genFunctionDeclaration(ESTree::FunctionDeclarationNode *func);

  /// Generate IR for FunctionExpression.
  Value *genFunctionExpression(
      ESTree::FunctionExpressionNode *FE,
      Identifier nameHint);

  /// Generate IR for ArrowFunctionExpression.
  Value *genArrowFunctionExpression(
      ESTree::ArrowFunctionExpressionNode *AF,
      Identifier nameHint);

  /// Generate IR for an ES5 function (function declaration or expression).
  /// The body may optionally be lazy.
  /// \param originalName is the original non-unique name specified by the user
  ///   or inferred according to the rules of ES6.
  /// \param lazyClosureAlias an optional variable in the parent that will
  ///   contain the closure being created. It is non-null only if an alias
  ///   binding from  \c originalName to the variable was created and is
  ///   available inside the closure. Used only by lazy compilation.
  /// \param functionNode is the ESTree function node (declaration, expression,
  ///   object method).
  /// \param isGeneratorInnerFunction whether this is a GeneratorInnerFunction.
  /// \returns a new Function.
  Function *genES5Function(
      Identifier originalName,
      Variable *lazyClosureAlias,
      ESTree::FunctionLikeNode *functionNode,
      bool isGeneratorInnerFunction = false);

  /// Generate the IR for two functions: an outer GeneratorFunction and an inner
  /// GeneratorInnerFunction. The outer function runs CreateGenerator on the
  /// inner function and returns the result.
  /// \param originalName is the original non-unique name specified by the user
  ///   or inferred according to the rules of ES6.
  /// \param lazyClosureAlias an optional variable in the parent that will
  ///   contain the closure being created. It is non-null only if an alias
  ///   binding from  \c originalName to the variable was created and is
  ///   available inside the closure. Used only by lazy compilation.
  /// \param functionNode is the ESTree function node (declaration, expression,
  ///   object method).
  /// \return the outer Function.
  Function *genGeneratorFunction(
      Identifier originalName,
      Variable *lazyClosureAlias,
      ESTree::FunctionLikeNode *functionNode);

  /// In the beginning of an ES5 function, initialize the special captured
  /// variables needed by arrow functions, constructors and methods.
  /// This is used only by \c genES5Function() and the global scope.
  void initCaptureStateInES5Function();

  /// Emit the function prologue for the current function, consisting of the
  /// following things:
  /// - a next block, so we can append instructions to it to generate the actual
  ///   code for the function
  /// - declare all hoisted es5 variables and global properties
  /// - initialize all hoisted es5 variables to undefined
  /// - declare all hoisted es5 functions and initialize them (recursively
  ///   generating their functions)
  /// - create "this" parameter
  /// - create all explicit parameters and store them in variables
  /// \param entry the unpopulated entry block for the function
  void emitFunctionPrologue(
      ESTree::FunctionLikeNode *funcNode,
      BasicBlock *entry);

  /// Emit the loading and initialization of parameters in the function
  /// prologue.
  void emitParameters(ESTree::FunctionLikeNode *funcNode);

  /// Optionally emit a return value and perform cleanup after emission of the
  /// current function is finished. Specifically it attempts to merge the entry
  /// and the next block in order to create less "noise".
  /// \param returnValue if non-nullptr, a return instruction for it is emitted
  ///   with debug location at the end of the source range.
  void emitFunctionEpilogue(Value *returnValue);

  /// Generate a body for a dummy function so that it doesn't crash the
  /// backend when encountered.
  static void genDummyFunction(Function *dummy);

  /// @}

 private:
  inline FunctionContext *curFunction() {
    assert(functionContext_ && "No active function context");
    return functionContext_;
  }

  /// Declare a variable or a global propery depending in function \p inFunc,
  /// depending on whether it is the global scope. Do nothing if the variable
  /// or property is already declared in that scope.
  /// \return A pair. pair.first is the variable, and pair.second is set to true
  ///   if it was declared, false if it already existed.
  std::pair<Value *, bool> declareVariableOrGlobalProperty(
      Function *inFunc,
      Identifier name);

  /// Declare a new ambient global property, if not already declared.
  GlobalObjectProperty *declareAmbientGlobalProperty(Identifier name);

  /// Scan all the global declarations in the supplied declaration file and
  /// declare them as global properties.
  void processDeclarationFile(ESTree::FileNode *fileNode);

  /// This method ensures that a variable with the name \p name exists in the
  /// current scope. The method reports an error if the variable does not exist
  /// and creates a new variable with this name in the current scope in an
  /// attempt to recover from the error.
  /// \return the existing variable or global property, or a freshly created
  ///   ambient global property in case of error.
  Value *ensureVariableExists(ESTree::IdentifierNode *id);

  /// Generate the IR for the property field of the MemberExpressionNode.
  /// The property field may be a string literal or some computed expression.
  Value *genMemberExpressionProperty(ESTree::MemberExpressionNode *Mem);

  /// Generates a left hand side reference from valid estree nodes
  /// that can be translated to lref.
  LReference createLRef(ESTree::Node *node);

  /// Generate a call to a method of HermesInterna with the specified name \p
  /// name.
  Value *genHermesInternalCall(
      StringRef name,
      Value *thisValue,
      ArrayRef<Value *> args);

  /// Generate code to ensure that \p value is an object and it it isn't, throw
  /// a type error with the specified message.
  void emitEnsureObject(Value *value, StringRef message);

  /// \return the internal value @@iterator
  Value *emitIterarorSymbol();

  /// IteratorRecord as defined in ES2018 7.4.1 GetIterator
  struct IteratorRecord {
    Value *iterator;
    Value *nextMethod;
  };

  /// ES2018 7.4.1 GetIterator
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-getiterator
  ///
  /// Call obj[@@iterator], which should return an iterator, and return the
  /// iterator itself and its \c next() method.
  ///
  /// \return (iterator, nextMethod)
  IteratorRecord emitGetIterator(Value *obj);

  /// ES2018 7.4.2 IteratorNext
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-iteratornext
  ///
  /// Call the next method and throw an exception if the result is not an
  /// object.
  ///
  /// \return the result object.
  Value *emitIteratorNext(IteratorRecord iteratorRecord);

  /// ES2018 7.4.3 IteratorComplete
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-iteratorcomplete
  ///
  /// \return \c iterResult.done
  Value *emitIteratorComplete(Value *iterResult);

  /// ES2018 7.4.4 IteratorValue
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-iteratorvalue
  ///
  /// \return \c iterResult.value
  Value *emitIteratorValue(Value *iterResult);

  /// Generate code for destructuring assignment to ArrayPattern or
  /// ObjectPattern.
  void emitDestructuringAssignment(ESTree::PatternNode *target, Value *source);

  /// Generate code for destructuring assignment to ArrayPattern.
  void emitDestructuringArray(ESTree::ArrayPatternNode *target, Value *source);

  /// Generate code for destructuring assignment to ObjectPattern.
  void emitDestructuringObject(
      ESTree::ObjectPatternNode *target,
      Value *source);

  /// If the initializer \p init is nullptr, just return \p value.
  /// Otherwise emit code to check whether \p value equals \c undefined, and
  /// evaluate and return the initializer in that case.
  Value *emitOptionalInitialization(Value *value, ESTree::Node *init);

 private:
  /// "Converts" a ScopeChain into a SerializedScope by resolving the
  /// identifiers.
  std::shared_ptr<SerializedScope> resolveScopeIdentifiers(
      const ScopeChain &chain);

  /// Materialize the provided scope.
  void materializeScopesInChain(
      Function *wrapperFunction,
      const std::shared_ptr<const SerializedScope> &scope,
      int depth);

  /// Save all variables currently in scope, for lazy compilation.
  std::shared_ptr<SerializedScope> saveCurrentScope();
};

} // namespace irgen
} // namespace hermes

#endif // HERMES_IRGEN_ESTREEIRGEN_H
