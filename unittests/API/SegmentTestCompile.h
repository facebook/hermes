/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

namespace hermes {

/// Compiles the two modules \param mainCode and \param segmentCode into
/// separate segments for testing through the HermesRuntime API.
/// \return a pair of bytecode files [mainBC, segmentBC], for testing.
std::pair<std::string, std::string> genSplitCode(
    std::string &mainCode,
    std::string &segmentCode);

} // namespace hermes
