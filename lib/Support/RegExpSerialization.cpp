/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/RegExpSerialization.h"
#include "hermes/Regex/Regex.h"
#include "hermes/Regex/RegexTraits.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/Support/UTF8.h"
#include "llvh/Support/Format.h"
#include "llvh/Support/raw_ostream.h"

#include <cctype>

using llvh::StringRef;
using namespace hermes;

/// Support for dumping regex bytecode.
namespace {
/// Base implementation of dumpInstruction() which just outputs the opcode.
/// Overloads may be used to specialize this.
void dumpInstruction(const regex::Insn *insn, llvh::raw_ostream &OS) {
/// Dump the instruction stream to a raw_ostream \p OS.
/// The default implementation of dump() outputs the opcode name only.
/// Note this is NOT a virtual function: we can't store a vtable since
/// instructions must be directly serializable.
#define REOP(Code)          \
  case regex::Opcode::Code: \
    OS << #Code;            \
    break;
  switch (insn->opcode) {
#include "hermes/Regex/RegexOpcodes.def"
  }
}

/// aligner can take a value and force it to the alignment for its type, even if
/// the value came from a packed struct in memory.
template <typename T>
T aligner(T v) {
  return v;
}

/// Helpers to compute the width of an instruction. All instructions are
/// fixed-width, except for brackets, MatchNChar8Insn, and MatchNCharICase8Insn.
template <typename Insn>
uint32_t instructionWidth(const Insn *insn) {
  return sizeof *insn;
}

template <>
uint32_t instructionWidth<regex::BracketInsn>(const regex::BracketInsn *insn) {
  return insn->totalWidth();
}

template <>
uint32_t instructionWidth<regex::U16BracketInsn>(
    const regex::U16BracketInsn *insn) {
  return insn->totalWidth();
}

template <>
uint32_t instructionWidth<regex::MatchNChar8Insn>(
    const regex::MatchNChar8Insn *insn) {
  return insn->totalWidth();
}

template <>
uint32_t instructionWidth<regex::MatchNCharICase8Insn>(
    const regex::MatchNCharICase8Insn *insn) {
  return insn->totalWidth();
}

void dumpInstruction(const regex::MatchChar8Insn *insn, llvh::raw_ostream &OS) {
  OS << "MatchChar8: ";
  char c = insn->c;
  if (std::isprint(c))
    OS << llvh::format("'%c'", c);
  else
    OS << llvh::format_hex(c, 4);
}

void dumpInstruction(
    const regex::MatchChar16Insn *insn,
    llvh::raw_ostream &OS) {
  OS << "MatchChar16: ";
  char32_t c = insn->c;
  OS << llvh::format_hex(c, 4);
}

void dumpInstruction(
    const regex::U16MatchChar32Insn *insn,
    llvh::raw_ostream &OS) {
  OS << "U16MatchChar32: ";
  uint32_t c = insn->c;
  OS << llvh::format_hex(c, 6);
}

void dumpInstruction(
    const regex::MatchNChar8Insn *insn,
    llvh::raw_ostream &OS) {
  OS << "MatchNChar8: '";
  const char *cPtr = reinterpret_cast<const char *>(insn + sizeof(regex::Insn));
  for (uint32_t i = 0; i < insn->charCount; i++) {
    char c = *cPtr;
    if (std::isprint(c)) {
      OS << llvh::format("%c", c);
    } else {
      OS << llvh::format_hex(c, 4);
    }
    cPtr++;
  }
  OS << "'";
}

void dumpInstruction(
    const regex::MatchNCharICase8Insn *insn,
    llvh::raw_ostream &OS) {
  OS << "MatchNCharICase8: '";
  const char *cPtr = reinterpret_cast<const char *>(insn + sizeof(regex::Insn));
  for (uint32_t i = 0; i < insn->charCount; i++) {
    char c = *cPtr;
    if (std::isprint(c)) {
      OS << llvh::format("%c", c);
    } else {
      OS << llvh::format_hex(c, 4);
    }
    cPtr++;
  }
  OS << "'";
}

void dumpInstruction(
    const regex::MatchCharICase8Insn *insn,
    llvh::raw_ostream &OS) {
  OS << "MatchCharICase8: ";
  char c = insn->c;
  if (std::isprint(c))
    OS << llvh::format("'%c'", c);
  else
    OS << llvh::format_hex(c, 4);
}

void dumpInstruction(
    const regex::MatchCharICase16Insn *insn,
    llvh::raw_ostream &OS) {
  OS << "MatchCharICase16: ";
  char32_t c = insn->c;
  OS << llvh::format_hex(c, 4);
}

void dumpInstruction(
    const regex::U16MatchCharICase32Insn *insn,
    llvh::raw_ostream &OS) {
  OS << "U16MatchCharICase32: ";
  uint32_t c = insn->c;
  OS << llvh::format_hex(c, 6);
}

void dumpInstruction(const regex::Jump32Insn *insn, llvh::raw_ostream &OS) {
  OS << "Jump32: " << llvh::format_hex(insn->target, 4);
}

void dumpInstruction(
    const regex::AlternationInsn *insn,
    llvh::raw_ostream &OS) {
  OS << "Alternation: Target " << llvh::format_hex(insn->secondaryBranch, 4)
     << ", constraints " << unsigned(insn->primaryConstraints) << ","
     << unsigned(insn->secondaryConstraints);
}

void dumpInstruction(const regex::BracketInsn *insn, llvh::raw_ostream &OS) {
  using namespace regex;
  OS << (insn->opcode == Opcode::U16Bracket ? "U16Bracket" : "Bracket")
     << ": [";
  if (insn->negate)
    OS << '^';
  if (insn->positiveCharClasses & CharacterClass::Digits)
    OS << "\\d";
  if (insn->positiveCharClasses & CharacterClass::Spaces)
    OS << "\\s";
  if (insn->positiveCharClasses & CharacterClass::Words)
    OS << "\\w";
  if (insn->negativeCharClasses & CharacterClass::Digits)
    OS << "\\D";
  if (insn->negativeCharClasses & CharacterClass::Spaces)
    OS << "\\S";
  if (insn->negativeCharClasses & CharacterClass::Words)
    OS << "\\W";

  auto output1Char = [&OS](uint32_t c) {
    if (c <= 127 && std::isprint(c))
      OS << char(c);
    else
      OS << llvh::format_hex(c, 4);
  };
  // BracketRange32 immediately follow insn.
  const BracketRange32 *range =
      reinterpret_cast<const BracketRange32 *>(1 + insn);
  for (uint32_t i = 0; i < insn->rangeCount; i++) {
    output1Char(range->start);
    if (range->end > range->start) {
      OS << '-';
      output1Char(range->end);
    }
    range++;
  }
  OS << ']';
}

void dumpInstruction(
    const regex::WordBoundaryInsn *insn,
    llvh::raw_ostream &OS) {
  OS << "WordBoundary: " << (insn->invert ? "\\B" : "\\b");
}

void dumpInstruction(
    const regex::BeginMarkedSubexpressionInsn *insn,
    llvh::raw_ostream &OS) {
  OS << "BeginMarkedSubexpression: " << insn->mexp;
}

void dumpInstruction(
    const regex::EndMarkedSubexpressionInsn *insn,
    llvh::raw_ostream &OS) {
  OS << "EndMarkedSubexpression: " << insn->mexp;
}

void dumpInstruction(const regex::BackRefInsn *insn, llvh::raw_ostream &OS) {
  OS << "BackRefInsn: " << insn->mexp;
}

void dumpInstruction(const regex::LookaroundInsn *insn, llvh::raw_ostream &OS) {
  OS << "Lookaround: " << (insn->forwards ? "" : "<")
     << (insn->invert ? '!' : '=')
     << " (constraints: " << unsigned(insn->constraints)
     << ", marked expressions=[" << insn->mexpBegin << "," << insn->mexpEnd
     << "), continuation " << llvh::format_hex(insn->continuation, 4) << ')';
}

void dumpInstruction(const regex::BeginLoopInsn *insn, llvh::raw_ostream &OS) {
  OS << llvh::format(
      "BeginLoop: %u %s {%u, %u} (constraints: %u)",
      aligner(insn->loopId),
      insn->greedy ? "greedy" : "nongreedy",
      aligner(insn->min),
      aligner(insn->max),
      aligner(insn->loopeeConstraints));
}

void dumpInstruction(const regex::EndLoopInsn *insn, llvh::raw_ostream &OS) {
  OS << "EndLoop: " << llvh::format_hex(insn->target, 4);
}

void dumpInstruction(
    const regex::BeginSimpleLoopInsn *insn,
    llvh::raw_ostream &OS) {
  OS << llvh::format(
      "BeginSimpleLoop: (constraints: %u)", insn->loopeeConstraints);
}

void dumpInstruction(
    const regex::EndSimpleLoopInsn *insn,
    llvh::raw_ostream &OS) {
  OS << "EndSimpleLoop: " << llvh::format_hex(insn->target, 4);
}

void dumpInstruction(const regex::Width1LoopInsn *insn, llvh::raw_ostream &OS) {
  OS << llvh::format(
      "Width1Loop: %u %s {%u, %u}",
      aligner(insn->loopId),
      insn->greedy ? "greedy" : "nongreedy",
      aligner(insn->min),
      aligner(insn->max));
}
} // namespace

