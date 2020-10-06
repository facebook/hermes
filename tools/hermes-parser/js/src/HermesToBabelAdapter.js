/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const HermesASTAdapter = require('./HermesASTAdapter');

class HermesToBabelAdapter extends HermesASTAdapter {
  fixSourceLocation(node) {
    const loc = node.loc;
    if (loc == null) {
      return;
    }

    node.loc = {
      start: loc.start,
      end: loc.end,
    };

    node.start = loc.rangeStart;
    node.end = loc.rangeEnd;
  }
}

module.exports = HermesToBabelAdapter;
