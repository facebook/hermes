/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::node::Node;
use super::node::NodePtr;
use super::node::NodePtrOpt;
use super::node::SMLoc;
use super::node::StringRef;
use crate::utf::utf8_with_surrogates_to_string_lossy;
use juno_support::NullTerminatedBuf;
use std::fmt::Formatter;
use std::marker::PhantomData;
use std::mem::MaybeUninit;
use std::os::raw::c_char;
use std::os::raw::c_uint;

#[repr(u8)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum ParserDialect {
    /// Good old JS.
    JavaScript,
    /// Parse all Flow type syntax.
    Flow,
    /// Parse all unambiguous Flow type syntax. Syntax that can be intepreted as
    /// either Flow types or standard JavaScript is parsed as if it were standard
    /// JavaScript.
    ///
    /// For example, `foo<T>(x)` is parsed as if it were standard JavaScript
    /// containing two comparisons, even though it could otherwise be interpreted
    /// as a call expression with Flow type arguments.
    FlowUnambiguous,
    /// Look for the '@flow' pragma in the first comment in the JS file.
    /// If it exists, then parse all Flow type syntax, otherwise only parse
    /// Flow type syntax according to the FlowUnambiguous mode.
    FlowDetect,
    /// Parse TypeScript.
    TypeScript,
}

/// Flags controlling the behavior of the parser.
#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ParserFlags {
    /// Start parsing in strict mode.
    pub strict_mode: bool,
    /// Enable JSX parsing.
    pub enable_jsx: bool,
    /// Dialect control.
    pub dialect: ParserDialect,
    /// Store doc-comment block at the top of the file.
    pub store_doc_block: bool,
}

impl Default for ParserFlags {
    fn default() -> Self {
        ParserFlags {
            strict_mode: false,
            enable_jsx: false,
            dialect: ParserDialect::JavaScript,
            store_doc_block: false,
        }
    }
}

#[derive(Clone, Copy, Debug)]
#[repr(u32)]
pub enum DiagKind {
    Error,
    Warning,
    Remark,
    Note,
}

#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct DataRef<'a, T: 'a> {
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

    /// Returns a new `DataRef` which is one byte longer than this one.
    ///
    /// # Safety
    ///
    /// This is only safe when the user is certain that one more byte past
    /// the end of the `DataRef` is still valid memory, such as when we know the `data` is
    /// null-terminated.
    pub unsafe fn extend_by_1(&self) -> Self {
        Self {
            data: self.data,
            length: self.length + 1,
            _marker: self._marker,
        }
    }

    // Note that the Clippy warning is bogus since we aren't deferencing a pointer.
    #[allow(clippy::not_unsafe_ptr_arg_deref)]
    pub fn try_offset_from(&self, ptr: *const T) -> Option<usize> {
        unsafe {
            if ptr >= self.data && ptr < self.data.add(self.length) {
                Some(ptr.offset_from(self.data) as usize)
            } else {
                None
            }
        }
    }
}

impl<T> Default for DataRef<'_, T> {
    fn default() -> Self {
        DataRef {
            data: std::ptr::null(),
            length: 0,
            _marker: Default::default(),
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Coord {
    /// 1-based line.
    pub line: c_uint,
    /// 0-based offset from start of line.
    pub offset: c_uint,
}

/// Result from looking for a line in the input buffer. Contains the 1-based
/// line number and a reference to the line itself in the buffer.
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct LineCoord<'a> {
    /// 1-based line number.
    pub line_no: c_uint,
    /// Reference to the line itself, including the EOL, if present.
    pub line_ref: DataRef<'a, u8>,
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
            self.coord.offset + 1,
            utf8_with_surrogates_to_string_lossy(self.message.as_slice())
        )
    }
}

#[repr(u32)]
#[derive(Copy, Clone)]
pub enum MagicCommentKind {
    SourceUrl = 0,
    SourceMappingUrl = 1,
}

impl MagicCommentKind {
    pub fn name(self) -> &'static str {
        match self {
            MagicCommentKind::SourceUrl => "sourceUrl",
            MagicCommentKind::SourceMappingUrl => "sourceMappingUrl",
        }
    }
}

#[repr(C)]
struct ParserContext {
    _unused: i32,
}

