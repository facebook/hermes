/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_JENKINSHASH_H
#define HERMES_SUPPORT_JENKINSHASH_H

#include <cstdint>

namespace hermes {

using JenkinsHash = uint32_t;

namespace jenkins_details {
template <typename CharT>
constexpr JenkinsHash jenkinsAdd(JenkinsHash hash, CharT c) {
  return hash + static_cast<JenkinsHash>(c);
}

constexpr JenkinsHash jenkinsMix1(JenkinsHash hash) {
  return hash + (hash << 10);
}

constexpr JenkinsHash jenkinsMix2(JenkinsHash hash) {
  return hash ^ (hash >> 6);
}
} // namespace jenkins_details

/// Incorporates the character \p c to the given \p hash, using the classic
/// Jenkins algorithm.
/// \return the new hash value.
template <typename CharT>
constexpr JenkinsHash updateJenkinsHash(JenkinsHash hash, CharT c) {
  using namespace jenkins_details;
  static_assert(
      sizeof(CharT) <= sizeof(char16_t),
      "Jenkins hash algorithm only hashes characters");
  return jenkinsMix2(jenkinsMix1(jenkinsAdd(hash, c)));
}

} // namespace hermes

#endif
