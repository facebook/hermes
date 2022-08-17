/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/SourceMap/c-api.h"

#include "hermes/SourceMap/SourceMapParser.h"
#include "hermes/Support/SimpleDiagHandler.h"

using namespace hermes;

namespace {
template <typename T>
inline HermesDataRef makeHermesDataRef(const T &ref) {
  return {ref.data(), ref.size()};
}
} // namespace

struct HermesSourceMap {
  std::string error_;
  std::unique_ptr<SourceMap> sourceMap_;

  /// Temporary storage for source path.
  std::string pathBuf_;

  /// Temporary storage for the returned location.
  HermesSourceMapLocation locationBuf_;

  explicit HermesSourceMap(std::string &&error) : error_(std::move(error)) {}

  explicit HermesSourceMap(std::unique_ptr<SourceMap> sourceMap)
      : sourceMap_(std::move(sourceMap)) {}
};

extern "C" HermesSourceMap *hermes_source_map_parse(
    const char *source,
    size_t len) {
  if (len == 0 || source[len - 1])
    return new HermesSourceMap("Input is not zero terminated");

  SourceErrorManager sm;
  SimpleDiagHandlerRAII handler(sm);

  auto sourceMap = SourceMapParser::parse(llvh::StringRef(source, len - 1), sm);
  // Defensive programming.
  if (!sourceMap && !handler.haveErrors())
    sm.error(SMLoc{}, "internal error");

  if (sourceMap)
    return new HermesSourceMap(std::move(sourceMap));
  else
    return new HermesSourceMap(handler.getErrorString());
}

extern "C" void hermes_source_map_free(HermesSourceMap *map) {
  delete map;
}

extern "C" HermesDataRef hermes_source_map_get_error(
    const HermesSourceMap *map) {
  if (!map)
    return makeHermesDataRef(llvh::StringLiteral("internal error"));
  if (!map->sourceMap_)
    return makeHermesDataRef(map->error_);
  return HermesDataRef{nullptr, 0};
}

uint32_t hermes_source_map_get_num_paths(const HermesSourceMap *map) {
  if (!map || !map->sourceMap_)
    return 0;
  return map->sourceMap_->getNumSourcePaths();
}

HermesDataRef hermes_source_map_get_full_path(
    HermesSourceMap *map,
    uint32_t index) {
  uint32_t num = hermes_source_map_get_num_paths(map);
  if (index >= num)
    return {nullptr, 0};

  map->pathBuf_ = map->sourceMap_->getSourceFullPath(index);
  return makeHermesDataRef(map->pathBuf_);
}

extern "C" HermesSourceMapLocation *hermes_source_map_get_location(
    HermesSourceMap *map,
    uint32_t line,
    uint32_t column) {
  if (!map->sourceMap_)
    return nullptr;

  auto location = map->sourceMap_->getLocationForAddressFIndex(line, column);
  if (!location)
    return nullptr;

  map->locationBuf_.path_index = location->fileIndex;
  map->locationBuf_.line = location->line;
  map->locationBuf_.column = location->column;

  return &map->locationBuf_;
}
