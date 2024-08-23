#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/Parser/JSLexer.h"
#include "llvh/ADT/StringRef.h"

namespace {
using namespace hermes;

/// Mutable vector that helps dealing with arrays of nodes safely.
/// Once done with the vector, it can create an ESTree::NodeList
/// representation which is used by the ESTree API in several places.
class NodeVector {
 public:
  using Storage = llvh::SmallVector<ESTree::Node *, 8>;

  NodeVector() = default;
  NodeVector(std::initializer_list<ESTree::Node *> nodes) {
    for (auto &node : nodes) {
      _storage.push_back(node);
    }
  }

  NodeVector(ESTree::NodeList &list) {
    for (auto &node : list) {
      _storage.push_back(&node);
    }
  }

  ~NodeVector() = default;

  size_t size() const {
    return _storage.size();
  }

  Storage::const_iterator begin() const {
    return _storage.begin();
  }

  Storage::const_iterator end() const {
    return _storage.end();
  }

  void append(ESTree::Node *node) {
    _storage.emplace_back(node);
  }

  void prepend(ESTree::Node *node) {
    _storage.insert(_storage.begin(), node);
  }

  ESTree::NodeList toNodeList() const {
    ESTree::NodeList nodeList;
    for (auto &node : _storage) {
      nodeList.push_back(*node);
    }
    return nodeList;
  }

 private:
  Storage _storage;
};

class TransformationsBase
    : public ESTree::RecursionDepthTracker<TransformationsBase> {
 public:
  static constexpr bool kEnableNodeListMutation = true;

  TransformationsBase(Context &context)
      : context_(context),
        identLet_(context.getIdentifier("let").getUnderlyingPointer()) {}

  void recursionDepthExceeded(ESTree::Node *n) {
    context_.getSourceErrorManager().error(
        n->getEndLoc(), "Too many nested expressions/statements/declarations");
  }

 protected:
  Context &context_;
  UniqueString *const identLet_;

  void doCopyLocation(ESTree::Node *src, ESTree::Node *dest) {
    if (src != nullptr) {
      dest->setStartLoc(src->getStartLoc());
      dest->setEndLoc(src->getEndLoc());
      dest->setDebugLoc(src->getDebugLoc());
    }
  }

  template <typename T>
  T *copyLocation(ESTree::Node *src, T *dest) {
    doCopyLocation(src, dest);
    return dest;
  }

  template <typename T, typename... Args>
  T *createTransformedNode(ESTree::Node *src, Args &&...args) {
    auto *node = new (context_) T(std::forward<Args>(args)...);
    return copyLocation(src, node);
  }

  ESTree::IdentifierNode *makeIdentifierNode(
      ESTree::Node *srcNode,
      UniqueString *name) {
    return createTransformedNode<ESTree::IdentifierNode>(
        srcNode, name, nullptr, false);
  }

  ESTree::IdentifierNode *makeIdentifierNode(
      ESTree::Node *srcNode,
      llvh::StringRef name) {
    return makeIdentifierNode(
        srcNode, context_.getIdentifier(name).getUnderlyingPointer());
  }

  ESTree::Node *makeSingleLetDecl(
      ESTree::Node *srcNode,
      ESTree::Node *identifier,
      ESTree::Node *value) {
    auto *variableDeclarator =
        createTransformedNode<ESTree::VariableDeclaratorNode>(
            srcNode, value, identifier);
    ESTree::NodeList variableList;
    variableList.push_back(*variableDeclarator);
    return createTransformedNode<ESTree::VariableDeclarationNode>(
        srcNode, identLet_, std::move(variableList));
  }

  ESTree::Node *makeHermesInternalCall(
      ESTree::Node *srcNode,
      llvh::StringRef methodName,
      const NodeVector &parameters) {
    auto hermesInternalIdentifier = getHermesInternalIdentifier(srcNode);
    auto methodIdentifier = makeIdentifierNode(srcNode, methodName);

    auto *getPropertyNode = createTransformedNode<ESTree::MemberExpressionNode>(
        srcNode, hermesInternalIdentifier, methodIdentifier, false);
    return createTransformedNode<ESTree::CallExpressionNode>(
        srcNode, getPropertyNode, nullptr, parameters.toNodeList());
  }

  virtual ESTree::Node *getHermesInternalIdentifier(ESTree::Node *srcNode) {
    return nullptr;
  };
};
} // namespace
