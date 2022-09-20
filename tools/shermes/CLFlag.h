/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef SHERMES_CLFLAG_H
#define SHERMES_CLFLAG_H

#include <string>

#include "llvh/Support/CommandLine.h"

namespace {
/// Encapsulate a compiler flag: for example, "-fflag/-fno-flag", or
/// "-Wflag/-Wno-flag".
class CLFlag {
  std::string yesName_;
  std::string yesHelp_;
  std::string noName_;
  std::string noHelp_;
  llvh::cl::opt<bool> yes_;
  llvh::cl::opt<bool> no_;
  const bool defaultValue_;

 public:
  CLFlag(const CLFlag &) = delete;
  void operator=(CLFlag &) = delete;

  /// \param flagChar is the character that will be prepended to the flag name.
  /// \param name is the name for the command line option
  /// \param defaultValue is the default if neither is specified.
  /// \param desc is the description starting with lower case like " inlining of
  /// functions".
  CLFlag(
      char flagChar,
      const llvh::Twine &name,
      bool defaultValue,
      const llvh::Twine &desc,
      llvh::cl::OptionCategory &category)
      : yesName_((llvh::Twine(flagChar) + name).str()),
        yesHelp_(("Enable " + desc).str()),
        noName_((llvh::Twine(flagChar) + "no-" + name).str()),
        noHelp_(("Disable " + desc).str()),
        yes_(
            llvh::StringRef(yesName_),
            llvh::cl::ValueDisallowed,
            llvh::cl::desc(llvh::StringRef(yesHelp_)),
            llvh::cl::cat(category)),
        no_(llvh::StringRef(noName_),
            llvh::cl::ValueDisallowed,
            llvh::cl::Hidden,
            llvh::cl::desc(llvh::StringRef(noHelp_)),
            llvh::cl::cat(category)),
        defaultValue_(defaultValue) {}

  /// Resolve the value of the flag depending on which command line option is
  /// present and which one is last.
  bool getValue() const {
    if (yes_.getPosition() > no_.getPosition())
      return true;
    if (yes_.getPosition() < no_.getPosition())
      return false;
    return defaultValue_;
  }

  /// Casting to bool always makes sense, so no "explicit" needed here.
  operator bool() const {
    return getValue();
  }
};

} // namespace

#endif // SHERMES_CLFLAG_H
