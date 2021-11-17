/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::ast::{self, GCLock, Node, NodePtr, Path, VariableDeclarationKind, Visitor};
use crate::node_isa;
use std::collections::HashMap;
use std::rc::Rc;

/// List of all declarations in a scope.
pub type ScopeDecls<'gc> = Vec<&'gc Node<'gc>>;

/// Collect all declarations in every scope in a function since they all need
/// to be hoisted either to the top of the function or the scope.
#[derive(Debug)]
pub(super) struct DeclCollector<'gc> {
    /// Associate a [ScopeDecls] struct with nodes.
    /// We need Rc<> to enable the consumer to reference the data without
    /// keeping a reference to DeclCollector.
    scopes: HashMap<NodePtr<'gc>, Rc<ScopeDecls<'gc>>>,

    /// Function declarations in a block scope. They have special rules described
    /// in Annex B 3.3.
    scoped_func_decls: Vec<&'gc Node<'gc>>,

    /// Stack of active scopes. Once a scope is closed, it is attached to an AST node.
    scope_stack: Vec<ScopeDecls<'gc>>,
}

impl<'gc> DeclCollector<'gc> {
    /// Collect all declarations in a function.
    pub fn run(ctx: &'gc GCLock, root: &'gc Node<'gc>) -> DeclCollector<'gc> {
        let mut dc = DeclCollector {
            scopes: Default::default(),
            scope_stack: Vec::with_capacity(4),
            scoped_func_decls: Vec::new(),
        };
        dc.run_impl(ctx, root);
        // Free the scope stack.
        dc.scope_stack = Vec::new();
        dc
    }

    /// Return the optional ScopeDecls for an AST node.
    pub fn scope_decls_for_node(&self, node: &Node) -> Option<Rc<ScopeDecls<'gc>>> {
        self.scopes.get(&NodePtr::from(node)).cloned()
    }

    /// Return a list of all scoped function declarations in the function.
    /// Not used until we enable scoped function promotion.
    #[allow(dead_code)]
    pub fn scope_func_decls(&self) -> &Vec<&'gc Node<'gc>> {
        &self.scoped_func_decls
    }

    /// Add the declaration node to the current scope.
    fn add_to_cur(&mut self, node: &'gc Node<'gc>) {
        self.scope_stack.last_mut().unwrap().push(node);
    }
    /// Add the declaration node to the function scope.
    fn add_to_func(&mut self, node: &'gc Node<'gc>) {
        self.scope_stack[0].push(node);
    }

    /// Create a new scope and make it active.
    fn new_scope(&mut self) {
        self.scope_stack.push(Default::default());
    }

    /// Associate the active scope with the specified AST node, pop the previous
    /// scope from the stack and make it active.
    fn close_scope(&mut self, ptr: &'gc Node<'gc>) {
        let decls = self.scope_stack.pop().unwrap();
        if !decls.is_empty() {
            self.scopes.insert(NodePtr::from_node(ptr), Rc::new(decls));
        }
    }

    fn run_impl(&mut self, lock: &'gc GCLock, root: &'gc Node<'gc>) {
        match root {
            Node::FunctionDeclaration(ast::FunctionDeclaration { body, .. })
            | Node::FunctionExpression(ast::FunctionExpression { body, .. }) => {
                self.new_scope();
                // Visit the children of the body, since we don't want to associate a
                // scope with it.
                debug_assert!(node_isa!(Node::BlockStatement, body));
                body.visit_children(lock, self);
                self.close_scope(root);
            }
            Node::ArrowFunctionExpression(ast::ArrowFunctionExpression { body, .. }) => {
                self.new_scope();
                // If there is a BlockStatement, don't visit it, just visit its children.
                if matches!(*body, Node::BlockStatement { .. }) {
                    body.visit_children(lock, self);
                } else {
                    root.visit_children(lock, self);
                }
                self.close_scope(root);
            }
            _ => {
                self.new_scope();
                root.visit_children(lock, self);
                self.close_scope(root);
            }
        }
    }
}

impl<'gc> Visitor<'gc> for DeclCollector<'gc> {
    fn call(&mut self, ctx: &'gc GCLock, node: &'gc Node<'gc>, _parent: Option<Path<'gc>>) {
        match node {
            Node::VariableDeclaration(ast::VariableDeclaration {
                kind: VariableDeclarationKind::Var,
                ..
            }) => {
                self.add_to_func(node);
                node.visit_children(ctx, self);
            }

            Node::VariableDeclaration { .. }
            | Node::ClassDeclaration { .. }
            | Node::ImportDeclaration { .. } => {
                self.add_to_cur(node);
                node.visit_children(ctx, self);
            }

            // Record function declarations and don't descend into them.
            Node::FunctionDeclaration { .. } => {
                self.add_to_cur(node);
                // If this is not the function scope, record a scoped func decl.
                if self.scopes.len() > 1 {
                    self.scoped_func_decls.push(node);
                }
            }

            // Don't descend into function expressions.
            Node::FunctionExpression { .. } | Node::ArrowFunctionExpression { .. } => {}

            // Associate a scope with scoped statements.
            Node::BlockStatement { .. }
            | Node::ForStatement { .. }
            | Node::ForInStatement { .. }
            | Node::ForOfStatement { .. }
            | Node::SwitchStatement { .. } => {
                self.new_scope();
                node.visit_children(ctx, self);
                self.close_scope(node);
            }

            // Descend into children.
            _ => {
                node.visit_children(ctx, self);
            }
        }
    }
}
