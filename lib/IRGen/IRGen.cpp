/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/IRGen/IRGen.h"

#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/AST/ESTree.h"
#include "hermes/AST/SemanticValidator.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Support/SimpleDiagHandler.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SaveAndRestore.h"
#include "llvm/Support/raw_ostream.h"

// Use this value to enable debug logging from the command line.
#define DEBUG_TYPE "irgen"

using namespace hermes;

using llvm::cast;
using llvm::dbgs;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::isa;

/// Return the name field from ID nodes.
static Identifier getNameFieldFromID(const ESTree::Node *ID) {
  return Identifier::getFromPointer(cast<ESTree::IdentifierNode>(ID)->_name);
}

/// Emit an instruction to load a value from a specified location.
/// \param from location to load from, either a Variable or
/// GlobalObjectProperty. \param inhibitThrow  if true, do not throw when
/// loading from mmissing global properties. \return the instruction performing
/// the load.
static Instruction *
emitLoad(IRBuilder &builder, Value *from, bool inhibitThrow = false) {
  if (auto *var = dyn_cast<Variable>(from)) {
    return builder.createLoadFrameInst(var);
  } else if (auto *globalProp = dyn_cast<GlobalObjectProperty>(from)) {
    if (globalProp->isDeclared() || inhibitThrow)
      return builder.createLoadPropertyInst(
          builder.getGlobalObject(), globalProp->getName());
    else
      return builder.createTryLoadGlobalPropertyInst(globalProp);
  } else {
    llvm_unreachable("unvalid value to load from");
  }
}

/// Emit an instruction to a store a value into the specified location.
/// \param storedValue value to store
/// \param ptr location to store into, either a Variable or
/// GlobalObjectProperty. \return the instruction performing the store.
static Instruction *
emitStore(IRBuilder &builder, Value *storedValue, Value *ptr) {
  if (auto *var = dyn_cast<Variable>(ptr)) {
    return builder.createStoreFrameInst(storedValue, var);
  } else if (auto *globalProp = dyn_cast<GlobalObjectProperty>(ptr)) {
    if (globalProp->isDeclared() || !builder.getFunction()->isStrictMode())
      return builder.createStorePropertyInst(
          storedValue, builder.getGlobalObject(), globalProp->getName());
    else
      return builder.createTryStoreGlobalPropertyInst(storedValue, globalProp);
  } else {
    llvm_unreachable("unvalid value to load from");
  }
}

/// Performs lowering of the JSON ESTree down to Hermes IR.
class ESTreeIRGen {
 public:
  /// Scoped hash table to represent the JS scope.
  using NameTableTy = hermes::ScopedHashTable<Identifier, Value *>;
  using NameTableScopeTy = hermes::ScopedHashTableScope<Identifier, Value *>;

  using BasicBlockListType = llvm::SmallVector<BasicBlock *, 4>;

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

    /// Initialize a new function context, while preserving the previous one.
    /// \param irGen the associated ESTreeIRGen object.
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
    /// \p
    /// hint appears in the name.
    Identifier genAnonymousLabelName(StringRef const hint);
  };

  /// This is a utility class that's related to the ES5 Reference type without
  /// the runtime semantic parts - https://es5.github.io/#x8.7.
  class LReference {
    /// The base of the object, or the variable we load from.
    Value *base_;
    /// The name/value of the field this reference accesses, or null if this is
    /// a
    /// variable access.
    Value *property_;
    /// Debug position for loads.
    SMLoc loadLoc_;

   public:
    LReference(Value *base, Value *property, SMLoc loadLoc)
        : base_(base), property_(property), loadLoc_(loadLoc) {}

    Value *emitLoad(IRBuilder &builder) {
      IRBuilder::ScopedLocationChange slc(builder, loadLoc_);

      if (property_)
        return builder.createLoadPropertyInst(base_, property_);
      else
        return ::emitLoad(builder, base_);
    }

    void emitStore(IRBuilder &builder, Value *value) {
      if (property_)
        builder.createStorePropertyInst(value, base_, property_);
      else
        ::emitStore(builder, value, base_);
    }

    Variable *castAsVariable() const {
      if (property_) {
        return nullptr;
      }
      return dyn_cast<Variable>(base_);
    }
    GlobalObjectProperty *castAsGlobalObjectProperty() const {
      if (property_) {
        return nullptr;
      }
      return dyn_cast<GlobalObjectProperty>(base_);
    }
  };

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
  FunctionContext *functionContext{};
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
  Identifier genAnonymousLabelName(StringRef hint);

 private:
  Identifier genGotoLabelName(StringRef hint, StringRef suffix);

  // "Converts" a ScopeChain into a SerializedScope by resolving the
  // identifiers.
  std::shared_ptr<SerializedScope> resolveScopeIdentifiers(
      const ScopeChain &chain) {
    std::shared_ptr<SerializedScope> current{};
    for (auto it = chain.functions.rbegin(), end = chain.functions.rend();
         it < end;
         it++) {
      auto next = std::make_shared<SerializedScope>();
      next->variables.reserve(it->variables.size());
      for (auto var : it->variables) {
        next->variables.push_back(std::move(Builder.createIdentifier(var)));
      }
      next->parentScope = current;
      current = next;
    }
    return current;
  }

  /// Materialize the provided scope.
  void materializeScopesInChain(
      Function *wrapperFunction,
      const std::shared_ptr<const SerializedScope> &scope,
      int depth);

 public:
  explicit ESTreeIRGen(
      ESTree::Node *root,
      const DeclarationFileListTy &declFileList,
      Module *M,
      const ScopeChain &scopeChain)
      : Mod(M),
        Builder(Mod),
        Root(root),
        DeclarationFileList(declFileList),
        lexicalScopeChain(resolveScopeIdentifiers(scopeChain)),
        identEval_(Builder.createIdentifier("eval")) {}

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

  /// Generate a body for a dummy function so that it doesn't crash the
  /// backend when encountered.
  void genDummyFunction(Function *dummy);

  /// \brief Perform IRGen for \p body, which can be the body of a function or
  /// the top-level of a program.
  void genBody(ESTree::NodeList &Body);

  /// Generate IR for function declarations;
  void genFunctionDeclaration(ESTree::FunctionDeclarationNode *func);

  /// Generate IR for function-like nodes (function declaration, expression,
  /// method, etc).
  /// \param originalName is the original non-unique name specified by the user
  ///   or inferred according to the rules of ES6.
  /// \param lazyClosureAlias an optional variable in the parent that will
  ///   contain the closure being created. It is non-null only if an alias
  ///   binding from  \c originalName to the variable was created and is
  ///   available inside the closure. Used only by lazy compilation.
  /// \param functionNode is the ESTree function node (declaration, expression,
  ///   object method).
  /// \param params are the formal parameters and \p body is the body of the
  ///   closure.
  /// \param lazy Whether or not to compile it lazily.
  /// \returns a new Function.
  Function *genFunctionLike(
      Identifier originalName,
      Variable *lazyClosureAlias,
      bool strictMode,
      ESTree::FunctionLikeNode *functionNode,
      const ESTree::NodeList &params,
      ESTree::Node *body,
      bool lazy);

  /// A lower level version of `genFunctionLike` which takes an already created
  /// Function and a function context. It offloads the generation of the
  /// actual body to a callback, which gives it additional flexibility to
  /// accomodate generating a closure for the top level function.
  ///
  /// \param NewFunc the just created new Function in a new FunctionContext.
  /// \param functionNode is the ESTree function node (declaration, expression,
  ///   object method).
  /// \param params the parameter list
  /// \param body the body ESTree element
  /// \param genBodyCB a callback to generate IR for the body
  void doGenFunctionLike(
      Function *NewFunc,
      ESTree::FunctionLikeNode *functionNode,
      const ESTree::NodeList &params,
      ESTree::Node *body,
      const std::function<void(ESTree::Node *body)> &genBodyCB);

  /// Declare a variable or a global propery depending in function \p inFunc,
  /// depending on whether it is the global scope. Do nothing if the variable
  /// or property is already declared in that scope.
  /// \return A pair. pair.first is the variable, and pair.second is set to true
  ///   if it was declared, false if it already existed.
  std::pair<Value *, bool> declareVariableOrGlobalProperty(
      Function *inFunc,
      Identifier name);

  /// Generate IR for IfStmt
  /// https://github.com/estree/estree/blob/master/spec.md#ifstatement
  void genIfStatement(ESTree::IfStatementNode *IfStmt);

  /// Generate IR for ForIn statements.
  void genForInStatement(ESTree::ForInStatementNode *ForInStmt);

  /// Generate IR for the return statement.
  /// https://github.com/estree/estree/blob/master/spec.md#returnstatement
  void genReturnStatement(ESTree::ReturnStatementNode *RetStmt);

  /// Generate IR for try/catch/finally statement.
  void genTry(ESTree::TryStatementNode *tryStmt);

  /// Generate IR "catch (e)".
  CatchInst *prepareCatch(ESTree::NodePtr catchParam);

  /// Generate IR for the logical expression \p logical.
  Value *genLogicalExpression(ESTree::LogicalExpressionNode *logical);

  /// Generate IR for the logical expression \p logical and jump on the
  /// corresponding label depending on its value.
  void genLogicalExpressionBranch(
      ESTree::LogicalExpressionNode *logical,
      BasicBlock *onTrue,
      BasicBlock *onFalse);

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

  /// Generate IR for the UpdateExpression node.
  /// https://github.com/estree/estree/blob/master/spec.md#updateexpression
  Value *genUpdateExpr(ESTree::UpdateExpressionNode *updateExpr);

  /// Generate code for the statement \p Stmt.
  void genStatement(ESTree::Node *Stmt);

  /// Generate the IR for the property field of the MemberExpressionNode.
  /// The property field may be a string literal or some computed expression.
  Value *genMemberExpressionProperty(ESTree::MemberExpressionNode *Mem);

  /// Generate IR for the expression \p Expr.
  /// \p nameHint is used to provide names for anonymous functions.
  /// We currently provide names for functions that are assigned to a variable,
  /// or functions that are assigned to an object key. These are a subset of
  /// ES6, but not all of it.
  Value *genExpression(ESTree::Node *Expr, Identifier nameHint = Identifier{});

  /// Generate an expression and perform a conditional branch depending on
  /// whether the expression evaluates to true or false.
  void genExpressionBranch(
      ESTree::Node *expr,
      BasicBlock *onTrue,
      BasicBlock *onFalse);

  /// Generate IR for the identifier expression.
  /// We need to know whether it's after typeof (\p afterTypeOf),
  /// to help decide whether a load can throw.
  Value *genIdentifierExpression(
      ESTree::IdentifierNode *Iden,
      bool afterTypeOf);

  /// Wrapper of genExpression. If functionContext->globalReturnRegister is
  /// set, stores the expression value into it.
  void genExpressionWrapper(ESTree::Node *expr);

  /// Convert a property key node to its JavaScript string representation.
  StringRef propertyKeyAsString(
      llvm::SmallVectorImpl<char> &storage,
      ESTree::Node *Key);

  /// Generate IR for the property key in \p Key.
  Value *genPropertyKey(ESTree::Node *Key);

  /// Generate IR for the object expression \p Expr.
  Value *genObjectExpr(ESTree::ObjectExpressionNode *Expr);

  /// Generate IR for the array expression \p Expr.
  Value *genArrayExpr(ESTree::ArrayExpressionNode *Expr);

  /// Generate IR for the call expression.
  Value *genCallExpr(ESTree::CallExpressionNode *call);

  /// Generate IR for a direct call to eval(). This is invoked from
  /// genCallExpr().
  Value *genCallEvalExpr(ESTree::CallExpressionNode *call);

  /// Check if all cases values are constant and stores them in
  /// \p casdeLiterals. The default case, if present, is stored as nullptr, to
  /// maintain the ordering of the cases.
  /// \returns true if all cases are constant.
  bool areAllCasesConstant(
      ESTree::SwitchStatementNode *switchStmt,
      llvm::SmallVectorImpl<Literal *> &caseLiterals);

  /// Generate IR Switch statements.
  void genSwitchStmt(ESTree::SwitchStatementNode *switchStmt);

  /// Generate IR Switch statements when all case tests are constant.
  void genConstSwitchStmt(
      ESTree::SwitchStatementNode *switchStmt,
      llvm::SmallVectorImpl<Literal *> &caseLiterals);

  /// Generate IR for the new expression.
  Value *genNewExpr(ESTree::NewExpressionNode *N);

  /// Generate the AssignmentExpression expression.
  Value *genAssignmentExpr(ESTree::AssignmentExpressionNode *AE);

  /// Generate the ternary (conditional) operator.
  Value *genConditionalExpr(ESTree::ConditionalExpressionNode *C);

  /// Generate the sequence expression.
  Value *genSequenceExpr(ESTree::SequenceExpressionNode *Sq);

  /// Generate all unary operators.
  Value *genUnaryExpression(ESTree::UnaryExpressionNode *U);

  /// Generate instructions for instanceof operator.
  Value *genInstanceOf(Value *left, Value *right);

  /// When we see a control change such as return, break, continue,
  /// we need to make sure to generate code for finally block if
  /// we are under a try/catch.
  /// \param sourceTry  the try statement surrounding the AST node performing
  ///   the goto (break, continue, return)
  /// \param targetTry  the try statement surrounding the AST node of the label.
  void genFinallyBeforeControlChange(
      ESTree::TryStatementNode *sourceTry,
      ESTree::TryStatementNode *targetTry);

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

  /// Generates a left hand side reference from valid estree nodes
  /// that can be translated to lref.
  LReference createLRef(ESTree::Node *node);

  /// Save all variables currently in scope, for lazy compilation.
  std::shared_ptr<SerializedScope> saveCurrentScope();
};

