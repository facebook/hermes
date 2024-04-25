/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_API_CDPJSONHELPERS_H
#define HERMES_UNITTESTS_API_CDPJSONHELPERS_H

namespace hermes {

void ensureErrorResponse(int id, const std::string &json);

} // namespace hermes

#endif // HERMES_UNITTESTS_API_CDPJSONHELPERS_H
