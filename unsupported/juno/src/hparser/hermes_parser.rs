/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::node::{Node, NodePtr, NodePtrOpt, SMLoc, StringRef};
use crate::hermes_utf::utf8_with_surrogates_to_string;
use libc::c_int;
use std::fmt::Formatter;
use std::marker::PhantomData;
use std::mem::MaybeUninit;
use std::os::raw::c_char;

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Coord {
    pub line: c_int,
    pub column: c_int,
}

#[derive(Clone, Copy, Debug)]
#[repr(u32)]
pub enum DiagKind {
    Error,
    Warning,
    Remark,
    Note,
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct DiagMessage {
    /// Location.
    pub loc: SMLoc,
    /// Source coordinate.
    pub coord: Coord,
    /// What kind of message.
    pub diag_kind: DiagKind,
    /// Error message.
    pub message: StringRef,
    /// Contents of the error line.
    pub line_contents: StringRef,
}

impl std::fmt::Display for DiagMessage {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}:{}:{}",
            self.coord.line,
            self.coord.column,
            utf8_with_surrogates_to_string(self.message.as_slice()).unwrap()
        )
    }
}

#[repr(u32)]
pub enum MagicCommentKind {
    SourceUrl = 0,
    SourceMappingUrl = 1,
}

#[repr(C)]
#[derive(Copy, Clone)]
struct DataRef<'a, T: 'a> {
    data: *const T,
    length: usize,
    _marker: PhantomData<&'a T>,
}

impl<'a, T> DataRef<'a, T> {
    pub fn is_empty(&self) -> bool {
        self.length == 0
    }
    pub fn as_slice(&self) -> &'a [T] {
        if self.data.is_null() {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.data, self.length) }
        }
    }
}

#[repr(C)]
struct ParserContext {
    _unused: i32,
}

extern "C" {
    /// Note: source[len-1] must be 0.
    fn hermes_parser_parse(source: *const c_char, len: usize) -> *mut ParserContext;
    fn hermes_parser_free(parser_ctx: *mut ParserContext);
    fn hermes_parser_get_first_error(parser_ctx: *const ParserContext) -> isize;
    fn hermes_parser_get_messages<'a>(parser_ctx: *const ParserContext)
        -> DataRef<'a, DiagMessage>;
    fn hermes_parser_get_ast(parser_ctx: *const ParserContext) -> NodePtrOpt;
    fn hermes_parser_find_location(
        parser_ctx: *mut ParserContext,
        loc: SMLoc,
        res: *mut Coord,
    ) -> bool;
    /// Return a magic comment or an empty string. The string is always guaranteed to be valid UTF-8.
    fn hermes_parser_get_magic_comment<'a>(
        parser_ctx: *mut ParserContext,
        kind: MagicCommentKind,
    ) -> DataRef<'a, u8>;
    fn hermes_get_node_name(node: NodePtr) -> DataRef<'static, u8>;
}

pub struct HermesParser {
    /// A pointer to the opaque C++ parser object. It should never be null.
    parser_ctx: *mut ParserContext,
    /// If the input is not zero-terminated, we create a zero-terminated copy
    /// here.
    tmpbuf: Vec<u8>,
}

impl Drop for HermesParser {
    fn drop(&mut self) {
        unsafe { hermes_parser_free(self.parser_ctx) }
    }
}

impl HermesParser {
    /// `file_id` is an opaque value used for encoding source coordinates.
    /// To avoid copying `source` can optionally be NUL-terminated.
    pub fn parse(source: &str) -> HermesParser {
        let bytes = source.as_bytes();

        // Optional temporary copy for zero termination.
        let mut tmpbuf = Vec::new();
        // Zero terminated source reference.
        let source_z = if let [.., 0] = bytes {
            bytes
        } else {
            tmpbuf.reserve_exact(bytes.len() + 1);
            tmpbuf.extend_from_slice(bytes);
            tmpbuf.push(0u8);
            tmpbuf.as_slice()
        };

        let parser_ctx =
            unsafe { hermes_parser_parse(source_z.as_ptr() as *const i8, source_z.len()) };

        HermesParser { parser_ctx, tmpbuf }
    }

    /// Return the index of the first parser error (there could be warnings before it).
    pub fn first_error_index(&self) -> Option<usize> {
        let index = unsafe { hermes_parser_get_first_error(self.parser_ctx) };
        if index < 0 {
            None
        } else {
            Some(index as usize)
        }
    }

    /// Return true if there was at least one parser error. That implies that there is no AST.
    pub fn has_errors(&self) -> bool {
        self.first_error_index().is_some()
    }

    /// Return a slice containing all parser messages.
    pub fn messages(&self) -> &[DiagMessage] {
        unsafe { hermes_parser_get_messages(self.parser_ctx).as_slice() }
    }

    /// Return the root AST node if the parse we successful, None otherwise.
    pub fn root(&self) -> Option<NodePtr> {
        unsafe { hermes_parser_get_ast(self.parser_ctx) }.as_node_ptr()
    }

    /// Translate a source coordinate represented as a SMLoc (a pointer) into
    /// line and column.
    pub fn find_coord(&self, loc: SMLoc) -> Option<Coord> {
        let mut res = MaybeUninit::<Coord>::uninit();
        if unsafe { hermes_parser_find_location(self.parser_ctx, loc, res.as_mut_ptr()) } {
            Some(unsafe { res.assume_init() })
        } else {
            None
        }
    }

    /// Return the last magic comment of the specified type (each comment overrides the previous
    /// one, so only the last is recorded).
    pub fn magic_comment(&self, kind: MagicCommentKind) -> Option<&str> {
        let comment = unsafe { hermes_parser_get_magic_comment(self.parser_ctx, kind) };
        if comment.is_empty() {
            None
        } else {
            Some(unsafe { std::str::from_utf8_unchecked(comment.as_slice()) })
        }
    }

    pub fn node_name(n: &Node) -> &str {
        unsafe { std::str::from_utf8_unchecked(hermes_get_node_name(NodePtr::new(n)).as_slice()) }
    }
}

#[cfg(test)]
mod tests {
    use super::super::generated_ffi::NodeKind;
    use super::*;

    #[test]
    fn good_parse() {
        let p = HermesParser::parse("var x = 10;\0");
        assert!(!p.has_errors());
        assert_eq!(p.root().unwrap().as_ref().kind, NodeKind::Program);
    }

    #[test]
    fn parse_error() {
        let p = HermesParser::parse("var x+ = 10;");
        assert!(p.has_errors());
        assert!(p.root().is_none());

        let messages = p.messages();
        assert_eq!(messages.len(), 1);
        assert_eq!(
            p.find_coord(messages[0].loc).unwrap(),
            Coord { line: 1, column: 6 }
        );
    }

    #[test]
    fn magic_comments() {
        let p = HermesParser::parse(
            "var p = 0;
            //# sourceURL=1
            //# sourceMappingURL=my map URL
            //# sourceURL=my source URL
            ",
        );
        assert!(!p.has_errors());
        assert_eq!(
            p.magic_comment(MagicCommentKind::SourceUrl).unwrap(),
            "my source URL"
        );
        assert_eq!(
            p.magic_comment(MagicCommentKind::SourceMappingUrl).unwrap(),
            "my map URL"
        );
    }
}