extern "C" {
    /// Note: source[len-1] must be 0.
    fn hermes_parser_parse(
        flags: ParserFlags,
        source: *const c_char,
        len: usize,
    ) -> *mut ParserContext;
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
    /// Return the line surrounding the specified location \p loc. This method
    /// allows the caller to calculate the location column taking UTF-8 into
    /// consideration and to perform its own location caching.
    fn hermes_parser_find_line(
        parser_ctx: *const ParserContext,
        loc: SMLoc,
        res: *mut LineCoord,
    ) -> bool;
    /// Return a reference to the specified (1-based) line.
    /// If the line is greater than the last line in the buffer, an empty
    /// reference is returned.
    fn hermes_parser_get_line_ref<'a>(
        parser_ctx: *const ParserContext,
        line: c_uint,
    ) -> DataRef<'a, u8>;
    /// Return a magic comment or an empty string. The string is always guaranteed to be valid UTF-8.
    fn hermes_parser_get_magic_comment<'a>(
        parser_ctx: *const ParserContext,
        kind: MagicCommentKind,
    ) -> DataRef<'a, u8>;
    fn hermes_get_node_name(node: NodePtr) -> DataRef<'static, u8>;
    /// Return the doc block for the file if `storeDocBlock` was provided at parse time.
    fn hermes_parser_get_doc_block<'a>(parser_ctx: *const ParserContext) -> DataRef<'a, u8>;
}

pub struct HermesParser<'a> {
    /// A pointer to the opaque C++ parser object. It should never be null.
    parser_ctx: *mut ParserContext,
    /// If the input is not zero-terminated, we create a zero-terminated copy
    /// here.
    source: &'a NullTerminatedBuf,
}

impl Drop for HermesParser<'_> {
    fn drop(&mut self) {
        unsafe { hermes_parser_free(self.parser_ctx) }
    }
}

impl HermesParser<'_> {
    /// `file_id` is an opaque value used for encoding source coordinates.
    pub fn parse(flags: ParserFlags, source: &NullTerminatedBuf) -> HermesParser {
        HermesParser {
            parser_ctx: unsafe { hermes_parser_parse(flags, source.as_c_char_ptr(), source.len()) },
            source,
        }
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

    /// Return the doc block at the top of the file if it exists.
    pub fn get_doc_block(&self) -> Option<&str> {
        let result = unsafe { hermes_parser_get_doc_block(self.parser_ctx) };
        if result.is_empty() {
            None
        } else {
            Some(unsafe { std::str::from_utf8_unchecked(result.as_slice()) })
        }
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

    /// Find the line surrounding the specified location. This method enables
    /// the caller to calculate the column taking UTF-8 into consideration and
    /// to perform its own location caching.
    pub fn find_line(&self, loc: SMLoc) -> Option<LineCoord> {
        let mut res = MaybeUninit::<LineCoord>::uninit();
        if unsafe { hermes_parser_find_line(self.parser_ctx, loc, res.as_mut_ptr()) } {
            Some(unsafe { res.assume_init() })
        } else {
            None
        }
    }

    /// Return a reference to the specified (1-based) line.
    /// If the line is greater than the last line in the buffer, an empty
    /// reference is returned.
    pub fn get_line_ref(&self, line: u32) -> DataRef<u8> {
        unsafe { hermes_parser_get_line_ref(self.parser_ctx, line) }
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
        let buf = NullTerminatedBuf::from_str_check("var x = 10;\0");
        let p = HermesParser::parse(Default::default(), &buf);
        assert!(!p.has_errors());
        assert_eq!(p.root().unwrap().as_ref().kind, NodeKind::Program);
    }

    #[test]
    fn parse_error() {
        let buf = NullTerminatedBuf::from_str_check("var x+ = 10;");
        let p = HermesParser::parse(Default::default(), &buf);
        assert!(p.has_errors());
        assert!(p.root().is_none());

        let messages = p.messages();
        assert_eq!(messages.len(), 1);
        assert_eq!(
            p.find_coord(messages[0].loc).unwrap(),
            Coord { line: 1, offset: 5 }
        );
    }

    #[test]
    fn magic_comments() {
        let buf = NullTerminatedBuf::from_str_check(
            "var p = 0;
            //# sourceURL=1
            //# sourceMappingURL=my map URL
            //# sourceURL=my source URL
            ",
        );
        let p = HermesParser::parse(Default::default(), &buf);
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
