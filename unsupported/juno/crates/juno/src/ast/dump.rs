/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::{
    AssignmentExpressionOperator, BinaryExpressionOperator, Context, ExportKind, GCLock,
    ImportKind, LogicalExpressionOperator, MethodDefinitionKind, Node, NodeLabel, NodeList, NodeRc,
    NodeString, PropertyKind, UnaryExpressionOperator, UpdateExpressionOperator,
    VariableDeclarationKind,
};
use std::io::{self, Write};
use support::{case::ascii_snake_to_camel, json::*};

pub use support::json::Pretty;

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
        fn dump_node<'gc, W: Write>(
            ctx: &'gc GCLock,
            node: &'gc Node<'gc>,
            emitter: &mut JSONEmitter<W>,
        ) {
            use crate::ast::*;
            emitter.open_dict();
            emitter.emit_key("type");
            emitter.emit_string(node.name());
            match node {
                $(
                    Node::$kind($kind {$($($field,)*)? .. }) => {
                        $($(
                            emitter.emit_key(&ascii_snake_to_camel(stringify!($field)));
                            $field.dump(ctx, emitter);
                        )*)?
                    }
                ),*
            }
            emitter.close_dict();
        }
    }
}

nodekind_defs! { gen_dumper }

trait DumpChild<'gc> {
    fn dump<W: Write>(&self, ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>);
}

impl<'gc> DumpChild<'gc> for f64 {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_number(*self);
    }
}

impl<'gc> DumpChild<'gc> for bool {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_bool(*self);
    }
}

impl<'gc> DumpChild<'gc> for NodeLabel {
    fn dump<W: Write>(&self, ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(ctx.str(*self));
    }
}

impl<'gc> DumpChild<'gc> for UnaryExpressionOperator {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for BinaryExpressionOperator {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for LogicalExpressionOperator {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for UpdateExpressionOperator {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for AssignmentExpressionOperator {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for VariableDeclarationKind {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for PropertyKind {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for MethodDefinitionKind {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for ImportKind {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for ExportKind {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string(self.as_str());
    }
}

impl<'gc> DumpChild<'gc> for NodeString {
    fn dump<W: Write>(&self, _ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.emit_string_literal(&self.str);
    }
}

impl<'gc, T: DumpChild<'gc>> DumpChild<'gc> for Option<T> {
    fn dump<W: Write>(&self, ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        match self {
            None => emitter.emit_null(),
            Some(t) => t.dump(ctx, emitter),
        };
    }
}

impl<'gc> DumpChild<'gc> for &'gc Node<'gc> {
    fn dump<W: Write>(&self, ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        dump_node(ctx, self, emitter);
    }
}

impl<'gc> DumpChild<'gc> for NodeList<'gc> {
    fn dump<W: Write>(&self, ctx: &'gc GCLock, emitter: &mut JSONEmitter<W>) {
        emitter.open_array();
        for &elem in self {
            dump_node(ctx, elem, emitter);
        }
        emitter.close_array();
    }
}

pub fn dump_json<W: Write>(
    writer: W,
    ctx: &mut Context,
    root: &NodeRc,
    pretty: Pretty,
) -> io::Result<()> {
    let gc = GCLock::new(ctx);
    let mut emitter = JSONEmitter::new(writer, pretty);
    dump_node(&gc, root.node(&gc), &mut emitter);
    emitter.end()
}
