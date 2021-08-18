/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::{Node, NodeKind, NodeLabel, NodeList, NodePtr, StringLiteral};
use crate::support::{case::ascii_snake_to_camel, json::*};
use std::io::{self, Write};

pub use crate::support::json::Pretty;

/// Generate boilerplate code for the `NodeKind` enum.
macro_rules! gen_dumper {
    ($name:ident {
        $(
            $kind:ident $([ $parent:ident ])? $({
                $(
                    $field:ident : $type:ty
                    $( [ $( $constraint:ident ),* ] )?
                ),*
                $(,)?
            })?
        ),*
        $(,)?
    }) => {
        fn dump_node<W: Write>(node: &Node, emitter: &mut JSONEmitter<W>) {
            emitter.open_dict();
            emitter.emit_key("type");
            emitter.emit_string(node.kind.name());
            match &node.kind {
                $(
                    NodeKind::$kind $({$($field),*})? => {
                        $($(
                            emitter.emit_key(&ascii_snake_to_camel(stringify!($field)));
                            $field.dump(emitter);
                        )*)?
                    }
                ),*
            }
            emitter.close_dict();
        }
    }
}

nodekind_defs! { gen_dumper }

trait DumpChild {
    fn dump<W: Write>(&self, emitter: &mut JSONEmitter<W>);
}

impl DumpChild for f64 {
    fn dump<W: Write>(&self, emitter: &mut JSONEmitter<W>) {
        emitter.emit_number(*self);
    }
}

impl DumpChild for bool {
    fn dump<W: Write>(&self, emitter: &mut JSONEmitter<W>) {
        emitter.emit_bool(*self);
    }
}

impl DumpChild for NodeLabel {
    fn dump<W: Write>(&self, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(&self.str);
    }
}

impl DumpChild for StringLiteral {
    fn dump<W: Write>(&self, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string_literal(&self.str);
    }
}

impl<T: DumpChild> DumpChild for Option<T> {
    fn dump<W: Write>(&self, emitter: &mut JSONEmitter<W>) {
        match self {
            None => emitter.emit_null(),
            Some(t) => t.dump(emitter),
        };
    }
}

impl DumpChild for NodePtr {
    fn dump<W: Write>(&self, emitter: &mut JSONEmitter<W>) {
        dump_node(self, emitter);
    }
}

impl DumpChild for NodeList {
    fn dump<W: Write>(&self, emitter: &mut JSONEmitter<W>) {
        emitter.open_array();
        for elem in self {
            dump_node(elem, emitter);
        }
        emitter.close_array();
    }
}

pub fn dump_json<W: Write>(writer: W, root: &Node, pretty: Pretty) -> io::Result<()> {
    let mut emitter = JSONEmitter::new(writer, pretty);
    dump_node(root, &mut emitter);
    emitter.end()
}
