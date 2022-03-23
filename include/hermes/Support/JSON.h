/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_JSON_H
#define HERMES_SUPPORT_JSON_H

namespace hermes {

/// Quotes a string given by \p view and puts the quoted version into \p output.
/// \p view should be utf16-encoded, and \p output will be as well.
/// \post output is a container that has a sequential list of utf16 characters
///   that can be embedded into a JSON string.
template <typename Output, typename StringView>
void quoteStringForJSON(Output &output, StringView view) {
  // Quote.1.
  output.push_back(u'"');
  // Quote.2.
  for (char16_t ch : view) {
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
          // Quote.2.d.
          output.push_back(ch);
        }
    }
  }
  // Quote.3.
  output.push_back(u'"');
}

} // namespace hermes

#endif