ESTreeIRGen::FunctionContext::FunctionContext(
    ESTreeIRGen *irGen,
    Function *function,
    sem::FunctionInfo *semInfo)
    : irGen_(irGen),
      semInfo_(semInfo),
      oldContext_(irGen->functionContext),
      builderSaveState_(irGen->Builder),
      function(function),
      scope(irGen->nameTable_) {
  irGen->functionContext = this;

  if (semInfo_) {
    // Allocate the label table. Each label definition will be encountered in
    // the AST before it is referenced (because of the nature of JavaScript), at
    // which point we will initialize the GotoLabel structure with basic blocks
    // targets.
    labels.resize(semInfo_->labels.size());
  }
}

ESTreeIRGen::FunctionContext::~FunctionContext() {
  irGen_->functionContext = oldContext_;
}

Identifier ESTreeIRGen::FunctionContext::genAnonymousLabelName(StringRef hint) {
  llvm::SmallString<16> buf;
  llvm::raw_svector_ostream nameBuilder{buf};
  nameBuilder << "?anon_" << anonymousLabelCounter++ << "_" << hint;
  return function->getContext().getIdentifier(nameBuilder.str());
}

Identifier ESTreeIRGen::genAnonymousLabelName(StringRef hint) {
  return functionContext->genAnonymousLabelName(hint);
}

Identifier ESTreeIRGen::genGotoLabelName(StringRef hint, StringRef suffix) {
  llvm::SmallString<32> str;
  str.append(hint);
  str.append(suffix);
  return Builder.createIdentifier(str);
}

void ESTreeIRGen::materializeScopesInChain(
    Function *wrapperFunction,
    const std::shared_ptr<const SerializedScope> &scope,
    int depth) {
  if (!scope)
    return;
  assert(depth < 1000 && "Excessive scope depth");

  // First materialize parent scopes.
  materializeScopesInChain(wrapperFunction, scope->parentScope, depth + 1);

  // If scope->closureAlias is specified, we must create an alias binding
  // between originalName (which must be valid) and the variable identified by
  // closureAlias.
  //
  // We do this *before* inserting the other variables below to reflect that
  // the closure alias is conceptually in an outside scope and also avoid the
  // closure name incorrectly shadowing the same name inside the closure.
  if (scope->closureAlias.isValid()) {
    assert(scope->originalName.isValid() && "Original name invalid");
    assert(
        scope->originalName != scope->closureAlias &&
        "Original name must be different from the alias");

    // NOTE: the closureAlias target must exist and must be a Variable.
    auto *closureVar = cast<Variable>(nameTable_.lookup(scope->closureAlias));

    // Re-create the alias.
    nameTable_.insert(scope->originalName, closureVar);
  }

  // Create an external scope.
  ExternalScope *ES = Builder.createExternalScope(wrapperFunction, -depth);
  for (auto variableId : scope->variables) {
    auto *variable = Builder.createVariable(ES, variableId);
    nameTable_.insert(variableId, variable);
  }
}

void ESTreeIRGen::doIt() {
  DEBUG(dbgs() << "Processing top level program.\n");

  ESTree::ProgramNode *Program;

  if (auto File = dyn_cast<ESTree::FileNode>(Root)) {
    DEBUG(dbgs() << "Found File decl.\n");
    Program = dyn_cast<ESTree::ProgramNode>(File->_program);
  } else {
    Program = dyn_cast<ESTree::ProgramNode>(Root);
  }

  if (!Program) {
    Builder.getModule()->getContext().getSourceErrorManager().error(
        SMLoc{}, "missing 'Program' AST node");
    return;
  }

  DEBUG(dbgs() << "Found Program decl.\n");

  // The function which will "execute" the module.
  Function *topLevelFunction;

  // Function context used only when compiling in an existing lexical scope
  // chain. It is only initialized if we have a lexical scope chain.
  llvm::Optional<FunctionContext> wrapperFunctionContext{};

  if (!lexicalScopeChain) {
    topLevelFunction = Builder.createTopLevelFunction(
        ESTree::isStrict(Program->strictness), Program->getSourceRange());
  } else {
    // If compiling in an existing lexical context, we need to install the
    // scopes in a wrapper function, which represents the "global" code.

    Function *wrapperFunction = Builder.createFunction(
        "",
        ESTree::isStrict(Program->strictness),
        Program->getSourceRange(),
        true);

    // Initialize the wrapper context.
    wrapperFunctionContext.emplace(this, wrapperFunction, nullptr);

    // Populate it with dummy code so it doesn't crash the back-end.
    genDummyFunction(wrapperFunction);

    // Restore the previously saved parent scopes.
    materializeScopesInChain(wrapperFunction, lexicalScopeChain, 1);

    // Finally create the function which will actually be executed.
    topLevelFunction = Builder.createFunction(
        "eval",
        ESTree::isStrict(Program->strictness),
        Program->getSourceRange(),
        false);
  }

  Mod->setTopLelevFunction(topLevelFunction);

  // Function context for topLevelFunction.
  FunctionContext topLevelFunctionContext{
      this, topLevelFunction, Program->getSemInfo()};

  // IRGen needs a pointer to the outer-most context, which is either
  // topLevelContext or wrapperFunctionContext, depending on whether the latter
  // was created.
  // We want to set the pointer to that outer-most context, but ensure that it
  // doesn't outlive the context it is pointing to.
  llvm::SaveAndRestore<FunctionContext *> saveTopLevelContext(
      topLevelContext,
      !wrapperFunctionContext.hasValue() ? &topLevelFunctionContext
                                         : &wrapperFunctionContext.getValue());

  // Now declare all externally supplied global properties, but only if we don't
  // have a lexical scope chain.
  if (!lexicalScopeChain) {
    for (auto declFile : DeclarationFileList) {
      processDeclarationFile(declFile);
    }
  }

  doGenFunctionLike(
      functionContext->function,
      Program,
      ESTree::NodeList{},
      Program,
      [this](ESTree::Node *body) {
        // Allocate the return register, initialize it to undefined.
        functionContext->globalReturnRegister =
            Builder.createAllocStackInst(genAnonymousLabelName("ret"));
        Builder.createStoreStackInst(
            Builder.getLiteralUndefined(),
            functionContext->globalReturnRegister);

        genBody(cast<ESTree::ProgramNode>(body)->_body);

        // Terminate the top-level scope with a return statement.
        auto ret_val =
            Builder.createLoadStackInst(functionContext->globalReturnRegister);
        Builder.createReturnInst(ret_val);
      });
}

void ESTreeIRGen::doCJSModule(
    Function *topLevelFunction,
    sem::FunctionInfo *semInfo,
    llvm::StringRef filename) {
  assert(Root && "no root in ESTreeIRGen");
  auto *func = cast<ESTree::FunctionExpressionNode>(Root);
  assert(func && "doCJSModule without a module");

  FunctionContext topLevelFunctionContext{this, topLevelFunction, semInfo};
  llvm::SaveAndRestore<FunctionContext *> saveTopLevelContext(
      topLevelContext, &topLevelFunctionContext);

  Identifier functionName = Builder.createIdentifier("cjs_module");
  Function *newFunc = genFunctionLike(
      functionName,
      nullptr,
      ESTree::isStrict(func->strictness),
      func,
      func->_params,
      func->_body,
      Mod->getContext().isLazyCompilation());

  Builder.getModule()->addCJSModule(
      Builder.createIdentifier(filename), newFunc);
}

Function *ESTreeIRGen::doLazyFunction(hbc::LazyCompilationData *lazyData) {
  // Create a dummy top level function so IRGen doesn't think our lazyFunction
  // is in global scope.
  Function *topLevel = Builder.createTopLevelFunction(lazyData->strictMode, {});
  genDummyFunction(topLevel);

  FunctionContext topLevelFunctionContext{this, topLevel, nullptr};

  // Save the top-level context, but ensure it doesn't outlive what it is
  // pointing to.
  llvm::SaveAndRestore<FunctionContext *> saveTopLevelContext(
      topLevelContext, &topLevelFunctionContext);

  auto *node = cast<ESTree::FunctionLikeNode>(Root);

  // Restore the previously saved parent scopes.
  lexicalScopeChain = lazyData->parentScope;
  materializeScopesInChain(topLevel, lexicalScopeChain, 1);

  // If lazyData->closureAlias is specified, we must create an alias binding
  // between originalName (which must be valid) and the variable identified by
  // closureAlias.
  Variable *parentVar = nullptr;
  if (lazyData->closureAlias.isValid()) {
    assert(lazyData->originalName.isValid() && "Original name invalid");
    assert(
        lazyData->originalName != lazyData->closureAlias &&
        "Original name must be different from the alias");

    // NOTE: the closureAlias target must exist and must be a Variable.
    parentVar = cast<Variable>(nameTable_.lookup(lazyData->closureAlias));

    // Re-create the alias.
    nameTable_.insert(lazyData->originalName, parentVar);
  }

  ESTree::NodeList const *params;
  ESTree::NodePtr body;

  if (auto *FE = dyn_cast<ESTree::FunctionExpressionNode>(node)) {
    params = &FE->_params;
    body = FE->_body;
  } else if (auto *FD = dyn_cast<ESTree::FunctionDeclarationNode>(node)) {
    params = &FD->_params;
    body = FD->_body;
  } else if (auto *OM = dyn_cast<ESTree::ObjectMethodNode>(node)) {
    params = &OM->_params;
    body = OM->_body;
  } else {
    llvm_unreachable("invalid lazy function AST node");
  }

  return genFunctionLike(
      lazyData->originalName,
      parentVar,
      lazyData->strictMode,
      node,
      *params,
      body,
      false);
}

void ESTreeIRGen::genDummyFunction(Function *dummy) {
  IRBuilder::SaveRestore saveState{Builder};

  Builder.createParameter(dummy, "this");
  BasicBlock *firstBlock = Builder.createBasicBlock(dummy);
  Builder.setInsertionBlock(firstBlock);
  Builder.createUnreachableInst();
  Builder.createReturnInst(Builder.getLiteralUndefined());
}

void ESTreeIRGen::genBody(ESTree::NodeList &Body) {
  DEBUG(dbgs() << "Compiling body.\n");

  // Generate code for the declarations statements.
  for (auto &Node : Body) {
    DEBUG(dbgs() << "IRGen node of type " << Node.getNodeName() << ".\n");
    genStatement(&Node);
  }
}

