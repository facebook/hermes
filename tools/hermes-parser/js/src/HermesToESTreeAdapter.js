/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const HermesASTAdapter = require('./HermesASTAdapter');

class HermesToESTreeAdapter extends HermesASTAdapter {
  fixSourceLocation(node) {
    const loc = node.loc;
    if (loc == null) {
      return;
    }

    node.loc = {
      start: loc.start,
      end: loc.end,
    };

    node.range = [loc.rangeStart, loc.rangeEnd];
  }
}

module.exports = HermesToESTreeAdapter;
