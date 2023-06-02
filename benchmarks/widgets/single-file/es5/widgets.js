/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

function _inheritsLoose(subClass, superClass) { subClass.prototype = Object.create(superClass.prototype); subClass.prototype.constructor = subClass; _setPrototypeOf(subClass, superClass); }
function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf ? Object.setPrototypeOf.bind() : function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }
(function () {
  // ==> widget.js <==
  let Widget = /*#__PURE__*/function () {
    function Widget() {
      this.key = void 0;
    }
    var _proto = Widget.prototype;
    _proto.reduce = function reduce(ctx) {
      throw new Error('Implement this in a subclass');
    };
    return Widget;
  }();
  let ComposedWidget = /*#__PURE__*/function (_Widget) {
    _inheritsLoose(ComposedWidget, _Widget);
    function ComposedWidget() {
      return _Widget.apply(this, arguments) || this;
    }
    var _proto2 = ComposedWidget.prototype;
    _proto2.render = function render() {
      throw new Error('Implement this in a subclass');
    };
    _proto2.reduce = function reduce(ctx) {
      let child = this.render();
      return child.reduce(ctx);
    };
    return ComposedWidget;
  }(Widget); // ==> widgets.js <==
  let Button = /*#__PURE__*/function (_Widget2) {
    _inheritsLoose(Button, _Widget2);
    function Button(num) {
      var _this;
      _this = _Widget2.call(this) || this;
      _this.num = void 0;
      _this.num = num;
      return _this;
    }
    var _proto3 = Button.prototype;
    _proto3.reduce = function reduce(ctx) {
      const component = {
        x: this.num
      };
      return RenderNode.create(ctx, [component], null);
    };
    return Button;
  }(Widget);
  let Floater = /*#__PURE__*/function (_Widget3) {
    _inheritsLoose(Floater, _Widget3);
    function Floater(num) {
      var _this2;
      _this2 = _Widget3.call(this) || this;
      _this2.num = void 0;
      _this2.num = num;
      return _this2;
    }
    var _proto4 = Floater.prototype;
    _proto4.reduce = function reduce(ctx) {
      const component = {
        x: this.num
      };
      return RenderNode.create(ctx, [component], null);
    };
    return Floater;
  }(Widget);
  let Gltf = /*#__PURE__*/function (_Widget4) {
    _inheritsLoose(Gltf, _Widget4);
    function Gltf(path) {
      var _this3;
      _this3 = _Widget4.call(this) || this;
      _this3.path = void 0;
      _this3.path = path;
      return _this3;
    }
    var _proto5 = Gltf.prototype;
    _proto5.reduce = function reduce(ctx) {
      const component = {
        x: this.path
      };
      return RenderNode.create(ctx, [component], null);
    };
    return Gltf;
  }(Widget);
  let Container = /*#__PURE__*/function (_Widget5) {
    _inheritsLoose(Container, _Widget5);
    function Container(children) {
      var _this4;
      _this4 = _Widget5.call(this) || this;
      _this4.children = void 0;
      _this4.children = children;
      return _this4;
    }
    var _proto6 = Container.prototype;
    _proto6.reduce = function reduce(ctx) {
      const component = {
        x: 13
      };
      const children = this.children.map(child => RenderNode.createForChild(ctx, child));
      return RenderNode.create(ctx, [component], children);
    };
    return Container;
  }(Widget); // ==> app_runner.js <==
  function reconcileRenderNode(newNode, oldNode) {
    return new RenderNode(newNode.key, oldNode.id, newNode.components, reconcileChildren(newNode.children, oldNode.children));
  }
  function reconcileChildren(newChildren, oldChildren) {
    const outChildren = [];
    const oldChildrenByKey = new Map();
    oldChildren.forEach(child => oldChildrenByKey.set(child.key, child));
    newChildren.forEach(child => {
      const newKey = child.key;
      const oldChild = oldChildrenByKey.get(newKey);
      if (oldChild !== undefined) {
        outChildren.push(reconcileRenderNode(child, oldChild));
      } else {
        outChildren.push(child);
      }
    });
    return outChildren;
  }
  function mapEntitiesToComponents(entities) {
    const map = new Map();
    entities.forEach(entity => {
      const key = entity[0];
      const value = entity[1];
      if (map.get(key) == undefined) {
        map.set(key, []);
      }
      const components = map.get(key);
      if (components !== undefined) {
        components.push(...value);
      } else {
        throw new Error('components shouldnt be undefined');
      }
    });
    return map;
  }
  function diffTrees(newEntities, oldEntities) {
    const createdComponents = [];
    const deletedComponents = [];
    const oldEntityIds = oldEntities.map(entity => entity[0]);
    const newEntityIds = newEntities.map(entity => entity[0]);
    const createdEntities = newEntityIds.filter(entityId => !oldEntityIds.includes(entityId));
    const deletedEntities = oldEntityIds.filter(entityId => !newEntityIds.includes(entityId));
    const oldComponents = mapEntitiesToComponents(oldEntities);
    const newComponents = mapEntitiesToComponents(newEntities);
    createdEntities.forEach(entityId => {
      const components = (newComponents.get(entityId) || []).map(it => [entityId, it]);
      createdComponents.push(...components);
    });
    newComponents.forEach((value, key) => {
      if (oldComponents.get(key) == undefined) {
        return;
      }
      const oldComponentsForKey = oldComponents.get(key) || [];
      const newComponentsForKey = value;
      const deleted = oldComponentsForKey.filter(it => !newComponentsForKey.includes(it));
      const created = newComponentsForKey.filter(it => !oldComponentsForKey.includes(it));
      deleted.forEach(it => deletedComponents.push([key, it]));
      created.forEach(it => createdComponents.push([key, it]));
    });
    return {
      createdEntities: createdEntities,
      deletedEntities: deletedEntities,
      createdComponents: createdComponents,
      deletedComponents: deletedComponents
    };
  }
  function runTest(includeTreeSerialization) {
    /* Render a tree with 10 widgets */
    const oldCtx = new Context('root');
    const oldWidgetTree = new TestApp(false).render();
    const oldRenderTree = oldWidgetTree.reduce(oldCtx);
    const oldEntityTree = oldRenderTree.reduce();

    /* Render a tree with 25 widgets, the first 5 of which are the same */
    const newCtx = new Context('root');
    const newWidgetTree = new TestApp(true).render();
    const newRenderTree = newWidgetTree.reduce(newCtx);
    const reconciledRenderTree = reconcileRenderNode(newRenderTree, oldRenderTree);
    const reconciledEntityTree = reconciledRenderTree.reduce();
    const diff = diffTrees(reconciledEntityTree, oldEntityTree);
    return includeTreeSerialization ? diff : null;
  }

  // ==> components.js <==
  // ==> context.js <==
  let Context = /*#__PURE__*/function () {
    function Context(key) {
      this.key = void 0;
      this.childCounter = void 0;
      this.key = key;
      this.childCounter = 0;
    }
    Context.createForChild = function createForChild(parentCtx, child) {
      const widgetKey = child.key;
      const childKey = widgetKey !== null && widgetKey !== undefined ? widgetKey : `${child.constructor.name}_${parentCtx.childCounter++}`;
      const newKey = `${parentCtx.key}_${childKey}`;
      return new Context(newKey);
    };
    return Context;
  }(); // ==> render_node.js <==
  let RenderNode = /*#__PURE__*/function () {
    function RenderNode(key, id, components, children) {
      this.key = void 0;
      this.id = void 0;
      this.components = void 0;
      this.children = void 0;
      this.key = key;
      this.id = id;
      this.components = components;
      this.children = children || [];
    }
    var _proto7 = RenderNode.prototype;
    _proto7.reduce = function reduce() {
      const childrenEntities = (this.children || []).flatMap(child => child.reduce());
      return [[this.id, this.components]].concat(childrenEntities);
    };
    RenderNode.create = function create(ctx, components, children) {
      return new RenderNode(ctx.key, RenderNode.idCounter++, components, children);
    };
    RenderNode.createForChild = function createForChild(ctx, child) {
      const childCtx = Context.createForChild(ctx, child);
      return child.reduce(childCtx);
    };
    return RenderNode;
  }(); // ==> test_app.js <==
  RenderNode.idCounter = 0;
  let ButtonAndModel = /*#__PURE__*/function (_ComposedWidget) {
    _inheritsLoose(ButtonAndModel, _ComposedWidget);
    function ButtonAndModel(data) {
      var _this5;
      _this5 = _ComposedWidget.call(this) || this;
      _this5.data = void 0;
      _this5.data = data;
      return _this5;
    }
    var _proto8 = ButtonAndModel.prototype;
    _proto8.render = function render() {
      const children = [new Button(this.data.buttonSize), new Gltf(this.data.modelPath), new Floater(Math.sqrt(this.data.buttonSize))];
      return new Container(children);
    };
    return ButtonAndModel;
  }(ComposedWidget);
  let TestApp = /*#__PURE__*/function (_ComposedWidget2) {
    _inheritsLoose(TestApp, _ComposedWidget2);
    function TestApp(renderLarge) {
      var _this6;
      _this6 = _ComposedWidget2.call(this) || this;
      _this6.renderLarge = void 0;
      _this6.renderLarge = renderLarge;
      return _this6;
    }
    var _proto9 = TestApp.prototype;
    _proto9.getWidgets = function getWidgets(sizes, models) {
      if (sizes.length != models.length) {
        throw new Error('sizes and models must have same length');
      }
      return models.map((modelPath, index) => {
        const buttonSize = sizes[index];
        const widget = new ButtonAndModel({
          modelPath: modelPath,
          buttonSize: buttonSize
        });
        widget.key = `${modelPath}_${buttonSize}`;
        return widget;
      });
    };
    _proto9.getChildren = function getChildren() {
      if (this.renderLarge) {
        return this.getWidgets(SIZES_LARGE, MODELS_LARGE);
      }
      return this.getWidgets(SIZES_SMALL, MODELS_SMALL);
    };
    _proto9.render = function render() {
      const children = this.getChildren();
      return new Container(children);
    };
    return TestApp;
  }(ComposedWidget); // ==> test_app_data.js <==
  const SIZES_SMALL = [64, 8, 8, 18, 7, 24, 84, 4, 29, 58];
  const SIZES_LARGE = SIZES_SMALL.slice(0, 5).concat([99, 9, 61, 27, 58, 30, 27, 95, 49, 37, 28, 87, 60, 95, 1, 58, 14, 90, 9, 57]);
  const MODELS_SMALL = ['sazGTSGrfY', 'uEQjieLDUq', 'jQKzwhnzYa', 'buIwVjnNDI', 'goBJPxAkFf', 'uKihCBaMwm', 'VAyeIqqnSU', 'bMNULcHsKb', 'NBMEpcDimq', 'wMCIoQQbNg'];
  const MODELS_LARGE = MODELS_SMALL.slice(0, 5).concat(['oCtHzjLczM', 'GJVcmhfddz', 'nbpEAplbzQ', 'yNXDBUcDys', 'IZZQpqwiXa', 'AAroNFkOBf', 'flsXwiIaQG', 'qazjSVkFcR', 'PefkCqwfKJ', 'yJDvizIEDY', 'XauGblPeuo', 'ZnvLBVjEom', 'UeosvyfoBE', 'BFeZAIAHQq', 'iOYUWXWXhr', 'WpOfaJwOlm', 'sVHOxutIGB', 'qOikwyZSWx', 'KJGEPQxUKU', 'cqrRvLCCYB']);

  // ==> index.js <==

  globalThis.runFullTest = runTest;

  // ==> main.js <==

  const printDiff = false;
  if (printDiff) {
    print(JSON.stringify(runFullTest(true)));
  } else {
    print("Running...");
    // Warmup.
    var i;
    for (i = 0; i < 50; ++i) runFullTest(false);
    // The actual execution.
    let t1 = Date.now();
    for (i = 0; i < 5000; ++i) runFullTest(false);
    print(Date.now() - t1, "ms", i, "iterations");
  }
})();
