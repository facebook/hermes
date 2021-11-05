use crate::Pass;
use juno::ast::*;

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
        _parent: Option<&'gc Node<'gc>>,
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
                builder.implements(vec![]);
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
                builder.implements(vec![]);
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
            _ => {}
        }
        node.visit_children_mut(gc, self)
    }
}
