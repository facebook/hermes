/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Macro-benchmark: React-like component tree creation and reconciliation.
// Simulates building virtual DOM trees, diffing old vs new, and patching.
// Exercises object creation, recursive traversal, array manipulation, and
// property access in patterns representative of a UI framework.

"use strict";

var ITERATIONS = 200;
var TREE_BREADTH = 10;
var TREE_DEPTH = 3;

// --- Virtual node constructor ---
function VNode(type, props, children) {
  this.type = type;
  this.props = props;
  this.children = children;
  this.key = props && props.key !== undefined ? props.key : null;
}

// --- Build a component tree of ~1000 nodes ---
function buildTree(depth, breadth, seed) {
  var children = [];
  if (depth > 0) {
    for (var i = 0; i < breadth; i++) {
      children.push(buildTree(depth - 1, breadth, seed * breadth + i));
    }
  }
  var type = depth === 0 ? "span" : "div";
  var props = {
    key: "k" + seed,
    className: "cls-" + (seed % 5),
    style: {
      color: seed % 2 === 0 ? "red" : "blue",
      fontSize: 12 + (seed % 4)
    },
    onClick: null,
    "data-id": seed
  };
  return new VNode(type, props, children);
}

// --- Count nodes in a tree ---
function countNodes(node) {
  var count = 1;
  for (var i = 0; i < node.children.length; i++) {
    count += countNodes(node.children[i]);
  }
  return count;
}

// --- Reconcile (diff) two trees ---
// Returns an array of patch operations.
function reconcile(oldNode, newNode) {
  var patches = [];

  if (oldNode.type !== newNode.type) {
    patches.push({kind: "REPLACE", node: newNode});
    return patches;
  }

  // Diff props
  var oldProps = oldNode.props;
  var newProps = newNode.props;
  var propKeys = Object.keys(newProps);
  for (var i = 0; i < propKeys.length; i++) {
    var k = propKeys[i];
    if (k === "style") {
      // Shallow diff style object
      var oldStyle = oldProps.style || {};
      var newStyle = newProps.style || {};
      var styleKeys = Object.keys(newStyle);
      for (var s = 0; s < styleKeys.length; s++) {
        if (oldStyle[styleKeys[s]] !== newStyle[styleKeys[s]]) {
          patches.push({kind: "STYLE", prop: styleKeys[s], value: newStyle[styleKeys[s]]});
        }
      }
    } else if (oldProps[k] !== newProps[k]) {
      patches.push({kind: "PROP", prop: k, value: newProps[k]});
    }
  }

  // Diff children using keyed reconciliation
  var oldChildren = oldNode.children;
  var newChildren = newNode.children;

  // Build key-to-index map for old children
  var oldKeyMap = {};
  for (var j = 0; j < oldChildren.length; j++) {
    var key = oldChildren[j].key;
    if (key !== null) {
      oldKeyMap[key] = j;
    }
  }

  var maxLen = oldChildren.length > newChildren.length
    ? oldChildren.length : newChildren.length;
  for (var c = 0; c < maxLen; c++) {
    if (c >= oldChildren.length) {
      patches.push({kind: "INSERT", index: c, node: newChildren[c]});
    } else if (c >= newChildren.length) {
      patches.push({kind: "REMOVE", index: c});
    } else {
      var childPatches = reconcile(oldChildren[c], newChildren[c]);
      if (childPatches.length > 0) {
        patches.push({kind: "CHILD", index: c, patches: childPatches});
      }
    }
  }

  return patches;
}

// --- Apply patches (simulated) ---
function applyPatches(node, patches) {
  var ops = 0;
  for (var i = 0; i < patches.length; i++) {
    var p = patches[i];
    switch (p.kind) {
      case "REPLACE":
        ops += countNodes(p.node);
        break;
      case "PROP":
      case "STYLE":
        ops += 1;
        break;
      case "INSERT":
        ops += countNodes(p.node);
        break;
      case "REMOVE":
        ops += 1;
        break;
      case "CHILD":
        ops += applyPatches(node.children[p.index], p.patches);
        break;
    }
  }
  return ops;
}

// --- Benchmark: Build tree ---
function benchBuildTree() {
  var totalNodes = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var tree = buildTree(TREE_DEPTH, TREE_BREADTH, i);
    totalNodes += countNodes(tree);
  }
  return totalNodes;
}

// --- Benchmark: Reconcile similar trees (few changes) ---
function benchReconcileMinor() {
  var patchCount = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var oldTree = buildTree(TREE_DEPTH, TREE_BREADTH, 0);
    // New tree differs only in a few leaf props (seed offset by 1)
    var newTree = buildTree(TREE_DEPTH, TREE_BREADTH, 1);
    var patches = reconcile(oldTree, newTree);
    patchCount += applyPatches(oldTree, patches);
  }
  return patchCount;
}

// --- Benchmark: Reconcile with structural changes ---
function benchReconcileMajor() {
  var patchCount = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var oldTree = buildTree(TREE_DEPTH, TREE_BREADTH, 0);
    // Build a structurally different tree (different breadth at leaves)
    var newTree = buildTree(TREE_DEPTH, TREE_BREADTH, 500);
    // Mutate: remove some children, add others
    if (newTree.children.length > 2) {
      newTree.children.splice(1, 1);
      newTree.children.push(buildTree(TREE_DEPTH - 1, TREE_BREADTH, 999));
    }
    var patches = reconcile(oldTree, newTree);
    patchCount += applyPatches(oldTree, patches);
  }
  return patchCount;
}

// --- Benchmark: Deep clone and update ---
function cloneTree(node) {
  var newChildren = [];
  for (var i = 0; i < node.children.length; i++) {
    newChildren.push(cloneTree(node.children[i]));
  }
  var newProps = {};
  var keys = Object.keys(node.props);
  for (var k = 0; k < keys.length; k++) {
    var key = keys[k];
    if (key === "style") {
      newProps.style = Object.assign({}, node.props.style);
    } else {
      newProps[key] = node.props[key];
    }
  }
  return new VNode(node.type, newProps, newChildren);
}

function benchCloneAndUpdate() {
  var tree = buildTree(TREE_DEPTH, TREE_BREADTH, 0);
  var totalNodes = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var cloned = cloneTree(tree);
    // Update a handful of props in the clone
    cloned.props.className = "updated-" + i;
    if (cloned.children.length > 0) {
      cloned.children[0].props.style.color = "green";
    }
    totalNodes += countNodes(cloned);
  }
  return totalNodes;
}

// Run all benchmarks
function runBench(name, fn) {
  var start = Date.now();
  var result = fn();
  var elapsed = Date.now() - start;
  var opsPerSec = Math.round(ITERATIONS / (elapsed / 1000));
  print("RESULT: " + name + " " + opsPerSec + " ops/sec");
  return result;
}

runBench("build_tree", benchBuildTree);
runBench("reconcile_minor", benchReconcileMinor);
runBench("reconcile_major", benchReconcileMajor);
runBench("clone_and_update", benchCloneAndUpdate);
