/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ESTreeIRGen.h"
#include "hermes/AST/ESTree.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/StringTable.h"
#include "llvh/ADT/ScopeExit.h"

namespace hermes {
namespace irgen {

/// This function initializes critical state that will be used in the rest of
/// IRGen for adding, reading, and writing private elements. The main
/// characteristic of private properties that informs our implementation is the
/// semi-typed nature of a private name. That is, each private name is uniquely
/// used to define a type of "private element" (using language from
/// ES2024 6.2.10 PrivateElement.) So when seeing `#privateName`, it is
/// immediately known if this expression refers to a field, a method, or an
/// accessor.
///
/// Methods & accessors have an even stronger guarantee. Because these private
/// elements are installed in bulk before any other user JS runs, and they are
/// not writable, we can actually store these values out of line- directly on
/// the instance. Private fields work mostly the same as normal properties, with
/// the added restrictions that trying to read a private field that doesn't
/// exist throws. But methods & accessors work very differently. Instead of
/// adding private methods as own properties to the instance (or the class in
/// the case of `static`) instead we add a single "private brand". This brand
/// can be thought of as a seal of approval: having this brand means the object
/// is allowed to use all methods & accessors granted by this brand.
///
/// Each class defines two private brands then: an instance brand and a static
/// brand. The class function object gets the static brand immediately on class
/// definition evaluation, and the instance gets the instance brand during
/// constructor execution. When trying to read a private method or accessor, we
/// first emit IR to check for the seal of approval. If it's there, then we can
/// directly invoke the corresponding function. So adding all instance or static
/// methods to an object is simply adding this single brand to it.
///
/// `Decl`s are the structure we use to hold these private brands, along with
/// the actual function objects to invoke. Take the following class:
///   class A {
///     #m1() {}
///     useM1() { this.#m1() }
///   }
/// In this case, there is a single decl that is associated with `#m1`. That
/// decl will contain two `Variable*`s. The first contains the value of the
/// instance private brand, and the second holds the function object value. In
/// order to get to these `Variable*`s, we create a side table that lives in
/// SemContext (so this can be used across lazy compilation.) This private name
/// side table contains lists of elements, where each element contains the
/// relevant set of `Variable*`s that needs to be associated with each private
/// name to hold the brand and function object. Most private names only need two
/// `Variable*`s, but a private name that defines both a getter and a setter
/// needs to hold the function object values for each accessor. So in that case,
/// the element in the side table is a different, larger struct. Just by
/// inspecting the kind of a `Decl`, it's possible to know which kind of element
/// in the side table the customData field of the `Decl` points to.
void ESTreeIRGen::emitPrivateNameDeclarations(
    sema::LexicalScope *scope,
    Identifier className) {
  Variable *staticBrand = nullptr;
  Variable *instanceBrand = nullptr;
  /// Lazily create and return the correct private brand for the given static
  /// level.
  auto getPrivateBrand =
      [this, &className, &staticBrand, &instanceBrand](bool isStatic) {
        if (isStatic) {
          if (!staticBrand) {
            staticBrand = Builder.createVariable(
                curFunction()->curScope->getVariableScope(),
                Twine("?static_brand_") + className.str(),
                Type::createPrivateName(),
                /* hidden */ true);
            Builder.createStoreFrameInst(
                curFunction()->curScope,
                Builder.createCreatePrivateNameInst(
                    Builder.getLiteralString(className)),
                staticBrand);
          }
          return staticBrand;
        }
        if (!instanceBrand) {
          instanceBrand = Builder.createVariable(
              curFunction()->curScope->getVariableScope(),
              Twine("?instance_brand_") + className.str(),
              Type::createPrivateName(),
              /* hidden */ true);
          Builder.createStoreFrameInst(
              curFunction()->curScope,
              Builder.createCreatePrivateNameInst(
                  Builder.getLiteralString(className)),
              instanceBrand);
        }
        return instanceBrand;
      };
  for (sema::Decl *decl : scope->decls) {
    if (!sema::Decl::isKindPrivateName(decl->kind))
      continue;
    assert(
        ((curFunction()->debugAllowRecompileCounter != 0) ||
         (decl->customData == nullptr)) &&
        "customData can be bound only if recompiling AST");
    switch (decl->kind) {
      case sema::Decl::Kind::PrivateField: {
        // PrivateField's custom data is Variable*.
        Variable *nameVar;
        if (!decl->customData) {
          nameVar = Builder.createVariable(
              curFunction()->curScope->getVariableScope(),
              decl->name,
              Type::createPrivateName(),
              /* hidden */ true);
          setDeclData(decl, nameVar);
        } else {
          nameVar = llvh::cast<Variable>(getDeclData(decl));
        }
        // Initialize private names to their values.
        Builder.createStoreFrameInst(
            curFunction()->curScope,
            Builder.createCreatePrivateNameInst(
                Builder.getLiteralString(decl->name)),
            nameVar);
        break;
      }
      case sema::Decl::Kind::PrivateMethod: {
        // PrivateMethod's custom data is
        // PrivateNameFunctionTable::SingleFunctionEntry*.
        if (!decl->customData) {
          Variable *methodClosure = Builder.createVariable(
              curFunction()->curScope->getVariableScope(),
              decl->name,
              Type::createObject(),
              /* hidden */ true);
          auto &elem = privateNameTable().singleFunctions.emplace_back(
              getPrivateBrand(
                  decl->special == sema::Decl::Special::PrivateStatic),
              methodClosure);
          setDeclDataPrivate(decl, &elem);
        }
        break;
      }
      case sema::Decl::Kind::PrivateGetter: {
        // PrivateGetter's custom data is
        // PrivateNameFunctionTable::SingleFunctionEntry*.
        if (!decl->customData) {
          Variable *getterClosure = Builder.createVariable(
              curFunction()->curScope->getVariableScope(),
              Twine("?private_get_") + decl->name.str(),
              Type::createObject(),
              /* hidden */ true);
          auto &elem = privateNameTable().singleFunctions.emplace_back(
              getPrivateBrand(
                  decl->special == sema::Decl::Special::PrivateStatic),
              getterClosure);
          setDeclDataPrivate(decl, &elem);
        }
        break;
      }
      case sema::Decl::Kind::PrivateSetter: {
        // PrivateSetter's custom data is
        // PrivateNameFunctionTable::SingleFunctionEntry*.
        if (!decl->customData) {
          Variable *setterClosure = Builder.createVariable(
              curFunction()->curScope->getVariableScope(),
              Twine("?private_set_") + decl->name.str(),
              Type::createObject(),
              /* hidden */ true);
          auto &elem = privateNameTable().singleFunctions.emplace_back(
              getPrivateBrand(
                  decl->special == sema::Decl::Special::PrivateStatic),
              setterClosure);
          setDeclDataPrivate(decl, &elem);
        }
        break;
      }
      case sema::Decl::Kind::PrivateGetterSetter: {
        // PrivateGetterSetter's custom data is
        // PrivateNameFunctionTable::GetterSetter*.
        if (!decl->customData) {
          Variable *getterClosure = Builder.createVariable(
              curFunction()->curScope->getVariableScope(),
              Twine("?private_get_") + decl->name.str(),
              Type::createObject(),
              /* hidden */ true);
          Variable *setterClosure = Builder.createVariable(
              curFunction()->curScope->getVariableScope(),
              Twine("?private_set_") + decl->name.str(),
              Type::createObject(),
              /* hidden */ true);
          auto &elem = privateNameTable().accessors.emplace_back(
              getPrivateBrand(
                  decl->special == sema::Decl::Special::PrivateStatic),
              getterClosure,
              setterClosure);
          setDeclDataPrivate(decl, &elem);
        }
        break;
      }
      default:
        assert(false && "unhandled private decl");
        continue;
    }
  }
}

void ESTreeIRGen::emitPrivateBrandCheck(Value *from, Value *brandVal) {
  auto *continueBB = Builder.createBasicBlock(Builder.getFunction());
  auto *throwBB = Builder.createBasicBlock(Builder.getFunction());
  Builder.createCondBranchInst(
      Builder.createBinaryOperatorInst(
          brandVal, from, ValueKind::BinaryPrivateInInstKind),
      continueBB,
      throwBB);
  Builder.setInsertionBlock(throwBB);
  Builder.createThrowTypeErrorInst(
      Builder.getLiteralString("Private element not found"));
  Builder.setInsertionBlock(continueBB);
}

Value *ESTreeIRGen::emitPrivateLookup(
    Value *from,
    Value *nameVal,
    ESTree::PrivateNameNode *nameNode) {
  auto *ID = llvh::cast<ESTree::IdentifierNode>(nameNode->_id);
  sema::Decl *decl = semCtx_.getExpressionDecl(ID);
  assert(sema::Decl::isKindPrivateName(decl->kind) && "private decl required");
  switch (decl->kind) {
    case sema::Decl::Kind::PrivateField:
      return Builder.createLoadOwnPrivateFieldInst(from, nameVal);
    case sema::Decl::Kind::PrivateMethod: {
      emitPrivateBrandCheck(from, nameVal);
      auto *entry =
          getDeclDataPrivate<PrivateNameFunctionTable::SingleFunctionEntry>(
              decl);
      return emitLoad(entry->functionObject, false);
    }
    case sema::Decl::Kind::PrivateSetter:
      emitPrivateBrandCheck(from, nameVal);
      Builder.createThrowTypeErrorInst(Builder.getLiteralString(
          "No field, method or getter with this name"));
      // Throwing is a terminal instruction, so make a new basic block to place
      // any IR following this private lookup.
      Builder.setInsertionBlock(
          Builder.createBasicBlock(Builder.getFunction()));
      return Builder.getLiteralUndefined();
      // For getters, invoke the function directly.
    case sema::Decl::Kind::PrivateGetter:
    case sema::Decl::Kind::PrivateGetterSetter: {
      Variable *getterFunctionObject =
          decl->kind == sema::Decl::Kind::PrivateGetter
          ? getDeclDataPrivate<PrivateNameFunctionTable::SingleFunctionEntry>(
                decl)
                ->functionObject
          : getDeclDataPrivate<PrivateNameFunctionTable::GetterSetterEntry>(
                decl)
                ->getterFunctionObject;
      emitPrivateBrandCheck(from, nameVal);
      auto *funcVal = emitLoad(getterFunctionObject, false);
      return Builder.createCallInst(
          funcVal,
          /* newTarget */ Builder.getLiteralUndefined(),
          /* thisValue */ from,
          /* args */ {});
    }
    default:
      assert(false && "unhandled private decl");
      return nullptr;
  }
}

void ESTreeIRGen::emitPrivateStore(
    Value *from,
    Value *storedValue,
    Value *nameVal,
    ESTree::PrivateNameNode *nameNode) {
  auto *ID = llvh::cast<ESTree::IdentifierNode>(nameNode->_id);
  sema::Decl *decl = semCtx_.getExpressionDecl(ID);
  assert(sema::Decl::isKindPrivateName(decl->kind) && "private decl required");
  switch (decl->kind) {
    case sema::Decl::Kind::PrivateField:
      Builder.createStoreOwnPrivateFieldInst(storedValue, from, nameVal);
      return;
    case sema::Decl::Kind::PrivateMethod: {
      emitPrivateBrandCheck(from, nameVal);
      Builder.createThrowTypeErrorInst(
          Builder.getLiteralString("Cannot overwrite a private method."));
      Builder.setInsertionBlock(
          Builder.createBasicBlock(Builder.getFunction()));
      return;
    }
    case sema::Decl::Kind::PrivateGetter:
      emitPrivateBrandCheck(from, nameVal);
      Builder.createThrowTypeErrorInst(
          Builder.getLiteralString("No field or setter with this name"));
      Builder.setInsertionBlock(
          Builder.createBasicBlock(Builder.getFunction()));
      return;
      // For setters, invoke the function directly.
    case sema::Decl::Kind::PrivateSetter:
    case sema::Decl::Kind::PrivateGetterSetter: {
      auto *setterFunctionObject = decl->kind == sema::Decl::Kind::PrivateSetter
          ? getDeclDataPrivate<PrivateNameFunctionTable::SingleFunctionEntry>(
                decl)
                ->functionObject
          : getDeclDataPrivate<PrivateNameFunctionTable::GetterSetterEntry>(
                decl)
                ->setterFunctionObject;
      emitPrivateBrandCheck(from, nameVal);
      auto *funcVal = emitLoad(setterFunctionObject, false);
      Builder.createCallInst(
          funcVal,
          Builder.getLiteralUndefined(),
          from,
          /* args */ {storedValue});
      return;
    }
    default:
      assert(false && "unhandled private decl");
  }
}

Value *ESTreeIRGen::genLegacyClassExpression(
    ESTree::ClassExpressionNode *node,
    Identifier nameHint) {
  // This is the possibly empty scope containing the class expression name.
  emitScopeDeclarations(node->getScope());
  return genLegacyClassLike(node, nameHint);
}

void ESTreeIRGen::genLegacyClassDeclaration(
    ESTree::ClassDeclarationNode *node) {
  emitScopeDeclarations(node->getScope());
  auto *CCI = genLegacyClassLike(node, Identifier{});
  // Legacy class declarations have two variables that are created. One for
  // after the class body, and one that is only visible inside the class body.
  // Here, we should initialize the outer variable, which can be found by
  // looking at the declaration decl.
  sema::Decl *decl =
      semCtx_.getDeclarationDecl(cast<ESTree::IdentifierNode>(node->_id));
  assert(decl && "ClassDeclarationNode missing a DeclarationDecl.");
  emitStore(CCI, getDeclData(decl), true);
}

CreateClassInst *ESTreeIRGen::genLegacyClassLike(
    ESTree::ClassLikeNode *classNode,
    Identifier nameHint) {
  ESTree::IdentifierNode *id = getClassID(classNode);
  Identifier className = id ? Identifier::getFromPointer(id->_name)
      : nameHint.isValid()  ? nameHint
                            : Builder.createIdentifier("");
  auto *superClassNode = getSuperClass(classNode);
  auto *classBody = getClassBody(classNode);
  Value *superCls = superClassNode ? genExpression(superClassNode)
                                   : Builder.getEmptySentinel();

  emitPrivateNameDeclarations(classNode->getScope(), className);
  auto *curScope = curFunction()->curScope;
  auto *curVarScope = curScope->getVariableScope();
  // Holds the .prototype of the class.
  Variable *clsPrototypeVar = Builder.createVariable(
      curScope->getVariableScope(),
      Builder.createIdentifier("?" + className.str() + ".prototype"),
      Type::createObject(),
      true);
  // Holds the class
  Variable *classVar = Builder.createVariable(
      curVarScope,
      Builder.createIdentifier("?" + className.str()),
      Type::createObject(),
      true);

  std::shared_ptr<LegacyClassContext> savedClsCtx =
      curFunction()->legacyClassContext;
  // Push a new class context.
  curFunction()->legacyClassContext =
      std::make_shared<LegacyClassContext>(classVar, nullptr);
  // Pop it when we are done generating IR for this class node.
  auto popCtx = llvh::make_scope_exit(
      [this, savedClsCtx] { curFunction()->legacyClassContext = savedClsCtx; });

  // Create the instance elements initializer.
  Function *instElemInitFunc =
      genLegacyInstanceElementsInit(classNode, clsPrototypeVar, className);
  if (instElemInitFunc) {
    CreateFunctionInst *createInstElemInitFunc =
        Builder.createCreateFunctionInst(
            curFunction()->curScope, instElemInitFunc);
    Variable *instElemInitFuncVar = Builder.createVariable(
        curFunction()->curScope->getVariableScope(),
        (llvh::Twine("<instElemInitFunc:") + className.str() + ">"),
        Type::createObject(),
        /* hidden */ true);
    Builder.createStoreFrameInst(
        curFunction()->curScope, createInstElemInitFunc, instElemInitFuncVar);
    curFunction()->legacyClassContext->instElemInitFuncVar =
        instElemInitFuncVar;
  }

  NormalFunction *constructorCode;
  // Generate the function code for the constructor- either from a user-provided
  // constructor or an implicit constructor we generate ourselves.
  if (ESTree::getDecoration<ESTree::ClassLikeDecoration>(classNode)
          ->implicitCtorFunctionInfo) {
    constructorCode =
        genLegacyImplicitConstructor(classNode, className, superClassNode);
  } else {
    ESTree::MethodDefinitionNode *consMethodNode = nullptr;
    for (auto &classElement : classBody->_body) {
      if (auto *method =
              llvh::dyn_cast<ESTree::MethodDefinitionNode>(&classElement);
          method && method->_kind == kw_.identConstructor) {
        consMethodNode = method;
        break;
      }
    }
    assert(consMethodNode && "no explicit or implicit constructor found");
    constructorCode = genBasicFunction(
        className,
        llvh::cast<ESTree::FunctionExpressionNode>(consMethodNode->_value),
        curScope->getVariableScope(),
        superClassNode,
        superClassNode ? Function::DefinitionKind::ES6DerivedConstructor
                       : Function::DefinitionKind::ES6BaseConstructor,
        clsPrototypeVar,
        consMethodNode);
  }
  AllocStackInst *clsPrototypeOutput = Builder.createAllocStackInst(
      genAnonymousLabelName("clsPrototype"), Type::createObject());
  CreateClassInst *createClass = Builder.createCreateClassInst(
      curScope, constructorCode, superCls, clsPrototypeOutput);
  auto *clsPrototype = Builder.createLoadStackInst(clsPrototypeOutput);

  /// Add a method to a given object \p O. In practice, O should either be the
  /// class itself or the class prototype.
  auto addMethod =
      [this](Value *O, llvh::StringRef kind, Value *key, Value *closure) {
        if (kind == "get") {
          Builder.createDefineOwnGetterSetterInst(
              closure,
              Builder.getLiteralUndefined(),
              O,
              key,
              IRBuilder::PropEnumerable::No);
        } else if (kind == "set") {
          Builder.createDefineOwnGetterSetterInst(
              Builder.getLiteralUndefined(),
              closure,
              O,
              key,
              IRBuilder::PropEnumerable::No);
        } else {
          assert(kind == "method" && "unhandled method definition");
          Builder.createDefineOwnPropertyInst(
              closure, O, key, IRBuilder::PropEnumerable::No);
        }
      };

  // Keep track of the first static private method. If one exists, then we will
  // need to stamp the class function object with the private static brand. The
  // way to get the value of the static brand is also through this AST node.
  ESTree::MethodDefinitionNode *firstStaticPrivateMethod = nullptr;
  // Space used to convert property keys to strings.
  llvh::SmallVector<char, 32> buffer;
  // Used to provide a slightly more helpful name for the `Variable*`s that hold
  // the value of a computed name.
  size_t computedKeyIdx = 0;
  // This loop is responsible for generating the value of all function class
  // elements, and field computed key values. Non-private static methods are
  // immediately added to the class.
  for (auto &classElement : classBody->_body) {
    if (auto *method =
            llvh::dyn_cast<ESTree::MethodDefinitionNode>(&classElement)) {
      if (method->_kind == kw_.identConstructor) {
        continue;
      }
      auto isStatic = method->_static;
      auto *homeObject = isStatic ? classVar : clsPrototypeVar;
      // Handle private methods & accessors. Here is where we generate the
      // function object value, and initialize the `Variable*` that is assocated
      // with each private name Decl.
      if (auto *PN = llvh::dyn_cast<ESTree::PrivateNameNode>(method->_key)) {
        auto *ID = llvh::cast<ESTree::IdentifierNode>(PN->_id);
        auto *funcValue = genFunctionExpression(
            llvh::cast<ESTree::FunctionExpressionNode>(method->_value),
            Mod->getContext().getPrivateNameIdentifier(ID->_name),
            superClassNode,
            Function::DefinitionKind::ES6Method,
            homeObject,
            method);
        sema::Decl *decl = semCtx_.getExpressionDecl(ID);
        // Handle private names that only define a single function.
        if (decl->kind == sema::Decl::Kind::PrivateMethod ||
            decl->kind == sema::Decl::Kind::PrivateGetter ||
            decl->kind == sema::Decl::Kind::PrivateSetter) {
          auto *entry =
              getDeclDataPrivate<PrivateNameFunctionTable::SingleFunctionEntry>(
                  decl);
          Builder.createStoreFrameInst(
              curScope, funcValue, entry->functionObject);
        } else {
          assert(
              decl->kind == sema::Decl::Kind::PrivateGetterSetter &&
              "unrecognized private decl kind");
          auto *entry =
              getDeclDataPrivate<PrivateNameFunctionTable::GetterSetterEntry>(
                  decl);
          // A name defining both a getter and setter defines two functions, one
          // for each accessor. Make sure we are initializing the correct
          // accessor.
          Builder.createStoreFrameInst(
              curScope,
              funcValue,
              method->_kind == kw_.identGet ? entry->getterFunctionObject
                                            : entry->setterFunctionObject);
        }
        // If this is the first instance private method discovered, set the
        // corresponding field in the class context.
        if (!method->_static &&
            !curFunction()->legacyClassContext->firstInstancePrivateMethod) {
          curFunction()->legacyClassContext->firstInstancePrivateMethod =
              method;
        }
        if (method->_static && !firstStaticPrivateMethod) {
          firstStaticPrivateMethod = method;
        }
        continue;
      }
      Value *key;
      Identifier nameHint{};
      if (method->_computed) {
        key = Builder.createToPropertyKeyInst(genExpression(method->_key));
      } else {
        nameHint = Mod->getContext().getIdentifier(
            propertyKeyAsString(buffer, method->_key));
        key = Builder.getLiteralString(nameHint);
      }
      auto *funcValue = genFunctionExpression(
          llvh::cast<ESTree::FunctionExpressionNode>(method->_value),
          nameHint,
          superClassNode,
          Function::DefinitionKind::ES6Method,
          homeObject,
          method);
      if (isStatic) {
        addMethod(createClass, method->_kind->str(), key, funcValue);
      } else {
        addMethod(clsPrototype, method->_kind->str(), key, funcValue);
      }
    } else if (
        auto *prop = llvh::dyn_cast<ESTree::ClassPropertyNode>(&classElement)) {
      if (prop->_computed) {
        Variable *fieldKeyVar = Builder.createVariable(
            curVarScope,
            Builder.createIdentifier(
                className.str() + "_computed_key_" + Twine(computedKeyIdx++)),
            Type::createAnyType(),
            /* hidden */ true);
        curFunction()->legacyClassContext->classComputedFieldKeys[prop] =
            fieldKeyVar;
        Builder.createStoreFrameInst(
            curScope,
            Builder.createToPropertyKeyInst(genExpression(prop->_key)),
            fieldKeyVar);
      }
    }
  }

  // Make the class name binding available after we've generated all computed
  // keys.
  if (id)
    emitStore(createClass, resolveIdentifier(id), true);
  // Initialize the class context variables.
  Builder.createStoreFrameInst(curScope, createClass, classVar);
  Builder.createStoreFrameInst(curScope, clsPrototype, clsPrototypeVar);

  // Make the static private methods available on the class *before* evaluating
  // the static elements initializer, e.g. the following code should work:
  //   class A {
  //     static f1 = this.#m1();
  //     static #m1() {}
  //   }
  if (firstStaticPrivateMethod) {
    auto *privateName =
        llvh::cast<ESTree::PrivateNameNode>(firstStaticPrivateMethod->_key);
    auto *staticBrand = genPrivateNameValue(
        llvh::cast<ESTree::IdentifierNode>(privateName->_id));
    // All we have to do to "install" the static methods & accessors is add the
    // private static brand to the class.
    Builder.createAddOwnPrivateFieldInst(
        Builder.getLiteralUndefined(), createClass, staticBrand);
  }

  if (Function *staticElementsCode =
          genStaticElementsInitFunction(classNode, className)) {
    CreateFunctionInst *staticElementsFunc = Builder.createCreateFunctionInst(
        curFunction()->curScope, staticElementsCode);
    Builder.createCallInst(
        staticElementsFunc,
        staticElementsCode,
        /* calleeIsAlwaysClosure */ Builder.getLiteralBool(true),
        /* env */ curFunction()->curScope,
        /*newTarget*/ Builder.getLiteralUndefined(),
        createClass,
        /*args*/ {});
  }

  return createClass;
}

Value *ESTreeIRGen::genLegacyDerivedThis() {
  assert(
      curFunction()->hasLegacyClassContext() &&
      "no legacy class context found.");
  assert(
      (semCtx_.nearestNonArrow(curFunction()->getSemInfo())->constructorKind ==
       sema::FunctionInfo::ConstructorKind::Derived) &&
      curFunction()->capturedState.thisVal &&
      "captured state not properly set.");
  auto *checkedThis = curFunction()->capturedState.thisVal;
  auto *baseConstructorScope =
      emitResolveScopeInstIfNeeded(checkedThis->getParent());
  return Builder.createThrowIfInst(
      Builder.createLoadFrameInst(baseConstructorScope, checkedThis),
      Type::createEmpty());
}

Value *ESTreeIRGen::genLegacyDirectSuper(ESTree::CallExpressionNode *call) {
  assert(
      curFunction()->hasLegacyClassContext() &&
      "no legacy class context found.");
  auto &LC = curFunction()->legacyClassContext;
  assert(LC->constructor && "class context not properly set.");
  assert(
      (semCtx_.nearestNonArrow(curFunction()->getSemInfo())->constructorKind ==
       sema::FunctionInfo::ConstructorKind::Derived) &&
      curFunction()->capturedState.thisVal &&
      "captured state not properly set.");
  assert(
      llvh::isa<ESTree::SuperNode>(getCallee(call)) &&
      "super is not being invoked");
  auto *newTarget = genNewTarget();
  auto *constructorScope =
      emitResolveScopeInstIfNeeded(LC->constructor->getParent());
  // We want to invoke the parent of the class we are generating.
  auto *callee = Builder.createLoadParentNoTrapsInst(
      Builder.createLoadFrameInst(constructorScope, LC->constructor));

  // We don't call into emitCall here because we need to generate the IR for the
  // arguments before generating the IR for the `this` value.
  bool hasSpread = llvh::any_of(getArguments(call), [](auto &arg) {
    return llvh::isa<ESTree::SpreadElementNode>(&arg);
  });
  Value *superRes;
  if (hasSpread) {
    auto *args = genArrayFromElements(getArguments(call));
    superRes = genBuiltinCall(
        BuiltinMethod::HermesBuiltin_applyWithNewTarget,
        {callee,
         args,
         Builder.createCreateThisInst(callee, newTarget),
         newTarget});
  } else {
    CallInst::ArgumentList args;
    for (auto &arg : getArguments(call)) {
      args.push_back(genExpression(&arg));
    }
    auto *thisVal = Builder.createCreateThisInst(callee, newTarget);
    auto *callInst = Builder.createCallInst(
        callee,
        Builder.getEmptySentinel(),
        false,
        /* env */ Builder.getEmptySentinel(),
        newTarget,
        thisVal,
        args);
    superRes = Builder.createGetConstructedObjectInst(thisVal, callInst);
  }
  // A super call always returns object.
  superRes->setType(Type::createObject());

  auto *checkedThis = curFunction()->capturedState.thisVal;
  auto *checkedThisScope =
      emitResolveScopeInstIfNeeded(checkedThis->getParent());
  Builder.createThrowIfThisInitializedInst(
      Builder.createLoadFrameInst(checkedThisScope, checkedThis));
  Builder.createStoreFrameInst(checkedThisScope, superRes, checkedThis);
  // After a successful call to super, run the field initializer function.
  emitLegacyInstanceElementsInitCall();
  return superRes;
}

Value *ESTreeIRGen::genLegacyDerivedConstructorRet(
    ESTree::ReturnStatementNode *node,
    Value *returnValue) {
  // Easy optimization- if we are returning `this`, then we already do the
  // required checks when generating the IR for the `this` expression.
  if (node && node->_argument &&
      llvh::isa<ESTree::ThisExpressionNode>(node->_argument)) {
    return returnValue;
  }
  assert(
      (curFunction()->getSemInfo()->constructorKind ==
       sema::FunctionInfo::ConstructorKind::Derived) &&
      curFunction()->capturedState.thisVal &&
      "captured state not properly set.");
  auto *checkedThis = curFunction()->capturedState.thisVal;
  auto *baseConstructorScope =
      emitResolveScopeInstIfNeeded(checkedThis->getParent());

  // Easy optimization- we will often be returning a literal undefined because
  // of implicit returns. Returning undefined is functionally equivalent to
  // returning `this`.
  if (llvh::isa<LiteralUndefined>(returnValue)) {
    return Builder.createThrowIfInst(
        Builder.createLoadFrameInst(baseConstructorScope, checkedThis),
        Type::createEmpty());
  }

  // Every return statement in a derived class constructor must be either an
  // object or undefined. If it's undefined, then convert that to the checked
  // this. Conceptually then, we generate IR like the following:
  //
  //   if (returnValue is object || returnValue is function) {
  //     return returnValue;
  //   } else if (returnValue === undefined) {
  //     return checkedThis();
  //   } else {
  //     throw new TypeError();
  //   }

  auto *curBB = Builder.getInsertionBlock();
  auto *returnBB = Builder.createBasicBlock(curFunction()->function);
  auto *checkIfUndefBB = Builder.createBasicBlock(curFunction()->function);
  auto *throwInvalidRetBB = Builder.createBasicBlock(curFunction()->function);
  auto *loadCheckedThisBB = Builder.createBasicBlock(curFunction()->function);
  auto *canUseRetVal = Builder.createTypeOfIsInst(
      returnValue,
      Builder.getLiteralTypeOfIsTypes(
          TypeOfIsTypes{}.withFunction(true).withObject(true)));

  // 10.a. If result.[[Value]] is an Object, return result.[[Value]].
  Builder.createCondBranchInst(canUseRetVal, returnBB, checkIfUndefBB);

  // 10.c. If result.[[Value]] is not undefined, throw a TypeError exception.
  Builder.setInsertionBlock(checkIfUndefBB);
  Builder.createCondBranchInst(
      Builder.createBinaryOperatorInst(
          returnValue,
          Builder.getLiteralUndefined(),
          ValueKind::BinaryStrictlyEqualInstKind),
      loadCheckedThisBB,
      throwInvalidRetBB);

  // 12. Let thisBinding be ? constructorEnv.GetThisBinding().
  // 13. Assert: thisBinding is an Object.
  // 14. Return thisBinding.
  Builder.setInsertionBlock(loadCheckedThisBB);
  auto *checkedThisVal = Builder.createThrowIfInst(
      Builder.createLoadFrameInst(baseConstructorScope, checkedThis),
      Type::createEmpty());
  Builder.createBranchInst(returnBB);

  Builder.setInsertionBlock(throwInvalidRetBB);
  Builder.createThrowTypeErrorInst(Builder.getLiteralString(
      "Derived constructors may only return an object or undefined."));

  Builder.setInsertionBlock(returnBB);
  return Builder.createPhiInst(
      {returnValue, checkedThisVal}, {curBB, loadCheckedThisBB});
}

NormalFunction *ESTreeIRGen::genLegacyImplicitConstructor(
    ESTree::ClassLikeNode *classNode,
    const Identifier &className,
    ESTree::Node *superClassNode) {
  const auto &LC = curFunction()->legacyClassContext;
  assert(LC->constructor && "class context not properly set");

  // Retrieve the FunctionInfo for the implicit constructor, which must exist.
  sema::FunctionInfo *funcInfo =
      ESTree::getDecoration<ESTree::ClassLikeDecoration>(classNode)
          ->implicitCtorFunctionInfo;
  assert(
      funcInfo &&
      "Semantic resolver failed to decorate class with implicit ctor");

  auto *consFunc = Builder.createFunction(
      className,
      funcInfo->constructorKind == sema::FunctionInfo::ConstructorKind::Derived
          ? Function::DefinitionKind::ES6DerivedConstructor
          : Function::DefinitionKind::ES6BaseConstructor,
      /* strictMode */ true,
      funcInfo->customDirectives);

  auto compileFunc = [this,
                      consFunc,
                      funcInfo,
                      LC,
                      superClassNode,
                      isDerived = funcInfo->constructorKind ==
                          sema::FunctionInfo::ConstructorKind::Derived,
                      parentScope =
                          curFunction()->curScope->getVariableScope()] {
    FunctionContext newFunctionContext{this, consFunc, funcInfo};
    newFunctionContext.superClassNode_ = superClassNode;
    newFunctionContext.legacyClassContext = LC;

    auto *entryBB = Builder.createBasicBlock(consFunc);
    emitFunctionPrologue(
        nullptr,
        entryBB,
        InitES5CaptureState::No,
        DoEmitDeclarations::No,
        parentScope);
    // Default subclass constructors must call super.
    if (isDerived) {
      // All derived constructors must have a captured state `this`.
      curFunction()->capturedState.thisVal = Builder.createVariable(
          curFunction()->curScope->getVariableScope(),
          Builder.createIdentifier("?CHECKED_this"),
          Type::createObject(),
          true);
      auto *baseConstructorScope =
          emitResolveScopeInstIfNeeded(LC->constructor->getParent());
      auto *callee = Builder.createLoadParentNoTrapsInst(
          Builder.createLoadFrameInst(baseConstructorScope, LC->constructor));
      auto newTarget = Builder.createGetNewTargetInst(
          curFunction()->function->getNewTargetParam());
      auto *createdThis = Builder.createCreateThisInst(callee, newTarget);
      // Emit IR equivalent to super(...arguments)
      auto *initializedThisVal = genBuiltinCall(
          BuiltinMethod::HermesBuiltin_applyArguments,
          {callee, createdThis, newTarget});
      // Construct call always returns object.
      initializedThisVal->setType(Type::createObject());
      Builder.createStoreFrameInst(
          curFunction()->curScope,
          initializedThisVal,
          newFunctionContext.capturedState.thisVal);
      emitLegacyInstanceElementsInitCall();
      // Returning undefined in a derived constructor is coerced into returning
      // the `thisVal` we just set.
      emitFunctionEpilogue(Builder.getLiteralUndefined());
      consFunc->setReturnType(Type::createObject());
    } else {
      emitLegacyInstanceElementsInitCall();
      emitFunctionEpilogue(Builder.getLiteralUndefined());
      consFunc->setReturnType(Type::createUndefined());
    }
  };
  enqueueCompilation(
      classNode, ExtraKey::ImplicitClassConstructor, consFunc, compileFunc);
  return consFunc;
}

NormalFunction *ESTreeIRGen::genStaticElementsInitFunction(
    ESTree::ClassLikeNode *legacyClassNode,
    const Identifier &consName) {
  sema::FunctionInfo *staticElementsFuncInfo =
      ESTree::getDecoration<ESTree::ClassLikeDecoration>(legacyClassNode)
          ->staticElementsInitFunctionInfo;
  if (staticElementsFuncInfo == nullptr) {
    return nullptr;
  }

  auto *staticElementsFunc = Builder.createFunction(
      (llvh::Twine("<static_elements_initializer:") + consName.str() + ">")
          .str(),
      Function::DefinitionKind::ES5Function,
      true /*strictMode*/);

  auto compileFunc = [this,
                      staticElementsFunc,
                      staticElementsFuncInfo,
                      legacyClassNode = legacyClassNode,
                      typedClassContext = curFunction()->typedClassContext,
                      legacyClassContext = curFunction()->legacyClassContext,
                      parentScope =
                          curFunction()->curScope->getVariableScope()] {
    FunctionContext newFunctionContext{
        this, staticElementsFunc, staticElementsFuncInfo};
    newFunctionContext.typedClassContext = typedClassContext;
    newFunctionContext.legacyClassContext = legacyClassContext;
    newFunctionContext.capturedState.homeObject =
        legacyClassContext->constructor;

    auto *prologueBB = Builder.createBasicBlock(staticElementsFunc);
    Builder.setInsertionBlock(prologueBB);

    emitFunctionPrologue(
        nullptr,
        prologueBB,
        InitES5CaptureState::Yes,
        DoEmitDeclarations::No,
        parentScope);
    auto &LC = curFunction()->legacyClassContext;
    auto *constructorScope =
        emitResolveScopeInstIfNeeded(LC->constructor->getParent());
    auto *classVal =
        Builder.createLoadFrameInst(constructorScope, LC->constructor);

    auto *classBody = ESTree::getClassBody(legacyClassNode);
    llvh::SmallVector<char, 32> buffer;

    // Emit a store to the constructor object for each static property on the
    // class.
    for (ESTree::Node &it : classBody->_body) {
      if (auto *prop = llvh::dyn_cast<ESTree::ClassPropertyNode>(&it)) {
        if (!prop->_static)
          continue;
        Value *propKey;
        Identifier nameHint;
        if (prop->_computed) {
          // use .at, since there should always be an entry for this node.
          Variable *fieldKeyVar = LC->classComputedFieldKeys.at(prop);
          auto *scope = emitResolveScopeInstIfNeeded(fieldKeyVar->getParent());
          propKey = Builder.createLoadFrameInst(scope, fieldKeyVar);
        } else {
          if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(prop->_key)) {
            nameHint = Identifier::getFromPointer(id->_name);
          }
          propKey =
              Builder.getLiteralString(propertyKeyAsString(buffer, prop->_key));
        }
        Value *propValue = prop->_value ? genExpression(prop->_value, nameHint)
                                        : Builder.getLiteralUndefined();
        Builder.createDefineOwnPropertyInst(
            propValue, classVal, propKey, IRBuilder::PropEnumerable::Yes);
        continue;
      }
      if (auto *prop = llvh::dyn_cast<ESTree::ClassPrivatePropertyNode>(&it)) {
        if (!prop->_static)
          continue;
        auto *privateNameID = llvh::cast<ESTree::IdentifierNode>(prop->_key);
        auto *propKey = genPrivateNameValue(privateNameID);
        Value *propValue = prop->_value
            ? genExpression(
                  prop->_value,
                  Builder.getModule()->getContext().getPrivateNameIdentifier(
                      privateNameID->_name))
            : Builder.getLiteralUndefined();
        Builder.createAddOwnPrivateFieldInst(propValue, classVal, propKey);
        continue;
      }
    }

    emitFunctionEpilogue(Builder.getLiteralUndefined());
    staticElementsFunc->setReturnType(Type::createUndefined());
  };
  enqueueCompilation(
      legacyClassNode,
      ExtraKey::ImplicitStaticElementsInitializer,
      staticElementsFunc,
      compileFunc);
  return staticElementsFunc;
}

NormalFunction *ESTreeIRGen::genLegacyInstanceElementsInit(
    ESTree::ClassLikeNode *legacyClassNode,
    Variable *homeObjectVar,
    const Identifier &consName) {
  sema::FunctionInfo *initFuncInfo =
      ESTree::getDecoration<ESTree::ClassLikeDecoration>(legacyClassNode)
          ->instanceElementsInitFunctionInfo;
  if (initFuncInfo == nullptr) {
    return nullptr;
  }

  auto *initFunc = Builder.createFunction(
      (llvh::Twine("<instance_members_initializer:") + consName.str() + ">")
          .str(),
      Function::DefinitionKind::ES5Function,
      true /*strictMode*/);

  auto compileFunc = [this,
                      initFunc,
                      initFuncInfo,
                      legacyClassNode = legacyClassNode,
                      typedClassContext = curFunction()->typedClassContext,
                      legacyClassContext = curFunction()->legacyClassContext,
                      parentScope = curFunction()->curScope->getVariableScope(),
                      homeObjectVar] {
    FunctionContext newFunctionContext{this, initFunc, initFuncInfo};
    newFunctionContext.typedClassContext = typedClassContext;
    newFunctionContext.legacyClassContext = legacyClassContext;
    newFunctionContext.capturedState.homeObject = homeObjectVar;

    auto *prologueBB = Builder.createBasicBlock(initFunc);
    Builder.setInsertionBlock(prologueBB);

    emitFunctionPrologue(
        nullptr,
        prologueBB,
        InitES5CaptureState::Yes,
        DoEmitDeclarations::No,
        parentScope);
    auto &LC = curFunction()->legacyClassContext;
    auto *thisParam = curFunction()->jsParams[0];

    bool hasCheckedDoubleInit = false;
    /// Before adding any private elements, we must make sure we haven't already
    /// run through this constructor before. This function emits the necessary
    /// IR to do this check.
    /// \param propKey is the private name used to determine when a constructor
    /// has already run.
    auto checkForDoubleInit =
        [this, &hasCheckedDoubleInit, initFunc, thisParam](Value *propKey) {
          if (hasCheckedDoubleInit)
            return;
          auto *continueBB = Builder.createBasicBlock(initFunc);
          auto *throwBB = Builder.createBasicBlock(initFunc);
          Builder.createCondBranchInst(
              Builder.createBinaryOperatorInst(
                  propKey, thisParam, ValueKind::BinaryPrivateInInstKind),
              throwBB,
              continueBB);
          Builder.setInsertionBlock(throwBB);
          Builder.createThrowTypeErrorInst(Builder.getLiteralString(
              "Cannot initialize private field twice."));
          Builder.setInsertionBlock(continueBB);
          hasCheckedDoubleInit = true;
        };

    // If there is a private instance method, then this instance must be stamped
    // with the private instance brand. This should be done before any other
    // part of the constructor runs, because all other user JS should be able to
    // access these private methods, e.g. the expression of a private field
    // initializer.
    if (auto *method = LC->firstInstancePrivateMethod) {
      auto *privateName = llvh::cast<ESTree::PrivateNameNode>(method->_key);
      auto *instanceBrand = genPrivateNameValue(
          llvh::cast<ESTree::IdentifierNode>(privateName->_id));
      checkForDoubleInit(instanceBrand);
      Builder.createAddOwnPrivateFieldInst(
          Builder.getLiteralUndefined(), thisParam, instanceBrand);
    }

    auto *classBody = ESTree::getClassBody(legacyClassNode);
    llvh::SmallVector<char, 32> buffer;
    // Emit a store to the constructor object for each instance property on the
    // class.
    for (ESTree::Node &it : classBody->_body) {
      if (auto *prop = llvh::dyn_cast<ESTree::ClassPropertyNode>(&it)) {
        if (prop->_static)
          continue;
        Value *propKey;
        Identifier nameHint;
        if (prop->_computed) {
          Variable *fieldKeyVar = LC->classComputedFieldKeys.at(prop);
          auto *scope = emitResolveScopeInstIfNeeded(fieldKeyVar->getParent());
          propKey = Builder.createLoadFrameInst(scope, fieldKeyVar);
        } else {
          if (auto *ID = llvh::dyn_cast<ESTree::IdentifierNode>(prop->_key)) {
            nameHint = Identifier::getFromPointer(ID->_name);
          }
          propKey =
              Builder.getLiteralString(propertyKeyAsString(buffer, prop->_key));
        }
        Value *propValue = prop->_value ? genExpression(prop->_value, nameHint)
                                        : Builder.getLiteralUndefined();
        Builder.createDefineOwnPropertyInst(
            propValue, thisParam, propKey, IRBuilder::PropEnumerable::Yes);
        continue;
      }
      if (auto *prop = llvh::dyn_cast<ESTree::ClassPrivatePropertyNode>(&it)) {
        if (prop->_static)
          continue;
        auto *privateNameID = llvh::cast<ESTree::IdentifierNode>(prop->_key);
        auto *propKey = genPrivateNameValue(privateNameID);
        Value *propValue = prop->_value
            ? genExpression(
                  prop->_value,
                  Mod->getContext().getPrivateNameIdentifier(
                      privateNameID->_name))
            : Builder.getLiteralUndefined();
        // Make sure to emit this after we run the side effects of the value
        // initializer.
        checkForDoubleInit(propKey);
        Builder.createAddOwnPrivateFieldInst(propValue, thisParam, propKey);
        continue;
      }
    }

    emitFunctionEpilogue(Builder.getLiteralUndefined());
    initFunc->setReturnType(Type::createUndefined());
  };
  enqueueCompilation(
      legacyClassNode,
      ExtraKey::ImplicitFieldInitializer,
      initFunc,
      compileFunc);
  return initFunc;
}

void ESTreeIRGen::emitLegacyInstanceElementsInitCall() {
  assert(
      curFunction()->legacyClassContext != nullptr && "no class context found");
  const auto &LC = curFunction()->legacyClassContext;
  auto *instElemInitFuncVar = LC->instElemInitFuncVar;
  // It's valid for a class to have no instance elements initialzer. In this
  // case, do nothing.
  if (!instElemInitFuncVar)
    return;
  auto *scope = emitResolveScopeInstIfNeeded(instElemInitFuncVar->getParent());
  Value *funcVal = Builder.createLoadFrameInst(scope, instElemInitFuncVar);
  funcVal->setType(Type::createObject());
  Builder
      .createCallInst(
          funcVal,
          /* target */ Builder.getEmptySentinel(),
          /* calleeIsAlwaysClosure */ Builder.getLiteralBool(true),
          /* env */ Builder.getEmptySentinel(),
          /* newTarget */ Builder.getLiteralUndefined(),
          genThisExpression(),
          /* args */ {})
      ->setType(Type::createUndefined());
}

} // namespace irgen
} // namespace hermes
