/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Pass to strip the Flow type declarations from code.

use juno::ast::*;

use crate::Pass;

#[derive(Default)]
pub struct StripFlow {}

impl StripFlow {
    pub fn new() -> Self {
        Default::default()
    }
}

impl Pass for StripFlow {
    fn name(&self) -> &'static str {
        "Strip Flow"
    }
    fn description(&self) -> &'static str {
        "Strip Flow declarations"
    }

    fn run<'gc>(
        &mut self,
        gc: &'gc GCLock<'_, '_>,
        node: &'gc Node<'gc>,
    ) -> TransformResult<&'gc Node<'gc>> {
        VisitorMut::call(self, gc, node, None)
    }
}

impl<'gc> VisitorMut<'gc> for StripFlow {
    fn call(
        &mut self,
        gc: &'gc GCLock<'_, '_>,
        node: &'gc Node<'gc>,
        _parent: Option<Path<'gc>>,
    ) -> TransformResult<&'gc Node<'gc>> {
        match node {
            Node::TypeCastExpression(TypeCastExpression { expression, .. }) => {
                return node.replace_with_existing(expression, gc, self);
            }

            Node::Identifier(id) => {
                if id.type_annotation.is_some() || id.optional {
                    let mut builder = builder::Identifier::from_node(id);
                    builder.type_annotation(None);
                    builder.optional(false);
                    return node.replace_with_new(builder::Builder::Identifier(builder), gc, self);
                }
            }

            Node::ImportDeclaration(decl) => {
                if decl.import_kind != ImportKind::Value {
                    return TransformResult::Removed;
                }
            }
            Node::ImportSpecifier(spec) => {
                if spec.import_kind != ImportKind::Value {
                    return TransformResult::Removed;
                }
            }
            Node::ExportNamedDeclaration(spec) => {
                if spec.export_kind != ExportKind::Value {
                    return TransformResult::Removed;
                }
            }
            Node::InterfaceDeclaration { .. }
            | Node::TypeAlias { .. }
            | Node::OpaqueType { .. }
            | Node::DeclareTypeAlias { .. }
            | Node::DeclareOpaqueType { .. }
            | Node::DeclareInterface { .. }
            | Node::DeclareClass { .. }
            | Node::DeclareFunction { .. }
            | Node::DeclareVariable { .. }
            | Node::DeclareExportDeclaration { .. }
            | Node::DeclareExportAllDeclaration { .. }
            | Node::DeclareModule { .. }
            | Node::DeclareModuleExports { .. } => return TransformResult::Removed,

            Node::FunctionDeclaration(n) => {
                let mut builder = builder::FunctionDeclaration::from_node(n);
                builder.params(NodeList::from_iter(
                    gc,
                    n.params.iter().filter(|p| match p {
                        Node::Identifier(Identifier { name, .. }) => gc.str(*name) != "this",
                        _ => true,
                    }),
                ));
                builder.type_parameters(None);
                builder.return_type(None);
                builder.predicate(None);
                return node.replace_with_new(
                    builder::Builder::FunctionDeclaration(builder),
                    gc,
                    self,
                );
            }
            Node::FunctionExpression(n) => {
                let mut builder = builder::FunctionExpression::from_node(n);
                builder.params(NodeList::from_iter(
                    gc,
                    n.params.iter().filter(|p| match p {
                        Node::Identifier(Identifier { name, .. }) => gc.str(*name) != "this",
                        _ => true,
                    }),
                ));
                builder.type_parameters(None);
                builder.return_type(None);
                builder.predicate(None);
                return node.replace_with_new(
                    builder::Builder::FunctionExpression(builder),
                    gc,
                    self,
                );
            }
            Node::ArrowFunctionExpression(n) => {
                let mut builder = builder::ArrowFunctionExpression::from_node(n);
                builder.type_parameters(None);
                builder.return_type(None);
                builder.predicate(None);
                return node.replace_with_new(
                    builder::Builder::ArrowFunctionExpression(builder),
                    gc,
                    self,
                );
            }

            Node::ClassDeclaration(n) => {
                let mut builder = builder::ClassDeclaration::from_node(n);
                builder.implements(NodeList::new(gc));
                builder.super_type_parameters(None);
                builder.type_parameters(None);
                return node.replace_with_new(
                    builder::Builder::ClassDeclaration(builder),
                    gc,
                    self,
                );
            }
            Node::ClassExpression(n) => {
                let mut builder = builder::ClassExpression::from_node(n);
                builder.implements(NodeList::new(gc));
                builder.super_type_parameters(None);
                builder.type_parameters(None);
                return node.replace_with_new(builder::Builder::ClassExpression(builder), gc, self);
            }

            Node::ClassProperty(n) => {
                if n.value.is_none() {
                    return TransformResult::Removed;
                }

                let mut builder = builder::ClassProperty::from_node(n);
                builder.type_annotation(None);
                builder.variance(None);
                return node.replace_with_new(builder::Builder::ClassProperty(builder), gc, self);
            }
            Node::ClassPrivateProperty(n) => {
                if n.value.is_none() {
                    return TransformResult::Removed;
                }

                let mut builder = builder::ClassPrivateProperty::from_node(n);
                builder.type_annotation(None);
                builder.variance(None);
                return node.replace_with_new(
                    builder::Builder::ClassPrivateProperty(builder),
                    gc,
                    self,
                );
            }

            Node::CallExpression(n) => {
                let mut builder = builder::CallExpression::from_node(n);
                builder.type_arguments(None);
                return node.replace_with_new(builder::Builder::CallExpression(builder), gc, self);
            }
            Node::NewExpression(n) => {
                let mut builder = builder::NewExpression::from_node(n);
                builder.type_arguments(None);
                return node.replace_with_new(builder::Builder::NewExpression(builder), gc, self);
            }

            Node::EnumDeclaration(n) => {
                return node.replace_with_existing(transform_enum(gc, n), gc, self);
            }
            Node::ExportDefaultDeclaration(ExportDefaultDeclaration {
                metadata,
                declaration: Node::EnumDeclaration(e),
            }) => {
                return node.replace_with_multiple(
                    vec![
                        builder::Builder::from_node(transform_enum(gc, e)),
                        builder::Builder::ExportDefaultDeclaration(
                            builder::ExportDefaultDeclaration::from_template(
                                template::ExportDefaultDeclaration {
                                    metadata: TemplateMetadata {
                                        range: metadata.range,
                                        ..Default::default()
                                    },
                                    declaration: e.id,
                                },
                            ),
                        ),
                    ],
                    gc,
                    self,
                );
            }
            _ => {}
        }
        node.visit_children_mut(gc, self)
    }
}

