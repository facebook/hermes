/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import inheritTrailingComments from "./inheritTrailingComments";
import inheritLeadingComments from "./inheritLeadingComments";
import inheritInnerComments from "./inheritInnerComments";

/**
 * Inherit all unique comments from `parent` node to `child` node.
 */
export default function inheritsComments<T: Object>(
  child: T,
  parent: Object,
): T {
  inheritTrailingComments(child, parent);
  inheritLeadingComments(child, parent);
  inheritInnerComments(child, parent);

  return child;
}
