#include "ESTreeIRGen.h"

#include "hermes/IR/Instrs.h"
#include "llvh/ADT/SmallString.h"

namespace hermes {
namespace irgen {

void ESTreeIRGen::genWithStatement(ESTree::WithStatementNode *with) {
  WithScopeInfo withScope{};
  withScope.depth = semCtx_.getWithLexicalDepth(with);

  withScope.object = Builder.createVariable(
      curFunction()->curScope->getVariableScope(),
      Builder
          .getLiteralString(
              "with" + std::to_string(withScope.depth))
          ->getValue(),
      Type::createAnyType(),
      true);
  withScope.object->setIsConst(true);
  emitStore(genExpression(with->_object), withScope.object, true);

  withScopes_.push_back(withScope);
  genStatement(with->_body);
  withScopes_.pop_back();
}

Value *ESTreeIRGen::emitLoadOrStoreWithStatementImpl(
    Value *value,
    Value *ptr,
    bool declInit_,
    ESTree::IdentifierNode *id,
    ConditionalChainType conditionalChainType,
    bool inhibitThrow) {
  uint32_t identifierDepth = INT_MAX;
  std::string_view name;

  identifierDepth = getIDDecl(id)->scope->depth;
  name = id->_name->c_str();

  auto compareDepth = [](uint32_t searching, const WithScopeInfo &scope) {
    return scope.depth > searching;
  };

  auto it = std::upper_bound(
      withScopes_.begin(), withScopes_.end(), identifierDepth, compareDepth);

  return createConditionalChainImpl(
      it,
      withScopes_.end(),
      ptr,
      value,
      declInit_,
      name,
      conditionalChainType,
      inhibitThrow);
}

Value *ESTreeIRGen::createConditionalChainImpl(
    std::vector<WithScopeInfo>::iterator begin,
    std::vector<WithScopeInfo>::iterator end,
    Value *ptr,
    Value *value,
    bool declInit_,
    std::string_view name,
    ConditionalChainType conditionalChainType,
    bool inhibitThrow) {
  if (begin == end) {
    if (conditionalChainType ==
        ConditionalChainType::OBJECT_ONLY_WITH_UNDEFINED_ALTERNATE) {
      return Builder.getLiteralUndefined();
    } else if (
        conditionalChainType ==
        ConditionalChainType::OBJECT_ONLY_WITH_GLOBAL_ALTERNATE) {
      return Builder.getGlobalObject();
    } else if (value) {
      return emitStore(value, ptr, declInit_);
    } else {
      return emitLoad(ptr, inhibitThrow);
    }
  }

  auto &current = *(end - 1);
  auto conditionGenerator = [&]() -> Value * {
    auto *wrapper = Builder.createLoadPropertyInst(
        Builder.getGlobalObject(), "HermesWithInternal");
    wrapper = Builder.createLoadPropertyInst(wrapper, "_containsField");

    auto *call = Builder.createCallInst(
        wrapper,
        Builder.getLiteralUndefined(),
        Builder.getLiteralUndefined(),
        {emitLoad(current.object, false),
         Builder.getLiteralString(name.data())});

    return call;
  };

  auto consequentGenerator = [&]() -> Value * {
    auto loadWithObj = emitLoad(current.object, false);

    if (conditionalChainType == ConditionalChainType::MEMBER_EXPRESSION) {
      if (value) {
        return Builder.createStorePropertyInst(value, loadWithObj, name.data());
      } else {
        return Builder.createLoadPropertyInst(loadWithObj, name.data());
      }
    }
    return loadWithObj;
  };

  auto alternateGenerator = [&]() -> Value * {
    return createConditionalChainImpl(
        begin, end - 1,
        ptr,
        value,
        declInit_,
        name,
        conditionalChainType,
        inhibitThrow);
  };

  return genConditionalExpr(
      conditionGenerator, consequentGenerator, alternateGenerator);
}

Value *ESTreeIRGen::withAwareEmitLoad(
    hermes::Value *ptr,
    ESTree::Node *node,
    ConditionalChainType conditionalChainType,
    bool inhibitThrow) {
  auto *identifier =
      llvh::dyn_cast_or_null<ESTree::IdentifierNode>(node);
  if (!identifier || withScopes_.empty()) {
    if (!ptr)
      return Builder.getLiteralUndefined();
    return emitLoad(ptr, inhibitThrow);
  }
  return emitLoadOrStoreWithStatementImpl(
      nullptr, ptr, false, identifier, conditionalChainType, inhibitThrow);
}

Value *ESTreeIRGen::withAwareEmitStore(
    Value *storedValue,
    Value *ptr,
    bool declInit_,
    ESTree::Node *node) {
  auto *identifier =
      llvh::dyn_cast_or_null<ESTree::IdentifierNode>(node);
  if (!identifier || withScopes_.empty()) {
    return emitStore(storedValue, ptr, declInit_);
  }
  return emitLoadOrStoreWithStatementImpl(
      storedValue,
      ptr,
      declInit_,
      identifier,
      ConditionalChainType::MEMBER_EXPRESSION);
}

} // namespace irgen
} // namespace hermes
