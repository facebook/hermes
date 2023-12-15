/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef SHERMES_PARSEJSFILE_H
#define SHERMES_PARSEJSFILE_H

#include "llvh/Support/MemoryBuffer.h"

namespace hermes {
// Forward declarations.
namespace ESTree {
class ProgramNode;
} // namespace ESTree

class Context;
class SourceMapTranslator;

using DeclarationFileListTy = std::vector<ESTree::ProgramNode *>;

/// This mode determines how to interpret the source map information encoded
/// in a magic comment in JS source.
enum class SourceMappingCommentMode {
  /// Ignore the source map magic comment.
  Off,
  /// Only use the source map if it us embedded using a data URL.
  Data,
  /// Use the source map if it is a data or file URL.
  File,
};

/// Whether to look for "use static builtin" directives in the source.
enum class StaticBuiltinSetting {
  ForceOn,
  ForceOff,
  AutoDetect,
};

/// Parse a JS file with an optional source map.
/// \param smCommentsMode how to deal with source mapping comments.
/// \param fileBufId buffer ID of the JS file.
/// \param inputSourceMap source map explicitly given to the compiler by the
///   user. It overrides everything else.
/// \param sourceMapTranslator the translator, which will be instantiated the
///   first time it is needed.
/// \return the parsed AST, or nullptr on error, in which case an error message
///     will have been generated.
ESTree::ProgramNode *parseJSFile(
    Context *context,
    SourceMappingCommentMode smCommentsMode,
    StaticBuiltinSetting staticBuiltinSetting,
    unsigned fileBufId,
    llvh::StringRef inputSourceMap,
    std::shared_ptr<SourceMapTranslator> &sourceMapTranslator);

/// Loads global definitions from MemoryBuffer and adds the definitions to \p
/// declFileList.
/// \return true on success, false on error.
bool loadGlobalDefinition(
    Context &context,
    std::unique_ptr<llvh::MemoryBuffer> content,
    DeclarationFileListTy &declFileList);

/// Read a file at path \p path into a memory buffer. If \p stdinOk is set,
/// allow "-" to mean stdin.
/// \param pathDesc an optional description of what we are trying to read, to be
///    used in error messages instead of the word "file".
/// \param silent if true, don't print an error message on failure.
///
/// \return the memory buffer, or nullptr on error, in
///     which case an error message will have been printed to llvh::errs().
std::unique_ptr<llvh::MemoryBuffer> memoryBufferFromFile(
    llvh::StringRef path,
    llvh::StringRef pathDesc,
    bool stdinOk = false,
    bool silent = false);

} // namespace hermes

#endif // SHERMES_PARSEJSFILE_H
