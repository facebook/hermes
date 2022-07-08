/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::fmt;
use std::io;
use std::io::BufWriter;
use std::io::Write;

/// Whether to pretty-print the JSON.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum Pretty {
    No,
    Yes,
}

/// The state indicates the next expected emit call in the current collection.
#[derive(Debug)]
enum State {
    /// Expecting a dict key.
    DictKey {
        /// The dict is currently empty.
        is_empty: bool,
    },

    /// Expecting a dict value.
    DictValue,

    /// Expecting a value in an array.
    ArrayValue {
        /// The array is currently empty.
        is_empty: bool,
    },
}

/// Allows the caller to output JSON to any Write type.
/// The caller is responsible for ensuring calls to `open` and `close`
/// functions are valid, and that keys and values are emitted in the proper order.
/// If any emitter functions are called out of order, panics.
pub struct JSONEmitter<W: Write> {
    /// Where to write the generated JSON.
    out: BufWriter<W>,

    /// Whether to pretty print the output JSON.
    pretty: Pretty,

    /// Stack used to maintain state.
    states: Vec<State>,

    /// Current indentation level, used in pretty mode.
    indent: usize,

    /// Size of the indentation step.
    /// May be configurable in the future.
    indent_step: usize,

    /// Some(err) if an error has occurred when writing, else None.
    error: Option<io::Error>,
}

/// Print to the output stream if no errors have been seen so far.
/// `$gen_js` is a mutable reference to the GenJS struct.
/// `$arg` arguments follow the format pattern used by `format!`.
macro_rules! out {
    ($emitter:expr, $($arg:tt)*) => {{
        $emitter.write_if_no_errors(format_args!($($arg)*));
    }}
}

impl<W: Write> JSONEmitter<W> {
    /// Create a new `JSONEmitter` to write to `out`.
    pub fn new(out: W, pretty: Pretty) -> JSONEmitter<W> {
        JSONEmitter {
            out: BufWriter::new(out),
            pretty,
            indent: 0,
            indent_step: 2,
            states: Vec::new(),
            error: None,
        }
    }

    /// Write to the `out` writer if we haven't seen any errors.
    /// If we have seen any errors, do nothing.
    /// Used via the `out!` macro.
    fn write_if_no_errors(&mut self, args: fmt::Arguments<'_>) {
        if self.error.is_none() {
            if let Err(e) = self.out.write_fmt(args) {
                self.error = Some(e);
            }
        }
    }

    /// Emit a finite f64.
    /// Panics if `val` is infinite or NaN.
    pub fn emit_number(&mut self, val: f64) {
        assert!(val.is_finite(), "{} is not finite", val);
        self.will_emit_value();
        out!(self, "{}", crate::convert::number_to_string(val));
    }

    /// Emit a bool.
    pub fn emit_bool(&mut self, val: bool) {
        self.will_emit_value();
        out!(self, "{}", if val { "true" } else { "false" });
    }

    /// Emit the null value.
    pub fn emit_null(&mut self) {
        self.will_emit_value();
        out!(self, "null");
    }

    /// Emit a key in a dict.
    /// Panics if not currently in key position in a dict.
    pub fn emit_key(&mut self, key: &str) {
        let state: &mut State = match self.states.last_mut() {
            None => {
                return;
            }
            Some(state) => state,
        };
        let need_comma = match state {
            State::DictKey { is_empty } => !*is_empty,
            _ => panic!("Not expecting dict key"),
        };
        *state = State::DictValue;
        if need_comma {
            out!(self, ",");
        }
        self.newline();
        self.primitive_emit_string(key);
        out!(self, ":");
        if self.pretty == Pretty::Yes {
            out!(self, " ");
        }
    }

    /// Open a new dict.
    /// Panics if a new dict cannot be opened in this position.
    pub fn open_dict(&mut self) {
        self.will_emit_value();
        out!(self, "{{");
        self.inc_indent();
        self.states.push(State::DictKey { is_empty: true });
    }

    /// Close a dict.
    /// Panics if there is no dict to close or if a value is expected.
    pub fn close_dict(&mut self) {
        match self.states.pop() {
            Some(State::DictKey { is_empty }) => {
                self.dec_indent();
                if !is_empty {
                    self.newline();
                }
                out!(self, "}}");
            }
            _ => {
                panic!("Not in dict");
            }
        }
    }

    /// Open a new array.
    /// Panics if a new array cannot be opened in this position.
    pub fn open_array(&mut self) {
        self.will_emit_value();
        out!(self, "[");
        self.inc_indent();
        self.states.push(State::ArrayValue { is_empty: true });
    }

