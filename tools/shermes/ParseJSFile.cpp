/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ParseJSFile.h"

#include "hermes/Parser/JSParser.h"
#include "hermes/SourceMap/SourceMapParser.h"
#include "hermes/SourceMap/SourceMapTranslator.h"
#include "hermes/Support/Base64.h"
#include "hermes/Support/MemoryBuffer.h"
#include "hermes/Support/SimpleDiagHandler.h"

#include "llvh/Support/Path.h"

namespace hermes {

namespace {

/// Extract a scheme, which is defined by more than one lowercase letter
/// followed by colon. The returned scheme includes the colon.
/// The return scheme is a substring of the input url, which continues at the
/// scheme's end pointer.
llvh::StringRef extractScheme(llvh::StringRef url) {
  const unsigned char *p = url.bytes_begin();
  while (p != url.bytes_end() && (*p | 32) >= 'a' && (*p | 32) <= 'z')
    ++p;
  return p != url.bytes_end() && *p == ':' && p - url.bytes_begin() > 1
      ? url.take_front(p - url.bytes_begin() + 1)
      : llvh::StringRef{};
}

/// Encapsulate state and various helpers for parsing a JS file with an optional
/// source map.
class ParseJSFile {
  using JSParser = parser::JSParser;

  /// The AST context.
  Context *const context_;
  /// How to deal with source mapping comments.
  SourceMappingCommentMode smCommentMode_;
  /// Whether to detect "use static builtin" directives in the source.
  StaticBuiltinSetting staticBuiltinSetting_;
  /// Buffer ID of the JS file.
  unsigned const fileBufId_;
  /// The buffer of the JS file.
  const llvh::MemoryBuffer *const fileBuf_;
  /// The source map URL. This is either a source map specified on the CLI,
  /// which overrides everything, or an URL extracted from the JS file.
  llvh::StringRef sourceMapURL_;
  /// If true, sourceMapURL_ was specified explicitly by the user when invoking
  /// the compiler. In this sense, the URL is "trusted". It overrides the
  /// comment mode or URLs found in the source.
  /// TODO: better name.
  bool trustedSourceMap_;
  /// The source map translator, which is instantiated the first time it is
  /// needed, in order to associate a source map with a JS file.
  std::shared_ptr<SourceMapTranslator> &sourceMapTranslator_;

  // Keep track of which magic comments to register with SourceErrorManager.
  JSParser::MCFlag::Type mcFlags_ = JSParser::MCFlag::All;

  /// For readability, encode failure or success status.
  enum class Status {
    Error,
    OK,
  };

 public:
  /// \param smCommentsMode how to deal with source mapping comments.
  /// \param fileBufId buffer ID of the JS file.
  /// \param inputSourceMap source map explicitly given to the compiler by the
  ///   user. It overrides everything else.
  /// \param sourceMapTranslator the translator, which will be instantiated the
  ///   first time it is needed.
  ParseJSFile(
      Context *context,
      SourceMappingCommentMode smCommentsMode,
      StaticBuiltinSetting staticBuiltinSetting,
      unsigned fileBufId,
      llvh::StringRef inputSourceMap,
      std::shared_ptr<SourceMapTranslator> &sourceMapTranslator)
      : context_(context),
        smCommentMode_(smCommentsMode),
        staticBuiltinSetting_(staticBuiltinSetting),
        fileBufId_(fileBufId),
        fileBuf_(context->getSourceErrorManager().getSourceBuffer(fileBufId)),
        sourceMapURL_(inputSourceMap),
        trustedSourceMap_(!inputSourceMap.empty()),
        sourceMapTranslator_(sourceMapTranslator) {
    assert(fileBuf_);
  }

  ~ParseJSFile() {}

