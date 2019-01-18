/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_CHECKEDMALLOC_H
#define HERMES_SUPPORT_CHECKEDMALLOC_H

#include <cstdlib>

namespace hermes {

/// These are like malloc and calloc, respectively, but check their return
/// results and call hermes_fatal if the allocation has failed.
void *checkedMalloc(size_t sz);
void *checkedCalloc(size_t elemSize, size_t numElems);

} // namespace hermes

#endif // HERMES_SUPPORT_CHECKEDMALLOC_H
