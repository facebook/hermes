/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdio>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Component {
  virtual void rtti() {}
};

class NumberComponent : public Component {
 public:
  int x;

  explicit NumberComponent(int x) : x(x) {}
};

class StringComponent : public Component {
 public:
  const char *x;

  explicit StringComponent(const char *x) : x(x) {}
};

class RenderNode;

class Context;

class Widget {
 public:
  std::optional<const char *> key;

  virtual RenderNode *reduce(Context *ctx) = 0;

  virtual const char *getName() {
    return "Widget";
  }
};

class Context {
 public:
  const char *key;
  int childCounter = 0;

  explicit Context(const char *key) : key(key) {}

  static Context *createForChild(Context *parentCtx, Widget *child) {
    auto widgetKey = child->key;
    const char *childKey;
    if (widgetKey)
      childKey = *widgetKey;
    else {
      auto *name = typeid(*child).name();
      // +1 for null terminator
      char *buf = (char *)malloc(strlen(name) + 11);
      sprintf(buf, "%s_%d", name, parentCtx->childCounter++);
      childKey = buf;
    }
    // +1 for underscore, +1 for null terminator
    char *newKey =
        (char *)malloc(strlen(parentCtx->key) + strlen(childKey) + 2);
    sprintf(newKey, "%s_%s", parentCtx->key, childKey);
    return new Context(newKey);
  }
};

using VirtualEntity = std::pair<int, std::vector<Component *> *>;

class RenderNode {
 public:
  const char *key;
  int id;
  std::vector<Component *> *components;
  std::vector<RenderNode *> *children;
  inline static int idCounter = 0;

  explicit RenderNode(
      const char *key,
      int id,
      std::vector<Component *> *components,
      std::optional<std::vector<RenderNode *> *> children)
      : key(key),
        id(id),
        components(components),
        children(children ? *children : new std::vector<RenderNode *>()) {}

  std::vector<VirtualEntity *> *reduce() {
    auto *res = new std::vector<VirtualEntity *>();
    res->push_back(new VirtualEntity{id, components});
    for (auto *child : *children)
      for (auto *v : *child->reduce())
        res->push_back(v);
    return res;
  }

  static RenderNode *create(
      Context *ctx,
      std::vector<Component *> *components,
      std::optional<std::vector<RenderNode *> *> children) {
    return new RenderNode(ctx->key, idCounter++, components, children);
  }

  static RenderNode *createForChild(Context *ctx, Widget *child) {
    auto *childCtx = Context::createForChild(ctx, child);
    return child->reduce(childCtx);
  }
};

class ComposedWidget : public Widget {
 public:
  virtual Widget *render() = 0;

  RenderNode *reduce(Context *ctx) override {
    auto *child = render();
    return child->reduce(ctx);
  }
};

class Button : public Widget {
 public:
  int num;

  explicit Button(int num) : num(num) {}

  RenderNode *reduce(Context *ctx) override {
    auto *component = new NumberComponent(num);
    auto *components = new std::vector<Component *>();
    components->push_back(component);
    return RenderNode::create(ctx, components, std::nullopt);
  }
};

class Floater : public Widget {
 public:
  int num;

  explicit Floater(int num) : num(num) {}

  RenderNode *reduce(Context *ctx) override {
    auto *component = new NumberComponent(num);
    auto *components = new std::vector<Component *>();
    components->push_back(component);
    return RenderNode::create(ctx, components, std::nullopt);
  }
};

class Gltf : public Widget {
 public:
  const char *path;

  explicit Gltf(const char *path) : path(path) {}

  RenderNode *reduce(Context *ctx) override {
    auto *component = new StringComponent(path);
    auto *components = new std::vector<Component *>();
    components->push_back(component);
    return RenderNode::create(ctx, components, std::nullopt);
  }
};

class Container : public Widget {
 public:
  std::vector<Widget *> *children;

  explicit Container(std::vector<Widget *> *children) : children(children) {}

  RenderNode *reduce(Context *ctx) override {
    auto *component = new NumberComponent(13);
    auto *mappedChildren = new std::vector<RenderNode *>();
    for (auto *child : *children)
      mappedChildren->push_back(RenderNode::createForChild(ctx, child));
    auto *components = new std::vector<Component *>();
    components->push_back(component);
    return RenderNode::create(ctx, components, mappedChildren);
  }
};

struct RenderData {
  const char *modelPath;
  int buttonSize;
};

class ButtonAndModel : public ComposedWidget {
 public:
  RenderData *data;

  explicit ButtonAndModel(RenderData *data) : data(data) {}

  Widget *render() override {
    auto *children = new std::vector<Widget *>();
    children->push_back(new Button(data->buttonSize));
    children->push_back(new Gltf(data->modelPath));
    children->push_back(new Floater(sqrt(data->buttonSize)));

    return new Container(children);
  }
};