  /// Parse the file, optionally associate the source map, generate errors.
  /// \return the parsed AST, or nullptr on error.
  ESTree::ProgramNode *run() {
    // This value will be set to true if the parser detected the 'use static
    // builtin' directive in the source.
    bool useStaticBuiltinDetected = false;
    // Parser mode - full or lazy.
    parser::ParserPass mode = parser::FullParse;

    // Disable registering the //# sourceMappingURL= magic comment in
    // SourceErrorManager, if we are consuming a source map.
    if (!sourceMapURL_.empty())
      mcFlags_ &= ~JSParser::MCFlag::SourceMappingURL;

    // Check whether the file is considered "large". Large files are compiled
    // lazily.
    bool isLargeFile = fileBuf_->getBufferSize() >=
        context_->getPreemptiveFileCompilationThreshold();

    if (context_->isLazyCompilation() && isLargeFile) {
      // When parsing lazily, first we "pre-parse" the buffer, recording the
      // locations of every function in the AST context.
      auto preParser = JSParser::preParseBuffer(*context_, fileBufId_);
      if (!preParser)
        return nullptr;
      useStaticBuiltinDetected = preParser->getUseStaticBuiltin();
      checkSourceMappingComment(*preParser);
      preParser->registerMagicURLs(mcFlags_);
      mode = parser::LazyParse;
    }

    llvh::Optional<ESTree::ProgramNode *> parsedJs;

    // Now perform the actual parsing. In lazy mode, this will only parse the
    // outer function.
    {
      JSParser jsParser(*context_, fileBufId_, mode);
      parsedJs = jsParser.parse();
      // If we are using lazy parse mode, we should have already detected the
      // 'use static builtin' directive and magic URLs in the pre-parsing
      // stage.
      if (parsedJs && mode != parser::LazyParse) {
        useStaticBuiltinDetected = jsParser.getUseStaticBuiltin();
        checkSourceMappingComment(jsParser);
        jsParser.registerMagicURLs(mcFlags_);
      }
    }
    if (!parsedJs)
      return nullptr;

    // If we have a source map, load it, parse it, and associate it with the
    // file buffer. Note however that the source map translation is not
    // enabled until sourceMapTranslator is set in SourceErrorManager, which
    // we are not doing yet.
    if (!sourceMapURL_.empty())
      if (addSourceMap() != Status::OK)
        return nullptr;

    ESTree::ProgramNode *parsedAST = parsedJs.getValue();

    if (staticBuiltinSetting_ == StaticBuiltinSetting::AutoDetect &&
        useStaticBuiltinDetected) {
      context_->setStaticBuiltinOptimization(true);
    }

    return parsedAST;
  }

 private:
  /// If needed and allowed, extract the source map URL from a magic comment.
  void checkSourceMappingComment(JSParser &parser) {
    if (sourceMapURL_.empty() &&
        smCommentMode_ != SourceMappingCommentMode::Off &&
        !parser.getSourceMappingURL().empty()) {
      sourceMapURL_ = parser.getSourceMappingURL();
      // Note that the magic comment URL is not trusted by default.
      trustedSourceMap_ = false;
      mcFlags_ &= ~JSParser::MCFlag::SourceMappingURL;
    }
  };

  /// If we have a source map, and depending on the comment mode, load it, parse
  /// it, and associate it with the file buffer. Note however that the source
  /// map translation is not enabled until sourceMapTranslator is set in
  /// SourceErrorManager, which we are not doing yet.
  Status addSourceMap() {
    // No source map URL, nothing to do.
    if (sourceMapURL_.empty())
      return Status::OK;
    // If we don't have a source map specified by the user, and detecting of
    // magic comments is off, nothing to do.
    if (!trustedSourceMap_ && smCommentMode_ == SourceMappingCommentMode::Off) {
      return Status::OK;
    }

    // The input directory path is used to resolve relative paths in the source
    // and the map.
    llvh::StringRef inputDirPath = llvh::sys::path::parent_path(
        context_->getSourceErrorManager().getBufferFileName(fileBufId_));

    // The source map buffer will be stored here.
    std::unique_ptr<llvh::MemoryBuffer> mapBuffer{};
    llvh::StringRef scheme = extractScheme(sourceMapURL_);

    if (scheme == "data:") {
      // Data URL.
      //
      if (auto [buf, retVal] = dataURLSourceMap(); buf)
        mapBuffer = std::move(buf);
      else
        return retVal;
    } else if (scheme == "file:" || scheme.empty()) {
      // File URL.
      //
      if (auto [buf, retVal] = fileURLSourceMap(scheme, inputDirPath); buf)
        mapBuffer = std::move(buf);
      else
        return retVal;
    } else {
      // Unsupported scheme.
      //
      return reportError(
          llvh::Twine("Unsupported source map URL scheme: '") + scheme +
          ":' .");
    }

    // Create a new SourceErrorManager, because we don't want to keep the source
    // map buffer around.
    std::unique_ptr<SourceMap> sourceMap{};
    {
      SourceErrorManager sm{};
      SimpleDiagHandler diagHandler{};
      diagHandler.installInto(sm);
      sourceMap = SourceMapParser::parse(*mapBuffer, inputDirPath, sm);
      if (!sourceMap) {
        const llvh::SMDiagnostic &msg = diagHandler.getFirstMessage();
        // Parsing the source map failed.
        return reportError(
            "Source map parse error:(" + mapBuffer->getBufferIdentifier() +
            ":" + llvh::Twine(msg.getLineNo()) + ":" +
            llvh::Twine(msg.getColumnNo() + 1) + "): " + msg.getMessage());
      }
    }

    if (!sourceMapTranslator_) {
      sourceMapTranslator_ = std::make_shared<SourceMapTranslator>(
          context_->getSourceErrorManager());
    }
    sourceMapTranslator_->addSourceMap(fileBufId_, std::move(sourceMap));
    return Status::OK;
  }