    /// Close an array.
    /// Panics if there is no array to close.
    pub fn close_array(&mut self) {
        match self.states.pop() {
            Some(State::ArrayValue { is_empty }) => {
                self.dec_indent();
                if !is_empty {
                    self.newline();
                }
                out!(self, "]");
            }
            _ => {
                panic!("Not in array");
            }
        }
    }

    /// Emit a valid UTF-8 string.
    pub fn emit_string(&mut self, string: &str) {
        self.will_emit_value();
        self.primitive_emit_string(string);
    }

    /// Emit a valid UTF-8 string.
    pub fn primitive_emit_string(&mut self, string: &str) {
        out!(self, "\"{}\"", string);
    }

    /// Consume the emitter and finish emitting.
    /// Panics if there are outstanding dicts/arrays to close.
    pub fn end(mut self) -> io::Result<()> {
        assert!(self.states.is_empty(), "Unterminated object");
        out!(self, "\n");
        match self.error {
            None => self.out.flush(),
            Some(err) => Err(err),
        }
    }

    /// Emit an escaped JS string literal.
    pub fn emit_string_literal(&mut self, value: &[u16]) {
        self.will_emit_value();
        out!(self, "\"");
        let esc = '\\';
        for &c in value {
            if c <= 0xff {
                match c as u8 as char {
                    '\\' => {
                        out!(self, "\\\\");
                        continue;
                    }
                    '\x08' => {
                        out!(self, "\\b");
                        continue;
                    }
                    '\x0c' => {
                        out!(self, "\\f");
                        continue;
                    }
                    '\n' => {
                        out!(self, "\\n");
                        continue;
                    }
                    '\r' => {
                        out!(self, "\\r");
                        continue;
                    }
                    '\t' => {
                        out!(self, "\\t");
                        continue;
                    }
                    '\x0b' => {
                        out!(self, "\\v");
                        continue;
                    }
                    _ => {}
                };
            }
            if c == esc as u16 {
                out!(self, "\\");
            }
            if (0x20..=0x7f).contains(&c) {
                // Printable.
                out!(self, "{}", c as u8 as char);
            } else {
                out!(self, "\\u{:04x}", c);
            }
        }
        out!(self, "\"");
    }

    /// Signal that we're about to emit a dict or array value and handle
    /// any necessary `,` and newlines.
    fn will_emit_value(&mut self) {
        // Need to use a local var here because we can't use `@` directly
        // in the `match` (unstable language feature).
        let state: &mut State = match self.states.last_mut() {
            None => {
                return;
            }
            Some(state) => state,
        };
        match state {
            State::DictValue => {
                *state = State::DictKey { is_empty: false };
            }
            State::ArrayValue { is_empty } => {
                let need_comma = !*is_empty;
                *state = State::ArrayValue { is_empty: false };
                if need_comma {
                    out!(self, ",");
                }
                self.newline();
            }
            _ => panic!("Expected a key"),
        };
    }

    fn newline(&mut self) {
        if self.pretty == Pretty::Yes {
            let indent = self.indent;
            out!(self, "\n{:indent$}", "", indent = indent);
        }
    }

    /// Increase the indent level.
    fn inc_indent(&mut self) {
        self.indent += self.indent_step;
    }

    /// Decrease the indent level.
    fn dec_indent(&mut self) {
        self.indent -= self.indent_step;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_values() {
        let mut out = vec![];
        let mut emitter = JSONEmitter::new(&mut out, Pretty::No);
        emitter.emit_null();
        emitter.end().unwrap();
        assert_eq!(String::from_utf8(out).unwrap(), "null\n");
    }

    #[test]
    fn test_string() {
        let mut out = vec![];
        let mut emitter = JSONEmitter::new(&mut out, Pretty::No);
        emitter.emit_string_literal(&[0xd00a, 0x0a]);
        emitter.end().unwrap();
        assert_eq!(String::from_utf8(out).unwrap(), "\"\\ud00a\\n\"\n");
    }

    #[test]
    fn test_dict() {
        let mut out = vec![];
        let mut emitter = JSONEmitter::new(&mut out, Pretty::No);
        emitter.open_dict();
        emitter.emit_key("foo");
        emitter.emit_string("bar");
        emitter.emit_key("baz");
        emitter.emit_string("quz");
        emitter.close_dict();
        emitter.end().unwrap();
        assert_eq!(
            String::from_utf8(out).unwrap(),
            "{\"foo\":\"bar\",\"baz\":\"quz\"}\n"
        );
    }
}