static std::vector<int> SIZES_SMALL({64, 8, 8, 18, 7, 24, 84, 4, 29, 58});
static std::vector<const char *> MODELS_SMALL(
    {"sazGTSGrfY",
     "uEQjieLDUq",
     "jQKzwhnzYa",
     "buIwVjnNDI",
     "goBJPxAkFf",
     "uKihCBaMwm",
     "VAyeIqqnSU",
     "bMNULcHsKb",
     "NBMEpcDimq",
     "wMCIoQQbNg"});

static std::vector<int> SIZES_LARGE({64, 8,  8,  18, 7,  99, 9,  61, 27,
                                     58, 30, 27, 95, 49, 37, 28, 87, 60,
                                     95, 1,  58, 14, 90, 9,  57});
static std::vector<const char *> MODELS_LARGE(
    {"sazGTSGrfY", "uEQjieLDUq", "jQKzwhnzYa", "buIwVjnNDI", "goBJPxAkFf",
     "oCtHzjLczM", "GJVcmhfddz", "nbpEAplbzQ", "yNXDBUcDys", "IZZQpqwiXa",
     "AAroNFkOBf", "flsXwiIaQG", "qazjSVkFcR", "PefkCqwfKJ", "yJDvizIEDY",
     "XauGblPeuo", "ZnvLBVjEom", "UeosvyfoBE", "BFeZAIAHQq", "iOYUWXWXhr",
     "WpOfaJwOlm", "sVHOxutIGB", "qOikwyZSWx", "KJGEPQxUKU", "cqrRvLCCYB"});

class TestApp : public ComposedWidget {
 public:
  bool renderLarge;

  explicit TestApp(bool renderLarge) : renderLarge(renderLarge) {}

  std::vector<Widget *> *getWidgets(
      std::vector<int> *sizes,
      std::vector<const char *> *models) {
    if (sizes->size() != models->size())
      throw 11;

    auto *ret = new std::vector<Widget *>();
    for (size_t i = 0; i < models->size(); i++) {
      int buttonSize = (*sizes)[i];
      const char *modelPath = (*models)[i];
      auto *widget = new ButtonAndModel(new RenderData{modelPath, buttonSize});
      char *key = (char *)malloc(strlen(modelPath) + 12);
      sprintf(key, "%s_%d", modelPath, buttonSize);
      widget->key = key;
      ret->push_back(widget);
    }
    return ret;
  }

  std::vector<Widget *> *getChildren() {
    if (renderLarge)
      return getWidgets(&SIZES_LARGE, &MODELS_LARGE);
    return getWidgets(&SIZES_SMALL, &MODELS_SMALL);
  }

  Widget *render() override {
    auto *children = getChildren();
    return new Container(children);
  }
};

using ComponentPair = std::pair<int, Component *>;
struct SceneDiff {
  std::vector<int> *createdEntities;
  std::vector<int> *deletedEntities;
  std::vector<ComponentPair *> *createdComponents;
  std::vector<ComponentPair *> *deletedComponents;
};

std::vector<RenderNode *> *reconcileChildren(
    std::vector<RenderNode *> *newChildren,
    std::vector<RenderNode *> *oldChildren);

RenderNode *reconcileRenderNode(RenderNode *newNode, RenderNode *oldNode) {
  return new RenderNode(
      newNode->key,
      oldNode->id,
      newNode->components,
      reconcileChildren(newNode->children, oldNode->children));
}

std::vector<RenderNode *> *reconcileChildren(
    std::vector<RenderNode *> *newChildren,
    std::vector<RenderNode *> *oldChildren) {
  auto *outChildren = new std::vector<RenderNode *>();
  std::unordered_map<std::string_view, RenderNode *> oldChildrenByKey;
  for (auto *child : *oldChildren) {
    oldChildrenByKey.emplace(child->key, child);
  }
  for (auto *child : *newChildren) {
    const char *newKey = child->key;
    auto it = oldChildrenByKey.find(newKey);
    if (it != oldChildrenByKey.end())
      outChildren->push_back(reconcileRenderNode(child, it->second));
    else
      outChildren->push_back(child);
  }
  return outChildren;
}

struct MapVector {
  std::unordered_map<int, std::vector<Component *> *> map;
  std::vector<std::pair<const int, std::vector<Component *> *> *> vec;
};

MapVector *mapEntitiesToComponents(std::vector<VirtualEntity *> *entities) {
  auto *map = new MapVector();
  for (auto *entity : *entities) {
    int key = entity->first;
    auto *value = entity->second;

    auto it = map->map.find(key);

    if (it == map->map.end()) {
      it = map->map.emplace(key, new std::vector<Component *>()).first;
      map->vec.push_back(&*it);
    }

    auto *components = it->second;

    for (auto *v : *value)
      components->push_back(v);
  }

  return map;
}

