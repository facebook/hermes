(function () {
  function getNodeById(root, id) {
    'noinline';
    const nodesToSearch = [root];
    let nodeSet;
    while ((nodeSet = nodesToSearch.pop())) {
      for (let i = 0; i < nodeSet.length; i++) {
        const node = nodeSet[i];
        if (node.props.id === id) {
          return node;
        }
        const children = node.children;
        if (children !== null) {
          nodesToSearch.push(children);
        }
      }
    }
    return null;
  }

  var idCounter = 0;
  function build(n) {
    if (n === 1) return [{ children: null, props: { id: idCounter++ } }];

    var nodeSet = [];
    for (var i = 0; i < n; ++i) {
      nodeSet.push({ children: build(n - 1), props: { id: idCounter++ } });
    }

    return nodeSet;
  }

  (function main() {
    'noinline';
    'use strict';
    var N = 10_000_000;

    var root = build(3);

    var queue;
    var start = Date.now();
    for (var i = 0; i < N; ++i) {
      queue = getNodeById(root, 12);
    }
    var end = Date.now();
    globalThis.q = queue;
    print('Time:', end - start);
  })();
})();