void ESTreeIRGen::genFunctionDeclaration(
    ESTree::FunctionDeclarationNode *func) {
  // Find the name of the function.
  Identifier functionName = getNameFieldFromID(func->_id);
  DEBUG(dbgs() << "IRGen function \"" << functionName << "\".\n");

  auto *funcStorage = nameTable_.lookup(functionName);
  assert(
      funcStorage && "function declaration variable should have been hoisted");

  Function *newFunc = genFunctionLike(
      functionName,
      nullptr,
      ESTree::isStrict(func->strictness),
      func,
      func->_params,
      func->_body,
      Mod->getContext().isLazyCompilation());

  // Store the newly created closure into a frame variable with the same name.
  auto *newClosure = Builder.createCreateFunctionInst(newFunc);

  emitStore(Builder, newClosure, funcStorage);
}

#ifndef HERMESVM_LEAN
std::shared_ptr<SerializedScope> ESTreeIRGen::saveCurrentScope() {
  auto *func = functionContext->function;
  assert(func && "Missing function when saving scope");

  auto scope = std::make_shared<SerializedScope>();

  // We currently only lazy compile a single level at a time. If we later start
  // compiling multiple, this method would need to walk the scopes.
  assert(
      ((func->isGlobalScope() && !functionContext->getPreviousContext()) ||
       (!func->isGlobalScope() && functionContext->getPreviousContext() &&
        !functionContext->getPreviousContext()->getPreviousContext())) &&
      "Expected exactly one function on the stack.");

  scope->parentScope = lexicalScopeChain;
  scope->originalName = func->getOriginalOrInferredName();
  if (auto *closure = func->getLazyClosureAlias()) {
    scope->closureAlias = closure->getName();
  }
  for (auto *var : func->getFunctionScope()->getVariables()) {
    scope->variables.push_back(var->getName());
  }
  return scope;
}

Function *ESTreeIRGen::genFunctionLike(
    Identifier originalName,
    Variable *lazyClosureAlias,
    bool strictMode,
    ESTree::FunctionLikeNode *functionNode,
    const ESTree::NodeList &params,
    ESTree::Node *body,
    bool lazy) {
  assert(functionNode && "Function AST cannot be null");

  auto *newFunction =
      Builder.createFunction(originalName, strictMode, body->getSourceRange());
  newFunction->setLazyClosureAlias(lazyClosureAlias);

  auto *bodyBlock = cast<ESTree::BlockStatementNode>(body);
  if (bodyBlock->isLazyFunctionBody) {
    // Set the AST position and variable context so we can continue later.
    newFunction->setLazyScope(saveCurrentScope());
    auto &lazySource = newFunction->getLazySource();
    lazySource.bufferId = bodyBlock->bufferId;
    lazySource.nodeKind = functionNode->getKind();
    lazySource.functionRange = functionNode->getSourceRange();

    // Give the stub parameters so that we'll know the function's .length .
    Builder.createParameter(newFunction, "this");
    for (auto &param : params) {
      auto idenNode = cast<ESTree::IdentifierNode>(&param);
      Identifier paramName = getNameFieldFromID(idenNode);
      Builder.createParameter(newFunction, paramName);
    }

    return newFunction;
  }

  FunctionContext newFunctionContext{
      this, newFunction, functionNode->getSemInfo()};

  doGenFunctionLike(
      newFunctionContext.function,
      functionNode,
      params,
      body,
      [this](ESTree::Node *body) {
        DEBUG(dbgs() << "IRGen function body.\n");
        // irgen the rest of the body.
        genStatement(body);

        Builder.setLocation(Builder.getFunction()->getSourceRange().End);
        Builder.createReturnInst(Builder.getLiteralUndefined());
      });

  return newFunctionContext.function;
}
#endif

std::pair<Value *, bool> ESTreeIRGen::declareVariableOrGlobalProperty(
    Function *inFunc,
    Identifier name) {
  Value *found = nameTable_.lookup(name);

  // If the variable is already declared in this scope, do not create a
  // second instance.
  if (found) {
    if (auto *var = dyn_cast<Variable>(found)) {
      if (var->getParent()->getFunction() == inFunc)
        return {found, false};
    } else {
      assert(
          isa<GlobalObjectProperty>(found) &&
          "Invalid value found in name table");
      if (inFunc->isGlobalScope())
        return {found, false};
    }
  }

  // Create a property if global scope, variable otherwise.
  Value *var;
  if (inFunc->isGlobalScope()) {
    var = Builder.createGlobalObjectProperty(name, true);
  } else {
    var = Builder.createVariable(inFunc->getFunctionScope(), name);
  }

  // Register the variable in the scoped hash table.
  nameTable_.insert(name, var);
  return {var, true};
}

void ESTreeIRGen::doGenFunctionLike(
    Function *NewFunc,
    ESTree::FunctionLikeNode *functionNode,
    const ESTree::NodeList &params,
    ESTree::Node *body,
    const std::function<void(ESTree::Node *body)> &genBodyCB) {
  auto *semInfo = functionContext->getSemInfo();
  DEBUG(
      dbgs() << "Hoisting "
             << (semInfo->decls.size() + semInfo->closures.size())
             << " variable decls.\n");

  Builder.setLocation(NewFunc->getSourceRange().Start);

  // Generate the code for closure:
  {
    // Create a new function with the right name.
    auto Entry = Builder.createBasicBlock(NewFunc);

    // Start pumping instructions into the entry basic block.
    Builder.setInsertionBlock(Entry);

    // Create variable declarations for each of the hoisted variables and
    // functions. Initialize only the variables to undefined.
    for (auto *vd : semInfo->decls) {
      auto res =
          declareVariableOrGlobalProperty(NewFunc, getNameFieldFromID(vd->_id));
      // If this is not a frame variable or it was already declared, skip.
      auto *var = dyn_cast<Variable>(res.first);
      if (!var || !res.second)
        continue;

      // Otherwise, initialize it to undefined.
      Builder.createStoreFrameInst(Builder.getLiteralUndefined(), var);
    }
    for (auto *fd : semInfo->closures)
      declareVariableOrGlobalProperty(NewFunc, getNameFieldFromID(fd->_id));

    // Construct the parameter list. Create function parameters and register
    // them in the scope.
    DEBUG(dbgs() << "IRGen function parameters.\n");
    // Always create the "this" parameter.
    Builder.createParameter(NewFunc, "this");
    for (auto &param : params) {
      auto idenNode = cast<ESTree::IdentifierNode>(&param);
      Identifier paramName = getNameFieldFromID(idenNode);
      DEBUG(dbgs() << "Adding parameter: " << paramName << "\n");

      auto *P = Builder.createParameter(NewFunc, paramName);
      auto *ParamStorage =
          Builder.createVariable(NewFunc->getFunctionScope(), paramName);

      // Register the storage for the parameter.
      nameTable_.insert(paramName, ParamStorage);

      // Store the parameter into the local scope.
      emitStore(Builder, P, ParamStorage);
    }

    // Generate and initialize the code for the hoisted function declarations
    // before generating the rest of the body.
    for (auto funcDecl : semInfo->closures) {
      genFunctionDeclaration(funcDecl);
    }

    // Separate the next block, so we can append instructions to the entry block
    // in the future.
    auto nextBlock = Builder.createBasicBlock(NewFunc);
    functionContext->entryTerminator = Builder.createBranchInst(nextBlock);
    Builder.setInsertionBlock(nextBlock);

    genBodyCB(body);

    // If Entry is the only user of nextBlock, merge Entry and nextBlock, to
    // create less "noise" when optimization is disabled.
    if (nextBlock->getNumUsers() == 1 &&
        nextBlock->hasUser(functionContext->entryTerminator)) {
      DEBUG(dbgs() << "Merging entry and nextBlock.\n");

      // Move all instructions from nextBlock into Entry.
      while (nextBlock->begin() != nextBlock->end())
        nextBlock->begin()->moveBefore(functionContext->entryTerminator);

      // Now we can delete the original terminator;
      functionContext->entryTerminator->eraseFromParent();
      functionContext->entryTerminator = nullptr;

      // Delete the now empty next block
      nextBlock->eraseFromParent();
      nextBlock = nullptr;
    } else {
      DEBUG(dbgs() << "Could not merge entry and nextBlock.\n");
    }
  }

  NewFunc->clearStatementCount();
}

void ESTreeIRGen::genIfStatement(ESTree::IfStatementNode *IfStmt) {
  DEBUG(dbgs() << "IRGen IF-stmt.\n");

  auto Parent = Builder.getInsertionBlock()->getParent();
  auto ThenBlock = Builder.createBasicBlock(Parent);
  auto ElseBlock = Builder.createBasicBlock(Parent);
  auto ContinueBlock = Builder.createBasicBlock(Parent);

  genExpressionBranch(IfStmt->_test, ThenBlock, ElseBlock);

  // IRGen the Then:
  Builder.setInsertionBlock(ThenBlock);
  genStatement(IfStmt->_consequent);
  Builder.createBranchInst(ContinueBlock);

  // IRGen the Else, if it exists:
  Builder.setInsertionBlock(ElseBlock);
  if (IfStmt->_alternate) {
    genStatement(IfStmt->_alternate);
  }

  Builder.createBranchInst(ContinueBlock);
  Builder.setInsertionBlock(ContinueBlock);
}