SceneDiff *diffTrees(
    std::vector<VirtualEntity *> *newEntities,
    std::vector<VirtualEntity *> *oldEntities) {
  auto *createdComponents = new std::vector<ComponentPair *>();
  auto *deletedComponents = new std::vector<ComponentPair *>();

  auto *oldEntityIds = new std::vector<int>();
  for (auto *entity : *oldEntities)
    oldEntityIds->push_back(entity->first);
  auto *newEntityIds = new std::vector<int>();
  for (auto *entity : *newEntities)
    newEntityIds->push_back(entity->first);

  auto *createdEntities = new std::vector<int>();
  for (int id : *newEntityIds)
    if (std::find(oldEntityIds->begin(), oldEntityIds->end(), id) ==
        oldEntityIds->end())
      createdEntities->push_back(id);

  auto *deletedEntities = new std::vector<int>();
  for (int id : *oldEntityIds)
    if (std::find(newEntityIds->begin(), newEntityIds->end(), id) ==
        newEntityIds->end())
      deletedEntities->push_back(id);

  auto *oldComponents = mapEntitiesToComponents(oldEntities);
  auto *newComponents = mapEntitiesToComponents(newEntities);

  for (int entityId : *createdEntities) {
    auto it = newComponents->map.find(entityId);
    if (it == newComponents->map.end())
      continue;
    for (Component *component : *it->second)
      createdComponents->push_back(new ComponentPair{entityId, component});
  }

  for (auto *kv : newComponents->vec) {
    auto [key, value] = *kv;

    if (!oldComponents->map.count(key))
      continue;

    auto it = oldComponents->map.find(key);
    auto *oldComponentsForKey = it != oldComponents->map.end()
        ? it->second
        : new std::vector<Component *>();
    auto *newComponentsForKey = value;

    auto *deleted = new std::vector<Component *>();
    for (auto *component : *oldComponentsForKey)
      if (std::find(
              newComponentsForKey->begin(),
              newComponentsForKey->end(),
              component) == newComponentsForKey->end())
        deleted->push_back(component);

    auto *created = new std::vector<Component *>();
    for (auto *component : *newComponentsForKey)
      if (std::find(
              oldComponentsForKey->begin(),
              oldComponentsForKey->end(),
              component) == oldComponentsForKey->end())
        created->push_back(component);

    for (auto *component : *deleted)
      deletedComponents->push_back(new ComponentPair{key, component});

    for (auto *component : *created)
      createdComponents->push_back(new ComponentPair{key, component});
  }
  return new SceneDiff{
      createdEntities, deletedEntities, createdComponents, deletedComponents};
}

std::optional<SceneDiff *> runTest(bool includeTreeSerialization) {
  auto *oldCtx = new Context("root");
  auto *oldWidgetTree = (new TestApp(false))->render();
  auto *oldRenderTree = oldWidgetTree->reduce(oldCtx);
  auto *oldEntityTree = oldRenderTree->reduce();

  auto *newCtx = new Context("root");
  auto *newWidgetTree = (new TestApp(true))->render();
  auto *newRenderTree = newWidgetTree->reduce(newCtx);

  auto *reconciledRenderTree =
      reconcileRenderNode(newRenderTree, oldRenderTree);

  auto *reconciledEntityTree = reconciledRenderTree->reduce();
  auto *diff = diffTrees(reconciledEntityTree, oldEntityTree);

  if (includeTreeSerialization)
    return diff;
  return std::nullopt;
}

static constexpr bool printDiff = false;

void printEntities(std::vector<int> *entities) {
  std::cout << "[";
  for (int d : *entities)
    std::cout << d << ", ";
  std::cout << "]";
}

void printComponents(std::vector<ComponentPair *> *components) {
  std::cout << "[";
  for (ComponentPair *cp : *components) {
    std::cout << "[" << cp->first << ",";
    if (auto *nc = dynamic_cast<NumberComponent *>(cp->second))
      std::cout << nc->x;
    else
      std::cout << dynamic_cast<StringComponent *>(cp->second)->x;
    std::cout << "], ";
  }
  std::cout << "]";
}

int main() {
  if (printDiff) {
    SceneDiff *res = *runTest(true);
    std::cout << "{\n"
              << "createdEntities: ";
    printEntities(res->createdEntities);
    std::cout << "\ndeletedEntities: ";
    printEntities(res->deletedEntities);
    std::cout << "\ncreatedComponents: ";
    printComponents(res->createdComponents);
    std::cout << "\ndeletedComponents: ";
    printComponents(res->deletedComponents);
    std::cout << "\n}";
  } else {
    int i;
    for (i = 0; i < 50; ++i)
      runTest(false);
    // The actual execution.
    auto t1 = std::chrono::steady_clock::now();
    for (i = 0; i < 5000; ++i)
      runTest(false);
    auto t2 = std::chrono::steady_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    printf("%llu ms %d iterations\n", t.count(), (int)i);
  }
}
