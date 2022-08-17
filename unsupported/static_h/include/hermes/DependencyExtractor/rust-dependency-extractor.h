/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/DependencyExtractor/DependencyExtractor.h"

namespace hermes {
/// An opaque object containing the result of an extraction.
class Dependencies;
class Dependencies {
 public:
  std::string error_;
  std::vector<Dependency> deps_;
};
/// Parse the supplied source and return a Dependencies, which is an opaque
/// structure containing the extracted dependencies or an error message. The
/// result must be freed with \c hermesDependencies_free().
///
/// \param source utf-8 encoded input string. It must be zero terminated.
/// \param sourceSize the length of \c source in bytes, including the
///     terminating zero.
/// \return a new instance of Dependencies.
Dependencies *hermesExtractDependencies(const char *source, size_t sourceSize);

/// Free the Dependencies allocated by \c hermesExtractDependencies().
void hermesDependencies_free(Dependencies *res);

/// \return nullptr if compilation was successful, the error string otherwise.
const char *hermesDependencies_getError(const Dependencies *res);

/// \return a the name of the dependency at the given index
const char *hermesDependencies_getDepName(
    const Dependencies *res,
    size_t index);

/// \return a the kind of the dependency at the given index.
/// See DependencyExtractor for a list of kinds
const char *hermesDependencies_getDepKind(
    const Dependencies *res,
    size_t index);

size_t hermesDependencies_getDepsLength(const Dependencies *res);
} // namespace hermes
