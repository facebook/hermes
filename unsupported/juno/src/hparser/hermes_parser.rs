/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::ffi::{c_void, CStr};
use std::os::raw::c_char;

use super::node::{Node, NodePtr};

extern "C" {
    /// Note: source[len-1] must be 0.
    fn hermes_parser_parse(source: *const c_char, len: usize) -> *mut c_void;
    fn hermes_parser_free(parsed: *mut c_void);
    fn hermes_parser_get_error(parsed: *const c_void) -> *const c_char;
    fn hermes_parser_get_ast(parsed: *const c_void) -> NodePtr;
    fn hermes_get_node_name(node: *const Node) -> *const c_char;
}

pub struct HermesParser {
    /// A pointer to the opaque C++ parser object. It should never be null.
    cparser: *mut c_void,
}

impl HermesParser {
    fn new(cparser: *mut c_void) -> HermesParser {
        assert!(!cparser.is_null(), "C++ parser instance must not be null");
        HermesParser { cparser }
    }
}

impl Drop for HermesParser {
    fn drop(&mut self) {
        unsafe { hermes_parser_free(self.cparser) }
    }
}

impl HermesParser {
    /// To acoid copying `source` can optionally be NUL-terminated.
    pub fn parse(source: &str) -> Result<HermesParser, String> {
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

        unsafe {
            let parser = HermesParser::new(hermes_parser_parse(
                source_z.as_ptr() as *const i8,
                source_z.len(),
            ));

            let err = hermes_parser_get_error(parser.cparser);
            if !err.is_null() {
                return Err(CStr::from_ptr(err).to_string_lossy().into_owned());
            }

            Ok(parser)
        }
    }

    pub fn root(&self) -> NodePtr {
        unsafe { hermes_parser_get_ast(self.cparser) }
    }

    pub fn node_name(n: &Node) -> &str {
        unsafe { CStr::from_ptr(hermes_get_node_name(n)).to_str().unwrap() }
    }
}
