/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_JSON_H
#define HERMES_SUPPORT_JSON_H

#include "hermes/Platform/Unicode/CharacterProperties.h"

namespace hermes {

template <typename Output>
void appendUTF16Escaped(Output &output, char16_t cp) {
  auto toLowerHex = [](uint8_t u) {
    assert(u <= 0xF);
    return u"0123456789abcdef"[u];
  };
  output.append({u'\\', u'u'});
  output.push_back(toLowerHex(cp >> 12));
  output.push_back(toLowerHex((cp >> 8) & 0xF));
  output.push_back(toLowerHex((cp >> 4) & 0xF));
  output.push_back(toLowerHex(cp & 0xF));
}

// If there is a valid surrogate pair at position \p i in \p view, then write
// both the high and low surrogate into \p output. Otherwise, write an escaped
// UTF16 value into \p output. \return true if a pair was found.
template <typename Output, typename StringView>
bool handleSurrogate(Output &output, StringView view, size_t i) {
  char16_t ch = view[i];
  assert(
      ch >= UNICODE_SURROGATE_FIRST && ch <= UNICODE_SURROGATE_LAST &&
      "charcter should be a surrogate character");
  // Handle well-formed-ness here: Represent unpaired surrogate code points as
  // JSON escape sequences.
  if (isHighSurrogate(ch) && i + 1 < view.length()) {
    char16_t next = view[i + 1];
    if (isLowSurrogate(next)) {
      // We found a surrogate pair. Simply write both of them unescaped to the
      // output.
      output.push_back(ch);
      output.push_back(next);
      return true;
    }
  }
  // We did not find a valid pair, so the current surrogate character must be
  // written as an escaped JSON sequence.
  appendUTF16Escaped(output, ch);
  return false;
}

/// Quotes a string given by \p view and puts the quoted version into \p output.
/// \p view should be utf16-encoded, and \p output will be as well.
/// \post output is a container that has a sequential list of utf16 characters
///   that can be embedded into a JSON string.
template <typename Output, typename StringView>
void quoteStringForJSON(Output &output, StringView view) {
  // Quote.1.
  output.push_back(u'"');
  // Quote.2.
  for (size_t i = 0; i < view.length(); i++) {
    char16_t ch = view[i];
#define ESCAPE(ch, replace)    \
  case ch:                     \
    output.push_back(u'\\');   \
    output.push_back(replace); \
    break

    switch (ch) {
      // Quote.2.a.
      ESCAPE(u'\\', u'\\');
      ESCAPE(u'"', u'"');
      // Quote.2.b.
      ESCAPE(u'\b', u'b');
      ESCAPE(u'\f', u'f');
      ESCAPE(u'\n', u'n');
      ESCAPE(u'\r', u'r');
      ESCAPE(u'\t', u't');
      default:
        if (ch < u' ') {
          // Quote.2.c.
          output.append({u'\\', u'u', u'0', u'0'});
          output.push_back(u'0' + (ch / 16));
          if (ch % 16 < 10) {
            output.push_back(u'0' + (ch % 16));
          } else {
            output.push_back(u'a' + (ch % 16 - 10));
          }
        } else {
          if (ch >= UNICODE_SURROGATE_FIRST && ch <= UNICODE_SURROGATE_LAST) {
            if (handleSurrogate(output, view, i)) {
              // Found a valid surrogate pair, so skip over the next character.
              i++;
            }
          } else {
            // Quote.2.d.
            output.push_back(ch);
          }
        }
    }
  }
  // Quote.3.
  output.push_back(u'"');
}

} // namespace hermes

#endif
