/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_API_CDPJSONHELPERS_H
#define HERMES_UNITTESTS_API_CDPJSONHELPERS_H

namespace facebook {
namespace hermes {

void ensureErrorResponse(const std::string &message, int id);
void ensureOkResponse(const std::string &message, int id);

void ensureResponse(
    const std::string &message,
    const std::string &method,
    int id);
void ensureNotification(const std::string &message, const std::string &method);

} // namespace hermes
} // namespace facebook

#endif // HERMES_UNITTESTS_API_CDPJSONHELPERS_H