  /// Decode and load into a buffer a source map encoded in a data URL.
  ///
  /// \return a buffer and a status. If the buffer is nullptr, we don't have a
  /// source map, but the status indicates whether this is an error. If the
  /// buffer is present, the status doesn't matter.
  std::pair<std::unique_ptr<llvh::MemoryBuffer>, Status> dataURLSourceMap() {
    // NOTE: this check can never pass for now, but we are keeping it here for
    // completeness.
    if (!trustedSourceMap_ && smCommentMode_ < SourceMappingCommentMode::Data) {
      return {
          nullptr,
          reportError("Data URLs in //# sourceMappingURL are disallowed")};
    }

    auto optSMStr = parseJSONBase64DataURL(sourceMapURL_);
    if (!optSMStr)
      return {nullptr, reportError("Failed to parse source map data URL")};

    return {
        std::unique_ptr<llvh::MemoryBuffer>(new StdStringLLVHMemoryBuffer(
            std::move(*optSMStr),
            (llvh::Twine("source map in ") +
             context_->getSourceErrorManager().getBufferFileName(fileBufId_))
                .str())),
        Status::OK};
  }

  /// Load a source map from a file URL into a buffer.
  ///
  /// \param scheme the scheme of the URL, which is either "file:" or empty.
  /// \param inputDirPath the directory of the input file.
  ///
  /// \return a buffer and a status. If the buffer is nullptr, we don't have a
  ///     source map, but the status indicates whether this is an error. If the
  ///     buffer is present, the status doesn't matter.
  std::pair<std::unique_ptr<llvh::MemoryBuffer>, Status> fileURLSourceMap(
      llvh::StringRef scheme,
      llvh::StringRef inputDirPath) {
    if (!trustedSourceMap_ && smCommentMode_ < SourceMappingCommentMode::File) {
      return {
          nullptr,
          reportError("File URLs in //# sourceMappingURL are disallowed")};
    }

    auto path = sourceMapURL_.drop_front(scheme.size());
    llvh::SmallString<64> pathBuf{};
    // If the path was read from a file and is relative, make it absolute by
    // the directory of the input file.
    if (!trustedSourceMap_ && llvh::sys::path::is_relative(path)) {
      pathBuf = inputDirPath;
      llvh::sys::path::append(pathBuf, path);
      path = pathBuf;
    }

    auto mapBuffer =
        memoryBufferFromFile(path, "input source map", false, true);
    return mapBuffer
        ? std::make_pair(std::move(mapBuffer), Status::OK)
        : std::make_pair(
              nullptr, reportError("Failed to read source map file " + path));
  }

  /// Report a problem associated with the source map URL.
  /// If the source map URL is "trusted" (given to us explicitly by the user),
  /// any problem is considered an error and has no location.
  /// If the source map URL was obtained from the JS file, problems are
  /// considered warnings.
  ///
  /// \return Status::OK if the problem is a warning, Status::Error if the
  ///   problem is an error.
  Status reportError(const llvh::Twine &msg) {
    if (trustedSourceMap_) {
      context_->getSourceErrorManager().error(SMLoc{}, msg);
      return Status::Error;
    } else {
      context_->getSourceErrorManager().warning(
          SMLoc::getFromPointer(sourceMapURL_.data()), msg);
      return Status::OK;
    }
  };
}; // class ParseJSFile

} // anonymous namespace

ESTree::ProgramNode *parseJSFile(
    Context *context,
    SourceMappingCommentMode smCommentsMode,
    StaticBuiltinSetting staticBuiltinSetting,
    unsigned fileBufId,
    llvh::StringRef inputSourceMap,
    std::shared_ptr<SourceMapTranslator> &sourceMapTranslator) {
  return ParseJSFile(
             context,
             smCommentsMode,
             staticBuiltinSetting,
             fileBufId,
             inputSourceMap,
             sourceMapTranslator)
      .run();
}

/// Loads global definitions from MemoryBuffer and adds the definitions to \p
/// declFileList.
/// \return true on success, false on error.
bool loadGlobalDefinition(
    Context &context,
    std::unique_ptr<llvh::MemoryBuffer> content,
    DeclarationFileListTy &declFileList) {
  parser::JSParser jsParser(context, std::move(content));
  auto parsedJs = jsParser.parse();
  if (!parsedJs)
    return false;
  jsParser.registerMagicURLs();

  declFileList.push_back(parsedJs.getValue());
  return true;
}

std::unique_ptr<llvh::MemoryBuffer> memoryBufferFromFile(
    llvh::StringRef path,
    llvh::StringRef pathDesc,
    bool stdinOk,
    bool silent) {
  auto fileBuf = stdinOk ? llvh::MemoryBuffer::getFileOrSTDIN(path)
                         : llvh::MemoryBuffer::getFile(path);
  if (!fileBuf) {
    if (!silent) {
      llvh::errs() << "Error! Failed to open "
                   << (pathDesc.empty() ? "file" : pathDesc) << ": " << path
                   << '\n';
    }
    return nullptr;
  }
  return std::move(*fileBuf);
}

} // namespace hermes