fn transform_enum<'gc>(gc: &'gc GCLock<'_, '_>, n: &'gc EnumDeclaration<'gc>) -> &'gc Node<'gc> {
    let (method, args) = match n.body {
        Node::EnumStringBody(body)
            if matches!(body.members.head(), Some(Node::EnumDefaultedMember(_))) =>
        {
            let elements = body.members.iter().map(|m| match m {
                Node::EnumDefaultedMember(m) => builder::StringLiteral::build_template(
                    gc,
                    template::StringLiteral {
                        metadata: Default::default(),
                        value: gc.atom_u16(match m.id {
                            Node::Identifier(id) => {
                                gc.str(id.name).encode_utf16().collect::<Vec<u16>>()
                            }
                            _ => unreachable!(),
                        }),
                    },
                ),
                _ => unreachable!(),
            });

            (
                Some("Mirrored"),
                NodeList::from_iter(
                    gc,
                    [builder::ArrayExpression::build_template(
                        gc,
                        template::ArrayExpression {
                            metadata: Default::default(),
                            trailing_comma: false,
                            elements: NodeList::from_iter(gc, elements),
                        },
                    )],
                ),
            )
        }
        Node::EnumSymbolBody(EnumSymbolBody { members, .. })
        | Node::EnumStringBody(EnumStringBody { members, .. })
        | Node::EnumBooleanBody(EnumBooleanBody { members, .. })
        | Node::EnumNumberBody(EnumNumberBody { members, .. }) => (
            None,
            NodeList::from_iter(
                gc,
                [builder::ObjectExpression::build_template(
                    gc,
                    template::ObjectExpression {
                        metadata: Default::default(),
                        properties: NodeList::from_iter(
                            gc,
                            members.iter().map(|m| match m {
                                Node::EnumStringMember(EnumStringMember { metadata, id, init })
                                | Node::EnumNumberMember(EnumNumberMember { metadata, id, init })
                                | Node::EnumBooleanMember(EnumBooleanMember {
                                    metadata,
                                    id,
                                    init,
                                }) => builder::Property::build_template(
                                    gc,
                                    template::Property {
                                        metadata: TemplateMetadata {
                                            range: metadata.range,
                                            ..Default::default()
                                        },
                                        kind: PropertyKind::Init,
                                        computed: false,
                                        method: false,
                                        shorthand: false,
                                        key: id,
                                        value: init,
                                    },
                                ),

                                // Has to be contained in a EnumSymbolBody
                                Node::EnumDefaultedMember(m) => builder::Property::build_template(
                                    gc,
                                    template::Property {
                                        metadata: TemplateMetadata {
                                            range: m.metadata.range,
                                            ..Default::default()
                                        },
                                        kind: PropertyKind::Init,
                                        computed: false,
                                        method: false,
                                        shorthand: false,
                                        key: m.id,
                                        value: builder::CallExpression::build_template(
                                            gc,
                                            template::CallExpression {
                                                metadata: Default::default(),
                                                type_arguments: None,
                                                callee: builder::Identifier::build_template(
                                                    gc,
                                                    template::Identifier {
                                                        metadata: Default::default(),
                                                        name: gc.atom("Symbol"),
                                                        optional: false,
                                                        type_annotation: None,
                                                    },
                                                ),
                                                arguments: NodeList::from_iter(
                                                    gc,
                                                    [builder::StringLiteral::build_template(
                                                        gc,
                                                        template::StringLiteral {
                                                            metadata: Default::default(),
                                                            value: gc.atom_u16(match m.id {
                                                                Node::Identifier(id) => gc
                                                                    .str(id.name)
                                                                    .encode_utf16()
                                                                    .collect::<Vec<u16>>(),
                                                                _ => unreachable!(),
                                                            }),
                                                        },
                                                    )],
                                                ),
                                            },
                                        ),
                                    },
                                ),
                                _ => unreachable!(),
                            }),
                        ),
                    },
                )],
            ),
        ),
        _ => unreachable!(),
    };
    let runtime = builder::CallExpression::build_template(
        gc,
        template::CallExpression {
            metadata: Default::default(),
            type_arguments: None,
            callee: builder::Identifier::build_template(
                gc,
                template::Identifier {
                    metadata: Default::default(),
                    name: gc.atom("require"),
                    optional: false,
                    type_annotation: None,
                },
            ),
            arguments: NodeList::from_iter(
                gc,
                [builder::StringLiteral::build_template(
                    gc,
                    template::StringLiteral {
                        metadata: Default::default(),
                        value: gc
                            .atom_u16("flow-enums-runtime".encode_utf16().collect::<Vec<u16>>()),
                    },
                )],
            ),
        },
    );
    return builder::VariableDeclaration::build_template(
        gc,
        template::VariableDeclaration {
            metadata: TemplateMetadata {
                range: n.metadata.range,
                ..Default::default()
            },
            kind: VariableDeclarationKind::Const,
            declarations: NodeList::from_iter(
                gc,
                [builder::VariableDeclarator::build_template(
                    gc,
                    template::VariableDeclarator {
                        metadata: Default::default(),
                        id: n.id,
                        init: Some(builder::CallExpression::build_template(
                            gc,
                            template::CallExpression {
                                metadata: Default::default(),
                                type_arguments: None,
                                callee: match method {
                                    Some(m) => builder::MemberExpression::build_template(
                                        gc,
                                        template::MemberExpression {
                                            metadata: Default::default(),
                                            computed: false,
                                            object: runtime,
                                            property: builder::Identifier::build_template(
                                                gc,
                                                template::Identifier {
                                                    metadata: Default::default(),
                                                    name: gc.atom(m),
                                                    optional: false,
                                                    type_annotation: None,
                                                },
                                            ),
                                        },
                                    ),
                                    None => runtime,
                                },
                                arguments: args,
                            },
                        )),
                    },
                )],
            ),
        },
    );
}
