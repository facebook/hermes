/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

include!(concat!(env!("OUT_DIR"), "/generated.rs"));
mod generated_extension;

use generated_extension::convert_comment;
pub use generated_extension::Comment;
use generated_extension::Context;
use generated_extension::FromHermes;
use hermes::parser::HermesParser;
pub use hermes::parser::ParserDialect;
pub use hermes::parser::ParserFlags;
use hermes::utf::utf8_with_surrogates_to_string;
use hermes_diagnostics::Diagnostic;
use hermes_estree::Program;
use juno_support::NullTerminatedBuf;

pub struct ParseResult {
    pub ast: Program,
    pub comments: Vec<Comment>,
}

pub fn parse(
    source: &str,
    _file: &str,
    flags: ParserFlags,
) -> Result<ParseResult, Vec<Diagnostic>> {
    let buf = NullTerminatedBuf::from_str_check(source);
    let result = HermesParser::parse(flags, &buf);
    let mut cx = Context::new(&buf);
    if result.has_errors() {
        let error_messages = result.messages();
        return Err(error_messages
            .iter()
            .map(|diag| {
                let message = utf8_with_surrogates_to_string(diag.message.as_slice()).unwrap();
                Diagnostic::invalid_syntax(message, None)
            })
            .collect());
    }

    let comments = result
        .comments()
        .iter()
        .map(|comment| convert_comment(&mut cx, comment))
        .collect();

    Ok(ParseResult {
        ast: FromHermes::convert(&mut cx, result.root().unwrap()),
        comments,
    })
}
