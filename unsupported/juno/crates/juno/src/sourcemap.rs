/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use sourcemap::SourceMap;
use sourcemap::SourceMapBuilder;

/// Return a merged version of the `input` and `output` source maps.
/// Currently this only supports a single input source map.
/// The function works by iterating through every `Token` in `output` and attempting
/// to look it up in `input` (use the `lookup_token` method to get the closest token near it).
/// If `input` does not contain the token, use the `output` token.
/// Currently the only metadata retained from the `input` map is `names` and `sources`.
/// TODO: Add support for other sourcemap metadata fields such as `metadata` and `sourcesContent`.
pub fn merge_sourcemaps(input: &SourceMap, output: &SourceMap) -> SourceMap {
    let mut merged = SourceMapBuilder::new(output.get_file());

    for name in input.names() {
        merged.add_name(name);
    }

    let mut output_src_id = 0;
    for src in input.sources() {
        output_src_id = merged.add_source(src) + 1;
    }
    for output_token in output.tokens() {
        let (merged_token, src_id) =
            match input.lookup_token(output_token.get_src_line(), output_token.get_src_col()) {
                Some(input_token) => (input_token, input_token.get_src_id()),
                None => (output_token, output_src_id),
            };

        merged.add_raw(
            output_token.get_dst_line(),
            output_token.get_dst_col(),
            merged_token.get_src_line(),
            merged_token.get_src_col(),
            Some(src_id),
            match merged_token.get_name() {
                Some(_) => Some(merged_token.get_name_id()),
                _ => None,
            },
        );
    }
    merged.into_sourcemap()
}