void ESTreeIRGen::genForInStatement(ESTree::ForInStatementNode *ForInStmt) {
  // The state of the enumerator. Notice that the instruction writes to the
  // storage
  // variables just like Load/Store instructions write to stack allocations.
  auto *iteratorStorage =
      Builder.createAllocStackInst(genAnonymousLabelName("iter"));
  auto *baseStorage =
      Builder.createAllocStackInst(genAnonymousLabelName("base"));
  auto *indexStorage =
      Builder.createAllocStackInst(genAnonymousLabelName("idx"));
  auto *sizeStorage =
      Builder.createAllocStackInst(genAnonymousLabelName("size"));

  // Generate the right hand side of the for-in loop. The result of this
  // expression is the object we iterate on. We use this object as the 'base'
  // of the enumerator.
  Value *object = genExpression(ForInStmt->_right);
  Builder.createStoreStackInst(object, baseStorage);

  // The storage for the property name that the enumerator loads:
  auto *propertyStorage =
      Builder.createAllocStackInst(genAnonymousLabelName("prop"));

  /*
    We generate the following loop structure for the for-in loops:

        [ current block ]
        [   get_pname   ]
               |         \
               |          \
               v           \ (on empty object)
    /----> [get_next]       \
    |          |     \       \
    |          |      \       \
    |          |       \       \ ->[ exit block ]
    |          |        \      /
    |    [ body block ]  \____/
    |          |          (on last iteration)
    \__________/
  */

  auto parent = Builder.getInsertionBlock()->getParent();
  auto *exitBlock = Builder.createBasicBlock(parent);
  auto *getNextBlock = Builder.createBasicBlock(parent);
  auto *bodyBlock = Builder.createBasicBlock(parent);

  // Initialize the goto labels.
  auto &label = functionContext->labels[ForInStmt->getLabelIndex()];
  label.breakTarget = exitBlock;
  label.continueTarget = getNextBlock;

  // Create the enumerator:
  Builder.createGetPNamesInst(
      iteratorStorage,
      baseStorage,
      indexStorage,
      sizeStorage,
      exitBlock,
      getNextBlock);

  // Generate the get_next part of the loop:
  Builder.setInsertionBlock(getNextBlock);
  Builder.createGetNextPNameInst(
      propertyStorage,
      baseStorage,
      indexStorage,
      sizeStorage,
      iteratorStorage,
      exitBlock,
      bodyBlock);

  // Emit the loop body and setup the property variable. When done jump into the
  // 'get_next' block and try to do another iteration.
  Builder.setInsertionBlock(bodyBlock);

  // The string property value of the current iteration is saved into this
  // variable.
  auto propertyStringRepr = Builder.createLoadStackInst(propertyStorage);

  // The left hand side of For-In statements can be any lhs expression
  // ("PutValue"). Example:
  //  1. for (x.y in [1,2,3])
  //  2. for (x in [1,2,3])
  //  3. for (var x in [1,2,3])
  // See ES5 $12.6.4 "The for-in Statement"
  LReference lref = createLRef(ForInStmt->_left);
  lref.emitStore(Builder, propertyStringRepr);

  genStatement(ForInStmt->_body);

  Builder.createBranchInst(getNextBlock);

  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genReturnStatement(ESTree::ReturnStatementNode *RetStmt) {
  DEBUG(dbgs() << "IRGen Return-stmt.\n");

  Value *Value;
  // Generate IR for the return value, or undefined if this is an empty return
  // statement.
  if (auto *A = RetStmt->_argument) {
    Value = genExpression(A);
  } else {
    Value = Builder.getLiteralUndefined();
  }

  genFinallyBeforeControlChange(RetStmt->surroundingTry, nullptr);
  Builder.createReturnInst(Value);

  // Code that comes after 'return' is dead code. Let's create a new un-linked
  // basic block and keep IRGen in that block. The optimizer will clean things
  // up.
  auto Parent = Builder.getInsertionBlock()->getParent();
  Builder.setInsertionBlock(Builder.createBasicBlock(Parent));
}

void ESTreeIRGen::genTry(ESTree::TryStatementNode *tryStmt) {
  DEBUG(dbgs() << "IRGen 'try' statement\n");
  auto *parent = Builder.getInsertionBlock()->getParent();

  // try-catch-finally statements must have been transformed by the validator
  // into two nested try statements with only "catch" or "finally" each.
  assert(
      (!tryStmt->_handler || !tryStmt->_finalizer) &&
      "Try statement can't have both catch and finally");

  auto *catchBlock = Builder.createBasicBlock(parent);
  auto *continueBlock = Builder.createBasicBlock(parent);
  auto *tryBodyBlock = Builder.createBasicBlock(parent);

  // Start with a TryStartInst, and transition to try body.
  Builder.createTryStartInst(tryBodyBlock, catchBlock);
  Builder.setInsertionBlock(tryBodyBlock);

  // Generate IR for the body of Try
  genStatement(tryStmt->_block);

  // Emit TryEnd in a new block.
  Builder.setLocation(SourceErrorManager::convertEndToLocation(
      tryStmt->_block->getSourceRange()));
  auto *tryEndBlock = Builder.createBasicBlock(parent);
  Builder.createBranchInst(tryEndBlock);
  Builder.setInsertionBlock(tryEndBlock);
  Builder.createTryEndInst();

  if (tryStmt->_finalizer) {
    genStatement(tryStmt->_finalizer);
    Builder.setLocation(SourceErrorManager::convertEndToLocation(
        tryStmt->_finalizer->getSourceRange()));
  }

  Builder.createBranchInst(continueBlock);

  // Generate the catch/finally block.
  Builder.setInsertionBlock(catchBlock);

  // If we have a catch block.
  if (tryStmt->_handler) {
    auto *catchClauseNode =
        dyn_cast<ESTree::CatchClauseNode>(tryStmt->_handler);

    // Catch takes a exception variable, hence we need to create a new
    // scope for it.
    NameTableScopeTy newScope(nameTable_);

    Builder.setLocation(tryStmt->_handler->getDebugLoc());
    prepareCatch(catchClauseNode->_param);

    genStatement(catchClauseNode->_body);

    Builder.setLocation(SourceErrorManager::convertEndToLocation(
        tryStmt->_handler->getSourceRange()));
    Builder.createBranchInst(continueBlock);
  } else {
    // A finally block catches the exception and rethrows is.
    Builder.setLocation(tryStmt->_finalizer->getDebugLoc());
    auto *catchReg = Builder.createCatchInst();

    genStatement(tryStmt->_finalizer);

    Builder.setLocation(SourceErrorManager::convertEndToLocation(
        tryStmt->_finalizer->getSourceRange()));
    Builder.createThrowInst(catchReg);
  }

  // Finally transition to continue block.
  Builder.setInsertionBlock(continueBlock);
}

CatchInst *ESTreeIRGen::prepareCatch(ESTree::NodePtr catchParam) {
  auto catchInst = Builder.createCatchInst();

  auto catchVariableName =
      getNameFieldFromID(dyn_cast<ESTree::IdentifierNode>(catchParam));

  // Generate a unique catch variable name and use this name for IRGen purpose
  // only. The variable lookup in the catch clause will continue to be done
  // using the declared name.
  auto uniquedCatchVariableName =
      genAnonymousLabelName(catchVariableName.str());

  auto errorVar = Builder.createVariable(
      functionContext->function->getFunctionScope(), uniquedCatchVariableName);

  /// Insert the synthesized variable into the function name table, so it can
  /// be looked up internally.
  nameTable_.insertIntoScope(
      &functionContext->scope, errorVar->getName(), errorVar);

  // Alias the lexical name to the synthesized variable.
  nameTable_.insert(catchVariableName, errorVar);

  ::emitStore(Builder, catchInst, errorVar);
  return catchInst;
}

Value *ESTreeIRGen::genLogicalExpression(
    ESTree::LogicalExpressionNode *logical) {
  auto opStr = logical->_operator->str();
  DEBUG(dbgs() << "IRGen of short circuiting: " << opStr << ".\n");

  // True if the operand is And (&&) or False if the operand is Or (||).
  bool isAnd = false;

  if (opStr == "&&") {
    isAnd = true;
  } else if (opStr == "||") {
    isAnd = false;
  } else {
    llvm_unreachable("Invalid update operator");
  }

  // Generate a new temporary stack allocation.
  auto tempVarName = genAnonymousLabelName("logical");
  auto parentFunc = Builder.getInsertionBlock()->getParent();
  auto tempVar = Builder.createAllocStackInst(tempVarName);

  auto evalRHSBlock = Builder.createBasicBlock(parentFunc);
  auto continueBlock = Builder.createBasicBlock(parentFunc);

  auto LHS = genExpression(logical->_left);

  // Store the LHS value of the expression in preparation for the case where we
  // won't need to evaluate the RHS side of the expression.
  Builder.createStoreStackInst(LHS, tempVar);

  // Don't continue if the value is evaluated to true for '&&' or false for
  // '||'. Notice that instead of negating the condition we swap the operands of
  // the branch.
  BasicBlock *T = continueBlock;
  BasicBlock *F = evalRHSBlock;
  if (isAnd) {
    std::swap(T, F);
  }
  Builder.createCondBranchInst(LHS, T, F);

  // Continue the evaluation of the right-hand-side of the expression.
  Builder.setInsertionBlock(evalRHSBlock);
  auto RHS = genExpression(logical->_right);

  // Evaluate the RHS and store the result into the temporary variable.
  Builder.createStoreStackInst(RHS, tempVar);

  // Finally, jump to the continuation block.
  Builder.createBranchInst(continueBlock);

  // Load the content of the temp variable that was set in one of the branches.
  Builder.setInsertionBlock(continueBlock);
  return Builder.createLoadStackInst(tempVar);
}

void ESTreeIRGen::genLogicalExpressionBranch(
    ESTree::LogicalExpressionNode *logical,
    BasicBlock *onTrue,
    BasicBlock *onFalse) {
  auto opStr = logical->_operator->str();
  DEBUG(dbgs() << "IRGen of short circuiting: " << opStr << " branch.\n");

  auto parentFunc = Builder.getInsertionBlock()->getParent();
  auto block = Builder.createBasicBlock(parentFunc);

  if (opStr == "&&") {
    genExpressionBranch(logical->_left, block, onFalse);
  } else if (opStr == "||") {
    genExpressionBranch(logical->_left, onTrue, block);
  } else {
    llvm_unreachable("Invalid update operator");
  }

  Builder.setInsertionBlock(block);
  genExpressionBranch(logical->_right, onTrue, onFalse);
}

Value *ESTreeIRGen::genUpdateExpr(ESTree::UpdateExpressionNode *updateExpr) {
  DEBUG(dbgs() << "IRGen update expression.\n");
  bool isPrefix = updateExpr->_prefix;

  // The operands ++ and -- are equivalent to adding or subtracting the
  // literal 1.
  // See section 12.4.4.1.
  BinaryOperatorInst::OpKind opKind;
  if (updateExpr->_operator->str() == "++") {
    opKind = BinaryOperatorInst::OpKind::AddKind;
  } else if (updateExpr->_operator->str() == "--") {
    opKind = BinaryOperatorInst::OpKind::SubtractKind;
  } else {
    llvm_unreachable("Invalid update operator");
  }

  LReference lref = createLRef(updateExpr->_argument);

  // Load the original value.
  Value *original = lref.emitLoad(Builder);

  // Convert the original value to number. Even on suffix operators we return
  // the converted value.
  original = Builder.createAsNumberInst(original);

  // Create the +1 or -1.
  Value *result = Builder.createBinaryOperatorInst(
      original, Builder.getLiteralNumber(1), opKind);

  // Store the result.
  lref.emitStore(Builder, result);

  // Depending on the prefixness return the previous value or the modified
  // value.
  return (isPrefix ? result : original);
}

void ESTreeIRGen::genForWhileLoops(
    ESTree::LoopStatementNode *loop,
    ESTree::Node *init,
    ESTree::Node *preTest,
    ESTree::Node *postTest,
    ESTree::Node *update,
    ESTree::Node *body) {
  /* In this section we generate a sequence of basic blocks that implement
   the for, while and do..while statements. Loop inversion is applied.
   For loops are syntactic-sugar for while
   loops and both have pre-test and post-test. do..while loop should only
   have post-test. They will all have the following structure:

        [ current block  ]
        [      init      ]
        [ pre test block ]
               |       \
               |        \
               |         \      ->[ exit block ]
    /-->  [ body block ]  \____/      ^
    |    [ update block ]             |
    |   [ post test block ]  --------/
    \__________/
  */

  // Create the basic blocks that make the while structure.
  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *bodyBlock = Builder.createBasicBlock(function);
  BasicBlock *exitBlock = Builder.createBasicBlock(function);
  BasicBlock *preTestBlock = Builder.createBasicBlock(function);
  BasicBlock *postTestBlock = Builder.createBasicBlock(function);
  BasicBlock *updateBlock = Builder.createBasicBlock(function);

  // Initialize the goto labels.
  auto &label = functionContext->labels[loop->getLabelIndex()];
  label.breakTarget = exitBlock;
  label.continueTarget = updateBlock;

  // Generate IR for the loop initialization.
  // The init field can be a variable declaration or any expression.
  // https://github.com/estree/estree/blob/master/spec.md#forstatement
  if (init) {
    if (isa<ESTree::VariableDeclarationNode>(init)) {
      genStatement(init);
    } else {
      genExpression(init);
    }
  }

  // Terminate the loop header section and jump to the condition block.
  Builder.createBranchInst(preTestBlock);
  Builder.setInsertionBlock(preTestBlock);

  // Branch out of the loop if the condition is false.
  if (preTest)
    genExpressionBranch(preTest, bodyBlock, exitBlock);
  else
    Builder.createBranchInst(bodyBlock);

  // Generate the update sequence of 'for' loops.
  Builder.setInsertionBlock(updateBlock);
  if (update) {
    genExpression(update);
  }

  // After executing the content of the body, jump to the post test block.
  Builder.createBranchInst(postTestBlock);
  Builder.setInsertionBlock(postTestBlock);

  // Branch out of the loop if the condition is false.
  if (postTest)
    genExpressionBranch(postTest, bodyBlock, exitBlock);
  else
    Builder.createBranchInst(bodyBlock);

  // Now, generate the body of the while loop.
  // Do this last so that the test and update blocks are associated with the
  // loop statement, and not the body statement.
  Builder.setInsertionBlock(bodyBlock);
  genStatement(body);
  Builder.createBranchInst(updateBlock);

  // Following statements are inserted to the exit block.
  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genStatement(ESTree::Node *Stmt) {
  DEBUG(dbgs() << "IRGen statement of type " << Stmt->getNodeName() << "\n");
  IRBuilder::ScopedLocationChange slc(Builder, Stmt->getDebugLoc());

  Builder.getFunction()->incrementStatementCount();

  // IRGen Function declarations.
  if (/* auto *FD = */ dyn_cast<ESTree::FunctionDeclarationNode>(Stmt)) {
    // It has already been hoisted. Do nothing.  But, keep this to
    // match the AST structure, and we may want to do something in the
    // future.
    return;
  }

  // IRGen if statement.
  if (auto *IF = dyn_cast<ESTree::IfStatementNode>(Stmt)) {
    return genIfStatement(IF);
  }

  // IRGen for-in statement.
  if (auto *FIS = dyn_cast<ESTree::ForInStatementNode>(Stmt)) {
    return genForInStatement(FIS);
  }

  // IRGen return statement.
  if (auto *Ret = dyn_cast<ESTree::ReturnStatementNode>(Stmt)) {
    return genReturnStatement(Ret);
  }

  // Expression statement.
  if (auto *Exp = dyn_cast<ESTree::ExpressionStatementNode>(Stmt)) {
    // Generate the expression and ignore the result.
    return genExpressionWrapper(Exp->_expression);
  }

  // Handle Switch expressions.
  if (auto *SW = dyn_cast<ESTree::SwitchStatementNode>(Stmt)) {
    return genSwitchStmt(SW);
  }

  // Variable declarations:
  if (auto *VND = dyn_cast<ESTree::VariableDeclaratorNode>(Stmt)) {
    auto variableName = getNameFieldFromID(VND->_id);
    DEBUG(
        dbgs() << "IRGen variable declaration for \"" << variableName
               << "\".\n");

    // Materialize the initialization clause and save it into the variable.
    auto *storage = nameTable_.lookup(variableName);
    assert(storage && "Declared variable not found in name table");
    if (VND->_init) {
      DEBUG(dbgs() << "Variable \"" << variableName << "\" has initializer.\n");
      emitStore(Builder, genExpression(VND->_init, variableName), storage);
    }

    return;
  }

  if (auto *VDN = dyn_cast<ESTree::VariableDeclarationNode>(Stmt)) {
    for (auto &decl : VDN->_declarations) {
      genStatement(&decl);
    }
    return;
  }

  // IRGen the content of the block.
  if (auto *BS = dyn_cast<ESTree::BlockStatementNode>(Stmt)) {
    for (auto &Node : BS->_body) {
      genStatement(&Node);
    }

    return;
  }

  if (auto *Label = dyn_cast<ESTree::LabeledStatementNode>(Stmt)) {
    // Create a new basic block which is the continuation of the current block
    // and the jump target of the label.
    BasicBlock *next = Builder.createBasicBlock(functionContext->function);

    // Set the jump point for the label to the new block.
    functionContext->labels[Label->getLabelIndex()].breakTarget = next;

    // Now, generate the IR for the statement that the label is annotating.
    genStatement(Label->_body);

    // End the current basic block with a jump to the new basic block.
    Builder.createBranchInst(next);
    Builder.setInsertionBlock(next);

    return;
  }

  // Handle the call expression that could appear in the context of statement
  // expr without the ExpressionStatementNode wrapper.
  if (auto *call = dyn_cast<ESTree::CallExpressionNode>(Stmt)) {
    return genExpressionWrapper(call);
  }

  if (auto *W = dyn_cast<ESTree::WhileStatementNode>(Stmt)) {
    DEBUG(dbgs() << "IRGen 'while' statement\n");
    genForWhileLoops(W, nullptr, W->_test, W->_test, nullptr, W->_body);
    return;
  }

  if (auto *F = dyn_cast<ESTree::ForStatementNode>(Stmt)) {
    DEBUG(dbgs() << "IRGen 'for' statement\n");
    genForWhileLoops(F, F->_init, F->_test, F->_test, F->_update, F->_body);
    return;
  }

  if (auto *D = dyn_cast<ESTree::DoWhileStatementNode>(Stmt)) {
    DEBUG(dbgs() << "IRGen 'do..while' statement\n");
    genForWhileLoops(D, nullptr, nullptr, D->_test, nullptr, D->_body);
    return;
  }

  if (auto *breakStmt = dyn_cast<ESTree::BreakStatementNode>(Stmt)) {
    DEBUG(dbgs() << "IRGen 'break' statement\n");

    auto labelIndex = breakStmt->getLabelIndex();
    auto &label = functionContext->labels[labelIndex];
    assert(label.breakTarget && "breakTarget not set");

    genFinallyBeforeControlChange(
        breakStmt->surroundingTry,
        functionContext->getSemInfo()->labels[labelIndex].surroundingTry);
    Builder.createBranchInst(label.breakTarget);

    // Continue code generation for stuff that comes after the break statement
    // in a new dead block.
    auto newBlock = Builder.createBasicBlock(functionContext->function);
    Builder.setInsertionBlock(newBlock);
    return;
  }

  if (auto *continueStmt = dyn_cast<ESTree::ContinueStatementNode>(Stmt)) {
    DEBUG(dbgs() << "IRGen 'continue' statement\n");

    auto labelIndex = continueStmt->getLabelIndex();
    auto &label = functionContext->labels[labelIndex];
    assert(label.continueTarget && "continueTarget not set");

    genFinallyBeforeControlChange(
        continueStmt->surroundingTry,
        functionContext->getSemInfo()->labels[labelIndex].surroundingTry);
    Builder.createBranchInst(label.continueTarget);

    // Continue code generation for stuff that comes after the break statement
    // in a new dead block.
    auto newBlock = Builder.createBasicBlock(functionContext->function);
    Builder.setInsertionBlock(newBlock);
    return;
  }

  if (auto *T = dyn_cast<ESTree::TryStatementNode>(Stmt)) {
    genTry(T);
    return;
  }

  if (auto *T = dyn_cast<ESTree::ThrowStatementNode>(Stmt)) {
    DEBUG(dbgs() << "IRGen 'throw' statement\n");
    Value *rightHandVal = genExpression(T->_argument);
    Builder.createThrowInst(rightHandVal);

    // Throw interferes with control flow, hence we need a new block.
    auto newBlock =
        Builder.createBasicBlock(Builder.getInsertionBlock()->getParent());
    Builder.setInsertionBlock(newBlock);
    return;
  }

  // Handle empty statements.
  if (isa<ESTree::EmptyStatementNode>(Stmt)) {
    return;
  }

  // Handle debugger statements.
  if (isa<ESTree::DebuggerStatementNode>(Stmt)) {
    Builder.createDebuggerInst();
    return;
  }

  Builder.getModule()->getContext().getSourceErrorManager().error(
      Stmt->getSourceRange(), Twine("Unsupported statement encountered."));
}

Value *ESTreeIRGen::genMemberExpressionProperty(
    ESTree::MemberExpressionNode *Mem) {
  // If computed is true, the node corresponds to a computed (a[b]) member
  // lookup and '_property' is an Expression. Otherwise, the node
  // corresponds to a static (a.b) member lookup and '_property' is an
  // Identifier.
  // Details of the computed field are available here:
  // https://github.com/estree/estree/blob/master/spec.md#memberexpression

  if (Mem->_computed) {
    return genExpression(Mem->_property);
  }

  // Arrays and objects may be accessed with integer indices.
  if (auto N = dyn_cast<ESTree::NumericLiteralNode>(Mem->_property)) {
    return Builder.getLiteralNumber(N->_value);
  }

  // ESTree encodes property access as MemberExpression -> Identifier.
  auto Id = cast<ESTree::IdentifierNode>(Mem->_property);

  Identifier fieldName = getNameFieldFromID(Id);
  DEBUG(
      dbgs() << "Emitting direct lable access to field '" << fieldName
             << "'\n");
  return Builder.getLiteralString(fieldName);
}

StringRef ESTreeIRGen::propertyKeyAsString(
    llvm::SmallVectorImpl<char> &storage,
    ESTree::Node *Key) {
  // Handle String Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-string-literals
  if (auto *Lit = dyn_cast<ESTree::StringLiteralNode>(Key)) {
    DEBUG(dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Lit->_value->str();
  }

  // Handle identifiers as if they are String Literals.
  if (auto *Iden = dyn_cast<ESTree::IdentifierNode>(Key)) {
    DEBUG(dbgs() << "Loading String Literal \"" << Iden->_name << "\"\n");
    return Iden->_name->str();
  }

  // Handle Number Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-numeric-literals
  if (auto *Lit = dyn_cast<ESTree::NumericLiteralNode>(Key)) {
    DEBUG(dbgs() << "Loading Numeric Literal \"" << Lit->_value << "\"\n");
    storage.resize(NUMBER_TO_STRING_BUF_SIZE);
    auto len = numberToString(Lit->_value, storage.data(), storage.size());
    return StringRef(storage.begin(), len);
  }

  llvm_unreachable("Don't know this kind of property key");
  return StringRef();
}

Value *ESTreeIRGen::genPropertyKey(ESTree::Node *Key) {
  // Handle Number Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-numeric-literals
  if (auto *Lit = dyn_cast<ESTree::NumericLiteralNode>(Key)) {
    DEBUG(dbgs() << "Loading Numeric Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralNumber(Lit->_value);
  }

  // Let propertyKeyAsString() handle the other possibilities, as they are all
  // strings.
  llvm::SmallVector<char, 1> dummyStorage;
  return Builder.getLiteralString(propertyKeyAsString(dummyStorage, Key));
}

Value *ESTreeIRGen::genObjectExpr(ESTree::ObjectExpressionNode *Expr) {
  DEBUG(dbgs() << "Initializing a new object\n");

  /// Store information about a property. Is it an accessor (getter/setter) or
  /// a value, and the actual value.
  class PropertyValue {
   public:
    /// Is this a getter/setter value.
    bool isAccessor = false;
    /// Did we set the accessors of this property already. We need this because
    /// accessors are two separate ObjectMethod nodes, but we can only set them
    /// once.
    bool accessorsGenerated = false;
    /// The value, if this is a regular property
    ESTree::Node *valueNode{};
    /// Getter accessor, if this is an accessor property.
    ESTree::ObjectMethodNode *getterNode{};
    /// Setter accessor, if this is an accessor property.
    ESTree::ObjectMethodNode *setterNode{};

    SMRange getSourceRange() {
      if (valueNode) {
        return valueNode->getSourceRange();
      }

      if (getterNode) {
        return getterNode->getSourceRange();
      }

      if (setterNode) {
        return setterNode->getSourceRange();
      }

      llvm_unreachable("Unset node has no location info");
    }

    void setValue(ESTree::Node *val) {
      isAccessor = false;
      valueNode = val;
      getterNode = setterNode = nullptr;
    }
    void setGetter(ESTree::ObjectMethodNode *get) {
      if (!isAccessor) {
        valueNode = nullptr;
        setterNode = nullptr;
        isAccessor = true;
      }
      getterNode = get;
    }
    void setSetter(ESTree::ObjectMethodNode *set) {
      if (!isAccessor) {
        valueNode = nullptr;
        getterNode = nullptr;
        isAccessor = true;
      }
      setterNode = set;
    }
  };

  // First accumulate all getters and setters. We walk all properties, convert
  // them to string and store the value into propMap, possibly overwriting the
  // previous value.
  llvm::StringMap<PropertyValue> propMap;
  llvm::SmallVector<char, 32> stringStorage;

  for (auto &P : Expr->_properties) {
    // We are reusing the storage, so make sure it is cleared at every
    // iteration.
    stringStorage.clear();

    if (auto prop = dyn_cast<ESTree::ObjectPropertyNode>(&P)) {
      propMap[propertyKeyAsString(stringStorage, prop->_key)].setValue(
          prop->_value);
    } else if (auto method = dyn_cast<ESTree::ObjectMethodNode>(&P)) {
      PropertyValue *propValue =
          &propMap[propertyKeyAsString(stringStorage, method->_key)];
      if (method->_kind->str() == "get")
        propValue->setGetter(method);
      else
        propValue->setSetter(method);
    } else {
      llvm_unreachable("invalid object property node");
    }
  }

  // Allocate a new javascript object on the heap.
  auto Obj = Builder.createAllocObjectInst(propMap.size());

  // Initialize all properties. We check whether the value of each property
  // will be overwritten (by comparing against what we have saved in propMap).
  // In that case we still compute the value (it could have side effects), but
  // we don't store it. The exception to this are accessor functions - there
  // is no need to create them if we don't use them because creating a function
  // has no side effects.
  for (auto &P : Expr->_properties) {
    // We are reusing the storage, so make sure it is cleared at every
    // iteration.
    stringStorage.clear();

    if (auto method = dyn_cast<ESTree::ObjectMethodNode>(&P)) {
      StringRef keyStr = propertyKeyAsString(stringStorage, method->_key);
      PropertyValue *propValue = &propMap[keyStr];
      // If the property ended up not being a getter/setter, or if we already
      // generated it, skip.
      if (!propValue->isAccessor || propValue->accessorsGenerated)
        continue;

      Value *getter = Builder.getLiteralUndefined();
      Value *setter = Builder.getLiteralUndefined();

      if (propValue->getterNode) {
        Function *newFunc = genFunctionLike(
            Builder.createIdentifier("get " + keyStr.str()),
            nullptr,
            ESTree::isStrict(propValue->getterNode->strictness),
            propValue->getterNode,
            propValue->getterNode->_params,
            propValue->getterNode->_body,
            Mod->getContext().isLazyCompilation());

        getter = Builder.createCreateFunctionInst(newFunc);
      }

      if (propValue->setterNode) {
        Function *newFunc = genFunctionLike(
            Builder.createIdentifier("set " + keyStr.str()),
            nullptr,
            ESTree::isStrict(propValue->setterNode->strictness),
            propValue->setterNode,
            propValue->setterNode->_params,
            propValue->setterNode->_body,
            Mod->getContext().isLazyCompilation());

        setter = Builder.createCreateFunctionInst(newFunc);
      }

      // StoreGetterSetterInst requires a string literal
      auto Key = Builder.getLiteralString(keyStr);
      Builder.createStoreGetterSetterInst(getter, setter, Obj, Key);

      propValue->accessorsGenerated = true;
    } else if (auto prop = dyn_cast<ESTree::ObjectPropertyNode>(&P)) {
      StringRef keyStr = propertyKeyAsString(stringStorage, prop->_key);

      // Always generate the values, even if we don't need it, for the side
      // effects.
      auto value =
          genExpression(prop->_value, Builder.createIdentifier(keyStr));

      // Only store the value if it won't be overwritten.
      if (propMap[keyStr].valueNode == prop->_value) {
        // StoreOwnPropertyInst requires a string literal.
        auto Key = Builder.getLiteralString(keyStr);
        Builder.createStoreOwnPropertyInst(value, Obj, Key);
      } else {
        Builder.getModule()->getContext().getSourceErrorManager().warning(
            propMap[keyStr].getSourceRange(),
            Twine("the property \"") + keyStr +
                "\" was set multiple times in the object definition.");

        StringRef note = "Previous definition location was here.";
        Builder.getModule()->getContext().getSourceErrorManager().note(
            prop->getSourceRange(), note);
      }
    } else {
      llvm_unreachable("invalid object property node");
    }
  }

  // Return the newly allocated object (because this is an expression, not a
  // statement).
  return Obj;
}

Value *ESTreeIRGen::genArrayExpr(ESTree::ArrayExpressionNode *Expr) {
  DEBUG(dbgs() << "Initializing a new array\n");
  AllocArrayInst::ArrayValueList elements;

  // We store consecutive elements until we encounter elision,
  // or we enounter a non-literal in limited-register mode.
  // The rest of them has to be initialized through property sets.
  unsigned count = 0;
  bool consecutive = true;
  auto codeGenOpts = Mod->getContext().getCodeGenerationSettings();
  AllocArrayInst *allocArrayInst = nullptr;
  for (auto &E : Expr->_elements) {
    Value *value{nullptr};
    if (!isa<ESTree::EmptyNode>(&E)) {
      value = genExpression(&E);
    }
    if (!value || (!isa<Literal>(value) && !codeGenOpts.unlimitedRegisters)) {
      // This is either an elision,
      // or a non-literal in limited-register mode.
      if (consecutive) {
        // So far we have been storing elements consecutively,
        // but not anymore, time to create the array.
        allocArrayInst =
            Builder.createAllocArrayInst(elements, Expr->_elements.size());
        consecutive = false;
      }
    }
    if (value) {
      if (consecutive) {
        elements.push_back(value);
      } else {
        Builder.createStoreOwnPropertyInst(value, allocArrayInst, count);
      }
    }
    count++;
  }

  if (!allocArrayInst) {
    allocArrayInst =
        Builder.createAllocArrayInst(elements, Expr->_elements.size());
  }
  if (count > 0 && isa<ESTree::EmptyNode>(&Expr->_elements.back())) {
    // Last element is an elision, VM cannot derive the length properly.
    // We have to explicitly set it.
    Builder.createStorePropertyInst(
        Builder.getLiteralNumber(count), allocArrayInst, StringRef("length"));
  }
  return allocArrayInst;
}

Value *ESTreeIRGen::genCallExpr(ESTree::CallExpressionNode *call) {
  DEBUG(dbgs() << "IRGen 'call' statement/expression.\n");

  // Check for a direct call to eval().
  if (auto *identNode = dyn_cast<ESTree::IdentifierNode>(call->_callee)) {
    if (Identifier::getFromPointer(identNode->_name) == identEval_) {
      auto *evalVar = nameTable_.lookup(identEval_);
      if (!evalVar || isa<GlobalObjectProperty>(evalVar))
        return genCallEvalExpr(call);
    }
  }

  Value *thisVal;
  Value *callee;

  // Handle MemberExpression expression calls that sets the 'this' property.
  if (auto *Mem = dyn_cast<ESTree::MemberExpressionNode>(call->_callee)) {
    Value *obj = genExpression(Mem->_object);
    Value *prop = genMemberExpressionProperty(Mem);

    // Call the callee with obj as the 'this' pointer.
    thisVal = obj;
    callee = Builder.createLoadPropertyInst(obj, prop);
  } else {
    thisVal = Builder.getLiteralUndefined();
    callee = genExpression(call->_callee);
  }

  CallInst::ArgumentList args;
  for (auto &arg : call->_arguments) {
    args.push_back(genExpression(&arg));
  }

  return Builder.createCallInst(callee, thisVal, args);
}

Value *ESTreeIRGen::genCallEvalExpr(ESTree::CallExpressionNode *call) {
  if (call->_arguments.empty()) {
    Mod->getContext().getSourceErrorManager().warning(
        call->getSourceRange(), "eval() without arguments returns undefined");
    return Builder.getLiteralUndefined();
  }

  Mod->getContext().getSourceErrorManager().warning(
      Warning::DirectEval,
      call->getSourceRange(),
      "Direct call to eval(), but lexical scope is not supported.");

  llvm::SmallVector<Value *, 1> args;
  for (auto &arg : call->_arguments) {
    args.push_back(genExpression(&arg));
  }

  if (args.size() > 1) {
    Mod->getContext().getSourceErrorManager().warning(
        call->getSourceRange(), "Extra eval() arguments are ignored");
  }

  return Builder.createDirectEvalInst(args[0]);
}

/// \returns true if \p node is the default case.
static bool isDefaultCase(ESTree::SwitchCaseNode *caseStmt) {
  // If there is no test field then this is the default block.
  return !caseStmt->_test;
}

/// \returns true if \p node is a constant expression.
static bool isConstantExpr(ESTree::Node *node) {
  switch (node->getKind()) {
    case ESTree::NodeKind::StringLiteral:
    case ESTree::NodeKind::NumericLiteral:
    case ESTree::NodeKind::NullLiteral:
    case ESTree::NodeKind::BooleanLiteral:
      return true;
    default:
      return false;
  }
}

bool ESTreeIRGen::areAllCasesConstant(
    ESTree::SwitchStatementNode *switchStmt,
    llvm::SmallVectorImpl<Literal *> &caseLiterals) {
  for (auto &c : switchStmt->_cases) {
    auto *caseStmt = cast<ESTree::SwitchCaseNode>(&c);

    if (isDefaultCase(caseStmt)) {
      caseLiterals.push_back(nullptr);
      continue;
    }

    if (!isConstantExpr(caseStmt->_test))
      return false;

    auto *lit = dyn_cast<Literal>(genExpression(caseStmt->_test));
    assert(lit && "constant expression must compile to a literal");
    caseLiterals.push_back(lit);
  }

  return true;
}

void ESTreeIRGen::genSwitchStmt(ESTree::SwitchStatementNode *switchStmt) {
  DEBUG(dbgs() << "IRGen 'switch' statement.\n");

  {
    llvm::SmallVector<Literal *, 8> caseLiterals{};
    if (areAllCasesConstant(switchStmt, caseLiterals) &&
        caseLiterals.size() > 1) {
      genConstSwitchStmt(switchStmt, caseLiterals);
      return;
    }
  }

  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *exitBlock = Builder.createBasicBlock(function);

  // Unless a default is specified the default case brings us to the exit block.
  BasicBlock *defaultBlock = exitBlock;

  // A BB for each case in the switch statement.
  llvm::SmallVector<BasicBlock *, 8> caseBlocks;

  // Initialize the goto labels.
  auto &label = functionContext->labels[switchStmt->getLabelIndex()];
  label.breakTarget = exitBlock;

  // The discriminator expression.
  Value *discr = genExpression(switchStmt->_discriminant);

  // Sequentially allocate a basic block for each case, compare the discriminant
  // against the case value and conditionally jump to the basic block.
  int caseIndex = -1; // running index of the case's basic block.
  BasicBlock *elseBlock = nullptr; // The next case's condition.

  for (auto &c : switchStmt->_cases) {
    auto *caseStmt = cast<ESTree::SwitchCaseNode>(&c);
    ++caseIndex;
    caseBlocks.push_back(Builder.createBasicBlock(function));

    if (isDefaultCase(caseStmt)) {
      defaultBlock = caseBlocks.back();
      continue;
    }

    auto *caseVal = genExpression(caseStmt->_test);
    auto *pred = Builder.createBinaryOperatorInst(
        caseVal, discr, BinaryOperatorInst::OpKind::StrictlyEqualKind);

    elseBlock = Builder.createBasicBlock(function);
    Builder.createCondBranchInst(pred, caseBlocks[caseIndex], elseBlock);
    Builder.setInsertionBlock(elseBlock);
  }

  Builder.createBranchInst(defaultBlock);

  // Generate the case bodies.
  bool isFirstCase = true;
  caseIndex = -1;
  for (auto &c : switchStmt->_cases) {
    auto *caseStmt = cast<ESTree::SwitchCaseNode>(&c);
    ++caseIndex;

    // Generate the fall-through from the previous block to this one.
    if (!isFirstCase)
      Builder.createBranchInst(caseBlocks[caseIndex]);

    Builder.setInsertionBlock(caseBlocks[caseIndex]);
    genBody(caseStmt->_consequent);
    isFirstCase = false;
  }

  if (!isFirstCase)
    Builder.createBranchInst(exitBlock);

  Builder.setInsertionBlock(exitBlock);
}

void ESTreeIRGen::genConstSwitchStmt(
    ESTree::SwitchStatementNode *switchStmt,
    llvm::SmallVectorImpl<Literal *> &caseLiterals) {
  Function *function = Builder.getInsertionBlock()->getParent();
  BasicBlock *exitBlock = Builder.createBasicBlock(function);

  // Unless a default is specified the default case brings us to the exit block.
  BasicBlock *defaultBlock = exitBlock;

  auto &label = functionContext->labels[switchStmt->getLabelIndex()];
  label.breakTarget = exitBlock;

  // The discriminator expression.
  Value *discr = genExpression(switchStmt->_discriminant);
  // Save the block where we will insert the switch instruction.
  auto *startBlock = Builder.getInsertionBlock();

  // Since this is a constant value switch, duplicates are not allowed and we
  // must filter them. We can conveniently store them in this set.
  llvm::SmallPtrSet<Literal *, 8> valueSet;

  SwitchInst::ValueListType values;
  SwitchInst::BasicBlockListType blocks;

  int caseIndex = -1;
  bool isFirstCase = true;

  for (auto &c : switchStmt->_cases) {
    auto *caseStmt = cast<ESTree::SwitchCaseNode>(&c);
    auto *caseBlock = Builder.createBasicBlock(function);
    ++caseIndex;

    if (isDefaultCase(caseStmt)) {
      defaultBlock = caseBlock;
    } else {
      auto *lit = caseLiterals[caseIndex];

      // Only generate the case and block if this is the first occurence of the
      // value.
      if (valueSet.insert(lit).second) {
        values.push_back(lit);
        blocks.push_back(caseBlock);
      }
    }

    if (!isFirstCase)
      Builder.createBranchInst(caseBlock);

    Builder.setInsertionBlock(caseBlock);
    genBody(caseStmt->_consequent);
    isFirstCase = false;
  }

  if (!isFirstCase)
    Builder.createBranchInst(exitBlock);

  Builder.setInsertionBlock(startBlock);
  Builder.createSwitchInst(discr, defaultBlock, values, blocks);

  Builder.setInsertionBlock(exitBlock);
};

Value *ESTreeIRGen::genNewExpr(ESTree::NewExpressionNode *N) {
  DEBUG(dbgs() << "IRGen 'new' statement/expression.\n");

  // Implement the new operator.
  // http://www.ecma-international.org/ecma-262/7.0/index.html#sec-new-operator

  Value *callee = genExpression(N->_callee);

  ConstructInst::ArgumentList args;
  for (auto &arg : N->_arguments) {
    args.push_back(genExpression(&arg));
  }

  return Builder.createConstructInst(callee, args);
}

Value *ESTreeIRGen::genAssignmentExpr(ESTree::AssignmentExpressionNode *AE) {
  DEBUG(dbgs() << "IRGen assignment operator.\n");

  auto opStr = AE->_operator->str();
  auto AssignmentKind = BinaryOperatorInst::parseAssignmentOperator(opStr);

  LReference lref = createLRef(AE->_left);
  Value *RHS = nullptr;

  Identifier nameHint{};
  if (auto *var = lref.castAsVariable()) {
    nameHint = var->getName();
  } else if (auto *globProp = lref.castAsGlobalObjectProperty()) {
    nameHint = globProp->getName()->getValue();
  }

  if (AssignmentKind != BinaryOperatorInst::OpKind::IdentityKind) {
    // Section 11.13.1 specifies that we should first load the
    // LHS before materializing the RHS. Unlike in C, this
    // code is well defined: "x+= x++".
    // https://es5.github.io/#x11.13.1
    auto V = lref.emitLoad(Builder);
    RHS = genExpression(AE->_right, nameHint);
    RHS = Builder.createBinaryOperatorInst(V, RHS, AssignmentKind);
  } else {
    RHS = genExpression(AE->_right, nameHint);
  }

  lref.emitStore(Builder, RHS);

  // Return the value that we stored as the result of the expression.
  return RHS;
}

Value *ESTreeIRGen::genConditionalExpr(ESTree::ConditionalExpressionNode *C) {
  auto parentFunc = Builder.getInsertionBlock()->getParent();

  PhiInst::ValueListType values;
  PhiInst::BasicBlockListType blocks;

  auto alternateBlock = Builder.createBasicBlock(parentFunc);
  auto consequentBlock = Builder.createBasicBlock(parentFunc);
  auto continueBlock = Builder.createBasicBlock(parentFunc);

  // Implement the ternary operator using control flow. We must use control
  // flow because the expressions may have side effects.
  genExpressionBranch(C->_test, consequentBlock, alternateBlock);

  // The 'then' side:
  Builder.setInsertionBlock(consequentBlock);

  values.push_back(genExpression(C->_consequent));
  blocks.push_back(Builder.getInsertionBlock());
  Builder.createBranchInst(continueBlock);

  // The 'else' side:
  Builder.setInsertionBlock(alternateBlock);
  values.push_back(genExpression(C->_alternate));
  blocks.push_back(Builder.getInsertionBlock());
  Builder.createBranchInst(continueBlock);

  // Continue:
  Builder.setInsertionBlock(continueBlock);
  return Builder.createPhiInst(values, blocks);
}

Value *ESTreeIRGen::genSequenceExpr(ESTree::SequenceExpressionNode *Sq) {
  Value *result = Builder.getLiteralUndefined();

  // Generate all expressions in the sequence, but take only the last one.
  for (auto &Ex : Sq->_expressions) {
    result = genExpression(&Ex);
  }

  return result;
}

Value *ESTreeIRGen::genUnaryExpression(ESTree::UnaryExpressionNode *U) {
  auto kind = UnaryOperatorInst::parseOperator(U->_operator->str());

  // Handle the delete unary expression. https://es5.github.io/#x11.4.1
  if (kind == UnaryOperatorInst::OpKind::DeleteKind) {
    if (auto *memberExpr =
            dyn_cast<ESTree::MemberExpressionNode>(U->_argument)) {
      DEBUG(dbgs() << "IRGen delete member expression.\n");

      Value *obj = genExpression(memberExpr->_object);
      Value *prop = genMemberExpressionProperty(memberExpr);

      // If this assignment is not the identity assignment ('=') then emit a
      // load-operation-store sequence.
      return Builder.createDeletePropertyInst(obj, prop);
    }

    // Check for "delete identifier". Note that deleting unqualified identifiers
    // is prohibited in strict mode, so that case is handled earlier in the
    // semantic validator. Here we are left to handle the non-strict mode case.
    if (auto *iden = dyn_cast<ESTree::IdentifierNode>(U->_argument)) {
      assert(
          !functionContext->function->isStrictMode() &&
          "delete identifier encountered in strict mode");
      // Check if this is a known variable.
      Identifier name = getNameFieldFromID(iden);
      auto *var = nameTable_.lookup(name);

      if (!var || isa<GlobalObjectProperty>(var)) {
        // If the variable doesn't exist or if it is global, we must generate
        // a delete global property instruction.
        return Builder.createDeletePropertyInst(
            Builder.getGlobalObject(), Builder.getLiteralString(name));
      } else {
        // Otherwise it is a local variable which can't be deleted and we just
        // return false.
        return Builder.getLiteralBool(false);
      }
    }

    // Generate the code for the delete operand.
    genExpression(U->_argument);

    // Deleting any value or a result of an expression returns True.
    return Builder.getLiteralBool(true);
  }

  // Need to handle the special case of "typeof <undefined variable>".
  if (kind == UnaryOperatorInst::OpKind::TypeofKind) {
    if (auto *id = dyn_cast<ESTree::IdentifierNode>(U->_argument)) {
      Value *argument = genIdentifierExpression(id, true);
      return Builder.createUnaryOperatorInst(argument, kind);
    }
  }

  // Generate the unary operand:
  Value *argument = genExpression(U->_argument);

  if (kind == UnaryOperatorInst::OpKind::PlusKind) {
    return Builder.createAsNumberInst(argument);
  }

  return Builder.createUnaryOperatorInst(argument, kind);
}

void ESTreeIRGen::genExpressionWrapper(ESTree::Node *expr) {
  Value *val = genExpression(expr);
  if (functionContext->globalReturnRegister) {
    Builder.createStoreStackInst(val, functionContext->globalReturnRegister);
  }
}

Value *ESTreeIRGen::genIdentifierExpression(
    ESTree::IdentifierNode *Iden,
    bool afterTypeOf) {
  DEBUG(dbgs() << "Looking for identifier \"" << Iden->_name << "\"\n");

  // 'arguments' is an array-like object holding all function arguments.
  // If one of the parameters is called "arguments" then it shadows the
  // arguments keyword.
  if (Iden->_name->str() == "arguments" &&
      !nameTable_.count(getNameFieldFromID(Iden))) {
    // The first time we encounter 'arguments' we must initialize the
    // arguments object before the entry terminator.
    if (!functionContext->createdArguments) {
      DEBUG(dbgs() << "Creating arguments object\n");

      IRBuilder::SaveRestore saveBuilder(Builder);
      Builder.setInsertionPoint(functionContext->entryTerminator);
      Builder.setLocation(Builder.getFunction()->getSourceRange().Start);
      functionContext->createdArguments = Builder.createCreateArgumentsInst();
      functionContext->createdArguments->moveBefore(
          functionContext->entryTerminator);
    }

    return functionContext->createdArguments;
  }

  // Lookup variable name.
  auto StrName = getNameFieldFromID(Iden);

  auto *Var = ensureVariableExists(Iden);

  // For uses of undefined as the global property, we make an optimization
  // to always return undefined constant.
  if (isa<GlobalObjectProperty>(Var) && StrName.str() == "undefined") {
    return Builder.getLiteralUndefined();
  }

  DEBUG(
      dbgs() << "Found variable " << StrName << " in function \""
             << (isa<GlobalObjectProperty>(Var) ? StringRef("global")
                                                : cast<Variable>(Var)
                                                      ->getParent()
                                                      ->getFunction()
                                                      ->getInternalNameStr())
             << "\"\n");

  // Typeof <variable> does not throw.
  return emitLoad(Builder, Var, afterTypeOf);
}

Value *ESTreeIRGen::genExpression(ESTree::Node *Expr, Identifier nameHint) {
  DEBUG(dbgs() << "IRGen expression of type " << Expr->getNodeName() << "\n");
  IRBuilder::ScopedLocationChange slc(Builder, Expr->getDebugLoc());

  // Handle identifiers.
  if (auto *Iden = dyn_cast<ESTree::IdentifierNode>(Expr)) {
    return genIdentifierExpression(Iden, false);
  }

  // Handle Null Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-null-literals
  if (isa<ESTree::NullLiteralNode>(Expr)) {
    return Builder.getLiteralNull();
  }

  // Handle String Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-string-literals
  if (auto *Lit = dyn_cast<ESTree::StringLiteralNode>(Expr)) {
    DEBUG(dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralString(Lit->_value->str());
  }

  // Handle Regexp Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-regular-expression-literals
  if (auto *Lit = dyn_cast<ESTree::RegExpLiteralNode>(Expr)) {
    DEBUG(
        dbgs() << "Loading regexp Literal \"" << Lit->_pattern->str() << " / "
               << Lit->_flags->str() << "\"\n");

    return Builder.createRegExpInst(
        Identifier::getFromPointer(Lit->_pattern),
        Identifier::getFromPointer(Lit->_flags));
  }

  // Handle Boolean Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-boolean-literals
  if (auto *Lit = dyn_cast<ESTree::BooleanLiteralNode>(Expr)) {
    DEBUG(dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralBool(Lit->_value);
  }

  // Handle Number Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-numeric-literals
  if (auto *Lit = dyn_cast<ESTree::NumericLiteralNode>(Expr)) {
    DEBUG(dbgs() << "Loading Numeric Literal \"" << Lit->_value << "\"\n");
    return Builder.getLiteralNumber(Lit->_value);
  }

  // Handle the assignment expression.
  if (auto Assign = dyn_cast<ESTree::AssignmentExpressionNode>(Expr)) {
    return genAssignmentExpr(Assign);
  }

  // Handle Call expressions.
  if (auto *call = dyn_cast<ESTree::CallExpressionNode>(Expr)) {
    return genCallExpr(call);
  }

  // Handle the 'new' expressions.
  if (auto *newExp = dyn_cast<ESTree::NewExpressionNode>(Expr)) {
    return genNewExpr(newExp);
  }

  // Handle MemberExpression expressions for access property.
  if (auto *Mem = dyn_cast<ESTree::MemberExpressionNode>(Expr)) {
    LReference lref = createLRef(Mem);
    return lref.emitLoad(Builder);
  }

  // Handle Array expressions (syntax: [1,2,3]).
  if (auto *Arr = dyn_cast<ESTree::ArrayExpressionNode>(Expr)) {
    return genArrayExpr(Arr);
  }

  // Handle object expressions (syntax: {"1" : "2"}).
  if (auto *Obj = dyn_cast<ESTree::ObjectExpressionNode>(Expr)) {
    return genObjectExpr(Obj);
  }

  // Handle logical expressions (short circuiting).
  if (auto *L = dyn_cast<ESTree::LogicalExpressionNode>(Expr)) {
    return genLogicalExpression(L);
  }

  // Handle Binary Expressions.
  if (auto *Bin = dyn_cast<ESTree::BinaryExpressionNode>(Expr)) {
    Value *LHS = genExpression(Bin->_left);
    Value *RHS = genExpression(Bin->_right);

    auto Kind = BinaryOperatorInst::parseOperator(Bin->_operator->str());

    return Builder.createBinaryOperatorInst(LHS, RHS, Kind);
  }

  // Handle Unary operator Expressions.
  if (auto *U = dyn_cast<ESTree::UnaryExpressionNode>(Expr)) {
    return genUnaryExpression(U);
  }

  // Handle the 'this' keyword.
  if (isa<ESTree::ThisExpressionNode>(Expr)) {
    return Builder.getInsertionBlock()->getParent()->getThisParameter();
  }

  // Handle function expressions.
  if (auto *FE = dyn_cast<ESTree::FunctionExpressionNode>(Expr)) {
    DEBUG(
        dbgs() << "Creating anonymous closure. "
               << Builder.getInsertionBlock()->getParent()->getInternalName()
               << ".\n");

    NameTableScopeTy newScope(nameTable_);
    Variable *tempClosureVar = nullptr;

    Identifier originalNameIden = nameHint;
    if (FE->_id) {
      auto closureName = genAnonymousLabelName("closure");
      tempClosureVar = Builder.createVariable(
          functionContext->function->getFunctionScope(), closureName);

      // Insert the synthesized variable into the name table, so it can be
      // looked up internally as well.
      nameTable_.insertIntoScope(
          &functionContext->scope, tempClosureVar->getName(), tempClosureVar);

      // Alias the lexical name to the synthesized variable.
      originalNameIden = getNameFieldFromID(FE->_id);
      nameTable_.insert(originalNameIden, tempClosureVar);
    }

    Function *newFunc = genFunctionLike(
        originalNameIden,
        tempClosureVar,
        ESTree::isStrict(FE->strictness),
        FE,
        FE->_params,
        FE->_body,
        Mod->getContext().isLazyCompilation());

    Value *func = Builder.createCreateFunctionInst(newFunc);

    if (tempClosureVar)
      emitStore(Builder, func, tempClosureVar);

    return func;
  }

  if (auto *U = dyn_cast<ESTree::UpdateExpressionNode>(Expr)) {
    return genUpdateExpr(U);
  }

  if (auto *C = dyn_cast<ESTree::ConditionalExpressionNode>(Expr)) {
    return genConditionalExpr(C);
  }

  if (auto *Sq = dyn_cast<ESTree::SequenceExpressionNode>(Expr)) {
    return genSequenceExpr(Sq);
  }

  assert(false && "Don't know this kind of expression");
  return nullptr;
}

void ESTreeIRGen::genExpressionBranch(
    ESTree::Node *expr,
    BasicBlock *onTrue,
    BasicBlock *onFalse) {
  switch (expr->getKind()) {
    case ESTree::NodeKind::LogicalExpression:
      return genLogicalExpressionBranch(
          cast<ESTree::LogicalExpressionNode>(expr), onTrue, onFalse);

    case ESTree::NodeKind::UnaryExpression: {
      auto *e = cast<ESTree::UnaryExpressionNode>(expr);
      switch (UnaryOperatorInst::parseOperator(e->_operator->str())) {
        case UnaryOperatorInst::OpKind::BangKind:
          return genExpressionBranch(e->_argument, onFalse, onTrue);
        default:
          break;
      }

      break;
    }

    case ESTree::NodeKind::SequenceExpression: {
      auto *e = cast<ESTree::SequenceExpressionNode>(expr);

      ESTree::NodePtr last = nullptr;
      for (auto &ex : e->_expressions) {
        if (last)
          genExpression(last);
        last = &ex;
      }
      if (last)
        genExpressionBranch(last, onTrue, onFalse);
      return;
    }

    default:
      break;
  }

  Value *condVal = genExpression(expr);
  Builder.createCondBranchInst(condVal, onTrue, onFalse);
}

void ESTreeIRGen::genFinallyBeforeControlChange(
    ESTree::TryStatementNode *sourceTry,
    ESTree::TryStatementNode *targetTry) {
  // We walk the nested try statements starting from the source, until we reach
  // the target, generating the finally statements on the way.
  for (; sourceTry != targetTry; sourceTry = sourceTry->surroundingTry) {
    assert(sourceTry && "invalid try chain");

    // Emit an end of the try statement.
    auto *tryEndBlock = Builder.createBasicBlock(functionContext->function);
    Builder.createBranchInst(tryEndBlock);
    Builder.setInsertionBlock(tryEndBlock);

    // Make sure we use the correct debug location for tryEndInst.
    if (sourceTry->_finalizer) {
      hermes::IRBuilder::ScopedLocationChange slc(
          Builder, sourceTry->_finalizer->getDebugLoc());
      Builder.createTryEndInst();
    } else {
      Builder.createTryEndInst();
    }

    if (sourceTry->_finalizer)
      genStatement(sourceTry->_finalizer);
  }
}

GlobalObjectProperty *ESTreeIRGen::declareAmbientGlobalProperty(
    Identifier name) {
  // Avoid redefining global properties.
  auto *prop = dyn_cast_or_null<GlobalObjectProperty>(nameTable_.lookup(name));
  if (prop)
    return prop;

  DEBUG(
      llvm::dbgs() << "declaring ambient global property " << name << " "
                   << name.getUnderlyingPointer() << "\n");

  prop = Builder.createGlobalObjectProperty(name, false);
  nameTable_.insertIntoScope(&topLevelContext->scope, name, prop);
  return prop;
}

namespace {
/// This visitor structs collects declarations within a single closure without
/// descending into child closures.
struct DeclHoisting {
  /// The list of collected identifiers (variables and functions).
  llvm::SmallVector<ESTree::VariableDeclaratorNode *, 8> decls{};

  /// A list of functions that need to be hoisted and materialized before we
  /// can generate the rest of the function.
  llvm::SmallVector<ESTree::FunctionDeclarationNode *, 8> closures;

  explicit DeclHoisting() = default;
  ~DeclHoisting() = default;

  /// Extract the variable name from the nodes that can define new variables.
  /// The nodes that can define a new variable in the scope are:
  /// VariableDeclarator and FunctionDeclaration>
  void collectDecls(ESTree::Node *V) {
    if (auto VD = dyn_cast<ESTree::VariableDeclaratorNode>(V)) {
      return decls.push_back(VD);
    }

    if (auto FD = dyn_cast<ESTree::FunctionDeclarationNode>(V)) {
      return closures.push_back(FD);
    }
  }

  bool shouldVisit(ESTree::Node *V) {
    // Collect declared names, even if we don't descend into children nodes.
    collectDecls(V);

    // Do not descend to child closures because the variables they define are
    // not exposed to the outside function.
    if (isa<ESTree::FunctionDeclarationNode>(V) ||
        isa<ESTree::FunctionExpressionNode>(V) ||
        isa<ESTree::ObjectMethodNode>(V))
      return false;
    return true;
  }

  void enter(ESTree::Node *V) {}
  void leave(ESTree::Node *V) {}
};

} // anonymous namespace.

void ESTreeIRGen::processDeclarationFile(ESTree::FileNode *fileNode) {
  auto File = dyn_cast_or_null<ESTree::FileNode>(fileNode);
  if (!File)
    return;

  auto Program = dyn_cast_or_null<ESTree::ProgramNode>(File->_program);
  if (!Program)
    return;

  DeclHoisting DH;
  Program->visit(DH);

  // Create variable declarations for each of the hoisted variables.
  for (auto vd : DH.decls)
    declareAmbientGlobalProperty(getNameFieldFromID(vd->_id));
  for (auto fd : DH.closures)
    declareAmbientGlobalProperty(getNameFieldFromID(fd->_id));
}

Value *ESTreeIRGen::ensureVariableExists(ESTree::IdentifierNode *id) {
  assert(id && "id must be a valid Identifier node");
  Identifier name = getNameFieldFromID(id);

  // Check if this is a known variable.
  if (auto *var = nameTable_.lookup(name))
    return var;

  if (functionContext->function->isStrictMode()) {
    // Report a warning in strict mode.
    auto currentFunc = Builder.getInsertionBlock()->getParent();

    Builder.getModule()->getContext().getSourceErrorManager().warning(
        Warning::UndefinedVariable,
        id->getSourceRange(),
        Twine("the variable \"") + name.str() +
            "\" was not declared in function \"" +
            currentFunc->getInternalNameStr() + "\"");
  }

  // Undeclared variable is an ambient global property.
  return declareAmbientGlobalProperty(name);
}

ESTreeIRGen::LReference ESTreeIRGen::createLRef(ESTree::Node *node) {
  SMLoc sourceLoc = node->getDebugLoc();
  IRBuilder::ScopedLocationChange slc(Builder, sourceLoc);

  /// Create lref for member expression (ex: o.f).
  if (auto *ME = dyn_cast<ESTree::MemberExpressionNode>(node)) {
    DEBUG(dbgs() << "Creating an LRef for member expression.\n");
    Value *obj = genExpression(ME->_object);
    Value *prop = genMemberExpressionProperty(ME);
    return LReference(obj, prop, sourceLoc);
  }

  /// Create lref for identifiers  (ex: a).
  if (auto *iden = dyn_cast<ESTree::IdentifierNode>(node)) {
    DEBUG(dbgs() << "Creating an LRef for identifier.\n");
    DEBUG(
        dbgs() << "Looking for identifier \"" << getNameFieldFromID(iden)
               << "\"\n");
    auto *var = ensureVariableExists(iden);
    return LReference(var, nullptr, sourceLoc);
  }

  /// Create lref for variable decls (ex: var a).
  if (auto *V = dyn_cast<ESTree::VariableDeclarationNode>(node)) {
    DEBUG(dbgs() << "Creating an LRef for variable declaration.\n");

    assert(V->_declarations.size() == 1 && "Malformed variable declaration");
    auto decl = cast<ESTree::VariableDeclaratorNode>(&V->_declarations.front());

    DEBUG(
        dbgs() << "Looking for var-identifier \""
               << getNameFieldFromID(decl->_id) << "\"\n");
    auto *var =
        ensureVariableExists(dyn_cast<ESTree::IdentifierNode>(decl->_id));
    return LReference(var, nullptr, sourceLoc);
  }

  llvm_unreachable("Unexpected for-in pattern.");
}

bool hermes::generateIRFromESTree(
    ESTree::NodePtr node,
    Module *M,
    const DeclarationFileListTy &declFileList,
    const ScopeChain &scopeChain) {
  // Generate IR into the module M.
  ESTreeIRGen Generator(node, declFileList, M, scopeChain);
  Generator.doIt();

  DEBUG(dbgs() << "Finished IRGen.\n");
  return false;
}

void hermes::generateIRForCJSModule(
    ESTree::FunctionExpressionNode *node,
    llvm::StringRef filename,
    Module *M,
    Function *topLevelFunction,
    const DeclarationFileListTy &declFileList) {
  // Generate IR into the module M.
  ESTreeIRGen generator(node, declFileList, M, {});
  return generator.doCJSModule(topLevelFunction, node->getSemInfo(), filename);
}

/// Generate a function which immediately throws the specified SyntaxError
/// message.
static Function *genSyntaxErrorFunction(
    Module *M,
    Identifier originalName,
    SMRange sourceRange,
    StringRef error) {
  IRBuilder builder{M};

  Function *function =
      builder.createFunction(originalName, true, sourceRange, false);

  builder.createParameter(function, "this");
  BasicBlock *firstBlock = builder.createBasicBlock(function);
  builder.setInsertionBlock(firstBlock);

  builder.createThrowInst(builder.createCallInst(
      emitLoad(
          builder, builder.createGlobalObjectProperty("SyntaxError", false)),
      builder.getLiteralUndefined(),
      builder.getLiteralString(error)));

  return function;
}

Function *hermes::generateLazyFunctionIR(
    hbc::LazyCompilationData *lazyData,
    Module *M) {
  auto &context = M->getContext();
  SimpleDiagHandlerRAII diagHandler{
      context.getSourceErrorManager().getSourceMgr()};

  AllocationScope alloc(context.getAllocator());
  sem::SemContext semCtx{};
  hermes::parser::JSParser parser(
      context, lazyData->bufferId, parser::LazyParse);

  // Note: we don't know the parent's strictness, which we need to pass, but
  // we can just use the child's strictness, which is always stricter or equal
  // to the parent's.
  parser.setStrictMode(lazyData->strictMode);

  auto parsed = parser.parseLazyFunction(
      (ESTree::NodeKind)lazyData->nodeKind, lazyData->span.Start);

  // In case of error, generate a function just throws a SyntaxError.
  if (!parsed ||
      !sem::validateFunctionAST(
          context, semCtx, *parsed, lazyData->strictMode)) {
    DEBUG(
        llvm::dbgs() << "Lazy AST parsing/validation failed with error: "
                     << diagHandler.getErrorString());
    return genSyntaxErrorFunction(
        M,
        lazyData->originalName,
        lazyData->span,
        diagHandler.getErrorString());
  }

  ESTreeIRGen generator{parsed.getValue(), {}, M, {}};
  return generator.doLazyFunction(lazyData);
}
