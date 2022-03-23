/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_C_API_H
#define HERMES_SUPPORT_C_API_H

#include <stddef.h>
#include <stdint.h>

typedef struct HermesDataRef {
  const void *data;
  size_t length;
} HermesDataRef;

typedef struct HermesSourceMapLocation {
  /// Index of the path.
  uint32_t path_index;
  /// 1-based.
  uint32_t line;
  /// 1-based.
  uint32_t column;
} HermesSourceMapLocation;

struct HermesSourceMap;
typedef struct HermesSourceMap HermesSourceMap;

#ifdef __cplusplus
extern "C" {
#endif

/// \return the created map, which is always non-null.
HermesSourceMap *hermes_source_map_parse(const char *source, size_t len);

/// \p map can be null (but it should never happen).
void hermes_source_map_free(HermesSourceMap *map);

/// \p map is the created map. It should never be null, but in case it is (stuff
///     happens), an "internal error" message is returned.
/// \return the error during map parsing or null if there is no error.
HermesDataRef hermes_source_map_get_error(const HermesSourceMap *map);

/// \return how many different paths are encoded in the source map.
uint32_t hermes_source_map_get_num_paths(const HermesSourceMap *map);

/// \return a reference to the full source path with index \p index.
///     The reference is valid until the call to the source map API.
HermesDataRef hermes_source_map_get_full_path(
    HermesSourceMap *map,
    uint32_t index);

/// \p line and \p column are 1-based.
/// \return null if the location could not be found, or a pointer to the found
///     location. The pointer is valid only until any other call to the map.
HermesSourceMapLocation *hermes_source_map_get_location(
    HermesSourceMap *map,
    uint32_t line,
    uint32_t column);

#ifdef __cplusplus
}
#endif

#endif // HERMES_SUPPORT_C_API_H
