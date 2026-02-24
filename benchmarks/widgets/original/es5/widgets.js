/******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId]) {
/******/ 			return installedModules[moduleId].exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			i: moduleId,
/******/ 			l: false,
/******/ 			exports: {}
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.l = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// define getter function for harmony exports
/******/ 	__webpack_require__.d = function(exports, name, getter) {
/******/ 		if(!__webpack_require__.o(exports, name)) {
/******/ 			Object.defineProperty(exports, name, { enumerable: true, get: getter });
/******/ 		}
/******/ 	};
/******/
/******/ 	// define __esModule on exports
/******/ 	__webpack_require__.r = function(exports) {
/******/ 		if(typeof Symbol !== 'undefined' && Symbol.toStringTag) {
/******/ 			Object.defineProperty(exports, Symbol.toStringTag, { value: 'Module' });
/******/ 		}
/******/ 		Object.defineProperty(exports, '__esModule', { value: true });
/******/ 	};
/******/
/******/ 	// create a fake namespace object
/******/ 	// mode & 1: value is a module id, require it
/******/ 	// mode & 2: merge all properties of value into the ns
/******/ 	// mode & 4: return value when already ns object
/******/ 	// mode & 8|1: behave like require
/******/ 	__webpack_require__.t = function(value, mode) {
/******/ 		if(mode & 1) value = __webpack_require__(value);
/******/ 		if(mode & 8) return value;
/******/ 		if((mode & 4) && typeof value === 'object' && value && value.__esModule) return value;
/******/ 		var ns = Object.create(null);
/******/ 		__webpack_require__.r(ns);
/******/ 		Object.defineProperty(ns, 'default', { enumerable: true, value: value });
/******/ 		if(mode & 2 && typeof value != 'string') for(var key in value) __webpack_require__.d(ns, key, function(key) { return value[key]; }.bind(null, key));
/******/ 		return ns;
/******/ 	};
/******/
/******/ 	// getDefaultExport function for compatibility with non-harmony modules
/******/ 	__webpack_require__.n = function(module) {
/******/ 		var getter = module && module.__esModule ?
/******/ 			function getDefault() { return module['default']; } :
/******/ 			function getModuleExports() { return module; };
/******/ 		__webpack_require__.d(getter, 'a', getter);
/******/ 		return getter;
/******/ 	};
/******/
/******/ 	// Object.prototype.hasOwnProperty.call
/******/ 	__webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";
/******/
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(__webpack_require__.s = 0);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
// ESM COMPAT FLAG
__webpack_require__.r(__webpack_exports__);

// CONCATENATED MODULE: ../context.js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * 
 * @format
 */


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
}();

// CONCATENATED MODULE: ../widget.js
function _inheritsLoose(subClass, superClass) { subClass.prototype = Object.create(superClass.prototype); subClass.prototype.constructor = subClass; _setPrototypeOf(subClass, superClass); }
function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf ? Object.setPrototypeOf.bind() : function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * 
 * @format
 */


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
}(Widget);
// CONCATENATED MODULE: ../render_node.js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * 
 * @format
 */



