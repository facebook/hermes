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

template <typename Callback>
Value *ESTreeIRGen::createWithConditionalChain(
    ESTree::Node *node,
    const Callback &callback) {
  ESTree::IdentifierNode *identifier = !withScopes_.empty() && node
      ? llvh::dyn_cast<ESTree::IdentifierNode>(node)
      : nullptr;
  if (!identifier) {
    return callback(nullptr, "");
  }

  uint32_t identifierDepth = INT_MAX;
  identifierDepth = getIDDecl(identifier)->scope->depth;
  std::string_view name = identifier->_name->c_str();

  auto compareDepth = [](uint32_t searching, const WithScopeInfo &scope) {
    return scope.depth > searching;
  };

  auto it = std::upper_bound(
      withScopes_.begin(), withScopes_.end(), identifierDepth, compareDepth);

  return createWithConditionalChainImpl(it, withScopes_.end(), callback, name);
}

template <typename Callback>
Value *ESTreeIRGen::createWithConditionalChainImpl(
    std::vector<WithScopeInfo>::iterator begin,
    std::vector<WithScopeInfo>::iterator end,
    const Callback &callback,
    std::string_view name) {
  if (begin == end) {
    return callback(nullptr, name);
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
    return callback(loadWithObj, name);
  };

  auto alternateGenerator = [&]() -> Value * {
    return createWithConditionalChainImpl(
        begin, end - 1, callback, name);
  };

  return genConditionalExpr(
      conditionGenerator, consequentGenerator, alternateGenerator);
}

Value *ESTreeIRGen::withAwareEmitLoad(
    hermes::Value *ptr,
    ESTree::Node *node,
    ConditionalChainType conditionalChainType,
    bool inhibitThrow) {
  return createWithConditionalChain(
      node, [&](Value *withObject, std::string_view idName) -> Value * {
        switch (conditionalChainType) {
          case ConditionalChainType::OBJECT_ONLY_WITH_UNDEFINED_ALTERNATE:
            return withObject ? withObject : Builder.getLiteralUndefined();
          case ConditionalChainType::OBJECT_ONLY_WITH_GLOBAL_ALTERNATE:
            return withObject ? withObject : Builder.getGlobalObject();
          case ConditionalChainType::MEMBER_EXPRESSION:
            return withObject
                ? Builder.createLoadPropertyInst(withObject, idName.data())
                : emitLoad(ptr, inhibitThrow);
          default:
            llvm_unreachable("Unhandled conditionalChainType");
        }
      });
}

void ESTreeIRGen::withAwareEmitStore(
    Value *storedValue,
    Value *ptr,
    bool declInit_,
    ESTree::Node *node) {
  createWithConditionalChain(
      node, [&](Value *withObject, std::string_view idName) {
        if (!withObject)
          emitStore(storedValue, ptr, declInit_);
        else
          Builder.createStorePropertyInst(
              storedValue, withObject, idName.data());
        return Builder.getLiteralUndefined();
      });
}

} // namespace irgen
} // namespace hermes