namespace hermes {

void dumpRegexBytecode(llvh::ArrayRef<uint8_t> bytes, llvh::raw_ostream &OS) {
  // Output the header and then slice it off.
  auto *header =
      reinterpret_cast<const regex::RegexBytecodeHeader *>(bytes.data());
  OS << llvh::format(
      "  Header: marked: %u loops: %u flags: %u constraints: %u\n",
      aligner(header->markedCount),
      aligner(header->loopCount),
      aligner(header->syntaxFlags),
      header->constraints);
  bytes = bytes.slice(sizeof *header);
  uint32_t cursor = 0;
  while (cursor < bytes.size()) {
    // Output offset in left column.
    OS << "  " << llvh::format_hex_no_prefix(cursor, 4) << "  ";

    // Call dumpInstruction() with its derived type.
    auto insn = reinterpret_cast<const regex::Insn *>(&bytes[cursor]);
    switch (insn->opcode) {
#define REOP(Code)                                          \
  case regex::Opcode::Code: {                               \
    auto derivedInsn = llvh::cast<regex::Code##Insn>(insn); \
    dumpInstruction(derivedInsn, OS);                       \
    cursor += instructionWidth(derivedInsn);                \
    break;                                                  \
  }
#include "hermes/Regex/RegexOpcodes.def"
    }
    OS << '\n';
  }
  // We expect to have consumed exactly the size of the stream.
  assert(cursor == bytes.size() && "Invalid instructions in regex stream");
}

CompiledRegExp::CompiledRegExp(CompiledRegExp &&) = default;
CompiledRegExp &CompiledRegExp::operator=(CompiledRegExp &&) = default;
CompiledRegExp::~CompiledRegExp() {}
CompiledRegExp::CompiledRegExp(
    std::vector<uint8_t> bytecode,
    std::string pattern,
    std::string flags,
    std::deque<llvh::SmallVector<char16_t, 5>> &&orderedGroupNames,
    regex::ParsedGroupNamesMapping &&mapping)
    : bytecode_(std::move(bytecode)),
      pattern_(std::move(pattern)),
      flags_(std::move(flags)),
      orderedGroupNames_(std::move(orderedGroupNames)),
      mapping_(std::move(mapping)) {}

llvh::Optional<CompiledRegExp> CompiledRegExp::tryCompile(
    StringRef pattern,
    StringRef flags,
    llvh::StringRef *outError) {
  using namespace regex::constants;

  // We have to match the way strings are constructed by StringTable, which is
  // by interpreting UTF-8 encoded surrogates as UTF-16 surrogates.
  llvh::SmallVector<char16_t, 16> re16;
  convertUTF8WithSurrogatesToUTF16(
      std::back_inserter(re16), pattern.begin(), pattern.end());

  llvh::SmallVector<char16_t, 6> flags16;
  convertUTF8WithSurrogatesToUTF16(
      std::back_inserter(flags16), flags.begin(), flags.end());
  // Build and compile the regexp.
  auto re = regex::Regex<regex::UTF16RegexTraits>(re16, flags16);
  if (!re.valid()) {
    if (outError)
      *outError = messageForError(re.getError());
    return llvh::None;
  }
  return CompiledRegExp(
      re.compile(),
      pattern,
      flags,
      re.acquireOrderedGroupNames(),
      re.acquireGroupNamesMapping());
}

llvh::ArrayRef<uint8_t> CompiledRegExp::getBytecode() const {
  return bytecode_;
}

std::vector<RegExpTableEntry> UniquingRegExpTable::getEntryList() const {
  std::vector<RegExpTableEntry> result;
  result.reserve(regexps_.size());
  uint32_t offset = 0;
  for (const auto &re : regexps_) {
    uint32_t size = re->getBytecode().size();
    result.push_back(RegExpTableEntry{offset, size});
    offset += size;
  }
  return result;
}

RegExpBytecode UniquingRegExpTable::getBytecodeBuffer() const {
  RegExpBytecode result;
  for (const auto &re : regexps_) {
    auto bytecode = re->getBytecode();
    result.insert(result.end(), bytecode.begin(), bytecode.end());
  }
  return result;
}

void UniquingRegExpTable::disassemble(llvh::raw_ostream &OS) const {
  uint32_t index = 0;
  // Note that regexp patterns and flags are read without interpreting escapes
  // by JSLexer, so this will output in a format identical to the source
  // pattern (preserving escapes, etc) except that non-BMP characters will have
  // been encoded in UTF-8 as two UTF-16 surrogates.
  for (const auto &regexp : regexps_) {
    OS << index << ": /" << regexp->getPattern() << '/' << regexp->getFlags()
       << '\n';
    dumpRegexBytecode(regexp->getBytecode(), OS);
    index++;
  }
  OS << '\n';
}

} // namespace hermes