let render_node_RenderNode = /*#__PURE__*/function () {
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
  var _proto = RenderNode.prototype;
  _proto.reduce = function reduce() {
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
}();
render_node_RenderNode.idCounter = 0;
// CONCATENATED MODULE: ../widgets.js
function widgets_inheritsLoose(subClass, superClass) { subClass.prototype = Object.create(superClass.prototype); subClass.prototype.constructor = subClass; widgets_setPrototypeOf(subClass, superClass); }
function widgets_setPrototypeOf(o, p) { widgets_setPrototypeOf = Object.setPrototypeOf ? Object.setPrototypeOf.bind() : function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return widgets_setPrototypeOf(o, p); }
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * 
 * @format
 */




let widgets_Button = /*#__PURE__*/function (_Widget) {
  widgets_inheritsLoose(Button, _Widget);
  function Button(num) {
    var _this;
    _this = _Widget.call(this) || this;
    _this.num = void 0;
    _this.num = num;
    return _this;
  }
  var _proto = Button.prototype;
  _proto.reduce = function reduce(ctx) {
    const component = {
      x: this.num
    };
    return render_node_RenderNode.create(ctx, [component], null);
  };
  return Button;
}(Widget);
let widgets_Floater = /*#__PURE__*/function (_Widget2) {
  widgets_inheritsLoose(Floater, _Widget2);
  function Floater(num) {
    var _this2;
    _this2 = _Widget2.call(this) || this;
    _this2.num = void 0;
    _this2.num = num;
    return _this2;
  }
  var _proto2 = Floater.prototype;
  _proto2.reduce = function reduce(ctx) {
    const component = {
      x: this.num
    };
    return render_node_RenderNode.create(ctx, [component], null);
  };
  return Floater;
}(Widget);
let widgets_Gltf = /*#__PURE__*/function (_Widget3) {
  widgets_inheritsLoose(Gltf, _Widget3);
  function Gltf(path) {
    var _this3;
    _this3 = _Widget3.call(this) || this;
    _this3.path = void 0;
    _this3.path = path;
    return _this3;
  }
  var _proto3 = Gltf.prototype;
  _proto3.reduce = function reduce(ctx) {
    const component = {
      x: this.path
    };
    return render_node_RenderNode.create(ctx, [component], null);
  };
  return Gltf;
}(Widget);
let widgets_Container = /*#__PURE__*/function (_Widget4) {
  widgets_inheritsLoose(Container, _Widget4);
  function Container(children) {
    var _this4;
    _this4 = _Widget4.call(this) || this;
    _this4.children = void 0;
    _this4.children = children;
    return _this4;
  }
  var _proto4 = Container.prototype;
  _proto4.reduce = function reduce(ctx) {
    const component = {
      x: 13
    };
    const children = this.children.map(child => render_node_RenderNode.createForChild(ctx, child));
    return render_node_RenderNode.create(ctx, [component], children);
  };
  return Container;
}(Widget);
// CONCATENATED MODULE: ../test_app_data.js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * 
 * @format
 */

const SIZES_SMALL = [64, 8, 8, 18, 7, 24, 84, 4, 29, 58];
const SIZES_LARGE = SIZES_SMALL.slice(0, 5).concat([99, 9, 61, 27, 58, 30, 27, 95, 49, 37, 28, 87, 60, 95, 1, 58, 14, 90, 9, 57]);
const MODELS_SMALL = ['sazGTSGrfY', 'uEQjieLDUq', 'jQKzwhnzYa', 'buIwVjnNDI', 'goBJPxAkFf', 'uKihCBaMwm', 'VAyeIqqnSU', 'bMNULcHsKb', 'NBMEpcDimq', 'wMCIoQQbNg'];
const MODELS_LARGE = MODELS_SMALL.slice(0, 5).concat(['oCtHzjLczM', 'GJVcmhfddz', 'nbpEAplbzQ', 'yNXDBUcDys', 'IZZQpqwiXa', 'AAroNFkOBf', 'flsXwiIaQG', 'qazjSVkFcR', 'PefkCqwfKJ', 'yJDvizIEDY', 'XauGblPeuo', 'ZnvLBVjEom', 'UeosvyfoBE', 'BFeZAIAHQq', 'iOYUWXWXhr', 'WpOfaJwOlm', 'sVHOxutIGB', 'qOikwyZSWx', 'KJGEPQxUKU', 'cqrRvLCCYB']);
// CONCATENATED MODULE: ../test_app.js
function test_app_inheritsLoose(subClass, superClass) { subClass.prototype = Object.create(superClass.prototype); subClass.prototype.constructor = subClass; test_app_setPrototypeOf(subClass, superClass); }
function test_app_setPrototypeOf(o, p) { test_app_setPrototypeOf = Object.setPrototypeOf ? Object.setPrototypeOf.bind() : function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return test_app_setPrototypeOf(o, p); }
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * 
 * @format
 */





let test_app_ButtonAndModel = /*#__PURE__*/function (_ComposedWidget) {
  test_app_inheritsLoose(ButtonAndModel, _ComposedWidget);
  function ButtonAndModel(data) {
    var _this;
    _this = _ComposedWidget.call(this) || this;
    _this.data = void 0;
    _this.data = data;
    return _this;
  }
  var _proto = ButtonAndModel.prototype;
  _proto.render = function render() {
    const children = [new widgets_Button(this.data.buttonSize), new widgets_Gltf(this.data.modelPath), new widgets_Floater(Math.sqrt(this.data.buttonSize))];
    return new widgets_Container(children);
  };
  return ButtonAndModel;
}(ComposedWidget);
let test_app_TestApp = /*#__PURE__*/function (_ComposedWidget2) {
  test_app_inheritsLoose(TestApp, _ComposedWidget2);
  function TestApp(renderLarge) {
    var _this2;
    _this2 = _ComposedWidget2.call(this) || this;
    _this2.renderLarge = void 0;
    _this2.renderLarge = renderLarge;
    return _this2;
  }
  var _proto2 = TestApp.prototype;
  _proto2.getWidgets = function getWidgets(sizes, models) {
    if (sizes.length != models.length) {
      throw new Error('sizes and models must have same length');
    }
    return models.map((modelPath, index) => {
      const buttonSize = sizes[index];
      const widget = new test_app_ButtonAndModel({
        modelPath: modelPath,
        buttonSize: buttonSize
      });
      widget.key = `${modelPath}_${buttonSize}`;
      return widget;
    });
  };
  _proto2.getChildren = function getChildren() {
    if (this.renderLarge) {
      return this.getWidgets(SIZES_LARGE, MODELS_LARGE);
    }
    return this.getWidgets(SIZES_SMALL, MODELS_SMALL);
  };
  _proto2.render = function render() {
    const children = this.getChildren();
    return new widgets_Container(children);
  };
  return TestApp;
}(ComposedWidget);
// CONCATENATED MODULE: ../app_runner.js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * 
 * @format
 */




function reconcileRenderNode(newNode, oldNode) {
  return new render_node_RenderNode(newNode.key, oldNode.id, newNode.components, reconcileChildren(newNode.children, oldNode.children));
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
  const oldWidgetTree = new test_app_TestApp(false).render();
  const oldRenderTree = oldWidgetTree.reduce(oldCtx);
  const oldEntityTree = oldRenderTree.reduce();

  /* Render a tree with 25 widgets, the first 5 of which are the same */
  const newCtx = new Context('root');
  const newWidgetTree = new test_app_TestApp(true).render();
  const newRenderTree = newWidgetTree.reduce(newCtx);
  const reconciledRenderTree = reconcileRenderNode(newRenderTree, oldRenderTree);
  const reconciledEntityTree = reconciledRenderTree.reduce();
  const diff = diffTrees(reconciledEntityTree, oldEntityTree);
  return includeTreeSerialization ? diff : null;
}
// CONCATENATED MODULE: ../index.js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * 
 * @format
 */


globalThis.runFullTest = runTest;
// CONCATENATED MODULE: ../main.js
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


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

/***/ })
/******/ ]);
//# sourceMappingURL=widgets.js.map