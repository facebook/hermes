/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Parser/JSParser.h"

#include "JSParserImpl.h"

using llvh::cast;
using llvh::dyn_cast;
using llvh::isa;

namespace hermes {
namespace parser {

JSParser::JSParser(Context &context, std::unique_ptr<llvh::MemoryBuffer> input)
    : impl_(new detail::JSParserImpl(context, std::move(input))) {}

JSParser::JSParser(Context &context, uint32_t bufferId, ParserPass pass)
    : impl_(new detail::JSParserImpl(context, bufferId, pass)) {}

JSParser::~JSParser() = default;

Context &JSParser::getContext() {
  return impl_->getContext();
}

bool JSParser::isStrictMode() const {
  return impl_->isStrictMode();
}

void JSParser::setStrictMode(bool mode) {
  return impl_->setStrictMode(mode);
}

llvh::ArrayRef<StoredComment> JSParser::getStoredComments() const {
  return impl_->getStoredComments();
}

void JSParser::setStoreComments(bool storeComments) {
  impl_->setStoreComments(storeComments);
}

bool JSParser::getUseStaticBuiltin() const {
  return impl_->getUseStaticBuiltin();
}

llvh::Optional<ESTree::ProgramNode *> JSParser::parse() {
  return impl_->parse();
}

void JSParser::seek(SMLoc startPos) {
  return impl_->seek(startPos);
}

bool JSParser::preParseBuffer(
    Context &context,
    uint32_t bufferId,
    bool &useStaticBuiltinDetected) {
  return detail::JSParserImpl::preParseBuffer(
      context, bufferId, useStaticBuiltinDetected);
}

llvh::Optional<ESTree::NodePtr> JSParser::parseLazyFunction(
    ESTree::NodeKind kind,
    SMLoc start) {
  return impl_->parseLazyFunction(kind, start);
}

} // namespace parser
} // namespace hermes
