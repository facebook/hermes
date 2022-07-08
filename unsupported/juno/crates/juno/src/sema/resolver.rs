/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::sem_context::*;
use crate::ast;
use crate::ast::builder;
use crate::ast::node_cast;
use crate::ast::node_isa;
use crate::ast::template;
use crate::ast::GCLock;
use crate::ast::Identifier;
use crate::ast::Node;
use crate::ast::NodeField;
use crate::ast::NodeList;
use crate::ast::NodePtr;
use crate::ast::NodeRc;
use crate::ast::NodeVariant;
use crate::ast::Path;
use crate::ast::TemplateMetadata;
use crate::ast::UnaryExpressionOperator;
use crate::ast::VariableDeclarationKind;
use crate::ast::Visitor;
use crate::resolve_dependency::DependencyKind;
use crate::resolve_dependency::DependencyResolver;
use crate::sema::decl_collector::DeclCollector;
use crate::sema::decl_collector::ScopeDecls;
use crate::sema::keywords::Keywords;
use crate::sema::known_globals::KNOWN_GLOBALS;
use juno_support::atom_table::Atom;
use juno_support::source_manager::SourceId;
use juno_support::source_manager::SourceRange;
use juno_support::ScopedHashMap;
use smallvec::SmallVec;
use std::collections::HashMap;
use std::collections::HashSet;
use std::rc::Rc;

#[derive(Debug)]
struct Binding<'gc> {
    decl: DeclId,
    /// The declaring node.
    ident: &'gc Identifier<'gc>,
}

#[derive(Debug)]
#[allow(dead_code)]
struct Label<'gc> {
    id: LabelId,
    /// The declaring node.
    label: &'gc Identifier<'gc>,
    /// Statement targeted by the label.
    /// Either a loop or LabeledStatement.
    target_statement: &'gc Node<'gc>,
}

struct FunctionContext<'gc> {
    /// Ast node declaring the function. In the global function this would
    /// be [`Node::Program`] (which is not "function-like").
    node: &'gc Node<'gc>,
    /// Associated semantic data structure.
    func_id: FunctionInfoId,
    decls: DeclCollector<'gc>,
    /// Map of the names of labels to the `Label` object.
    label_table: HashMap<Atom, Label<'gc>>,
}

/// What type of file the resolver is resolving.
#[derive(Clone)]
enum ResolverMode<'a> {
    Script,
    Module {
        /// How to resolve the dependency.
        dependency_resolver: &'a dyn DependencyResolver,
    },
}

struct Resolver<'gc, 'mode> {
    kw: Keywords,
    sem: SemContext,
    func_stack: Vec<FunctionContext<'gc>>,
    current_scope: Option<LexicalScopeId>,
    /// The most nested active loop statement.
    current_loop: Option<LabelId>,
    /// The most nested active loop or switch statement.
    current_loop_or_switch: Option<LabelId>,
    binding_table: ScopedHashMap<Atom, Binding<'gc>>,
    /// True for a short time we are validating a formal parameter list.
    validating_formal_params: bool,
    /// The depth of the global scope in ['binding_table'].
    /// It is None until we have actually entered the global scope.
    global_binding_scope_depth: Option<usize>,
    file_id: SourceId,
    mode: ResolverMode<'mode>,
}

impl FunctionContext<'_> {
    /// Return the optional function name as a string.
    fn name_str<'a>(&self, lock: &'a GCLock) -> Option<&'a str> {
        if let Node::Program(_) = self.node {
            Some("global")
        } else if let Some(Node::Identifier(Identifier { name, .. })) = self.node.function_like_id()
        {
            Some(lock.str(*name))
        } else {
            None
        }
    }
}

/// Perform semantic validation of the AST and return semantic resolution information.
pub fn resolve_program<'gc>(
    lock: &'gc GCLock,
    file_id: SourceId,
    root: &'gc Node<'gc>,
) -> SemContext {
    let mut r = Resolver::new(lock, file_id, ResolverMode::Script);
    r.program(lock, root);
    r.sem
}

/// Perform semantic validation of the AST and return semantic resolution information.
pub fn resolve_module<'gc>(
    lock: &'gc GCLock,
    module: &'gc Node<'gc>,
    file_id: SourceId,
    dependency_resolver: &'_ dyn DependencyResolver,
) -> SemContext {
    let mut r = Resolver::new(
        lock,
        file_id,
        ResolverMode::Module {
            dependency_resolver,
        },
    );
    r.module(lock, module);
    r.sem
}

// Public functions
impl<'gc, 'mode> Resolver<'gc, 'mode> {
    fn new(lock: &'gc GCLock, file_id: SourceId, mode: ResolverMode<'mode>) -> Self {
        let kw = Keywords::new(lock.ctx().atom_table());
        Resolver {
            kw,
            sem: Default::default(),
            func_stack: Default::default(),
            current_scope: Default::default(),
            current_loop: None,
            current_loop_or_switch: None,
            binding_table: Default::default(),
            validating_formal_params: false,
            global_binding_scope_depth: None,
            file_id,
            mode,
        }
    }

    fn program(&mut self, ctx: &'gc GCLock, node: &'gc Node<'gc>) {
        assert!(node_isa!(Node::Program, node));
        self.visit_program(ctx, node);
    }

    fn module(&mut self, ctx: &'gc GCLock, module: &'gc Node<'gc>) {
        self.visit_module(ctx, module);
    }
}

// Utility functions
impl<'gc> Resolver<'gc, '_> {
    /// Return a reference to the current function context.
    fn function_context(&self) -> &FunctionContext<'gc> {
        self.func_stack.last().unwrap()
    }
    /// Return a reference to the current function context.
    fn function_context_mut(&mut self) -> &mut FunctionContext<'gc> {
        self.func_stack.last_mut().unwrap()
    }
    /// Return true if the current function is strict.
    fn function_strict_mode(&self) -> bool {
        self.sem.function(self.function_context().func_id).strict
    }
    /// Return a mutable reference to the current strict mode flag.
    fn function_strict_mode_mut(&mut self) -> &mut bool {
        &mut self
            .sem
            .function_mut(self.function_context().func_id)
            .strict
    }

    // Convenience function returning the scope object of a declaration.
    fn decl_scope(&self, decl: DeclId) -> &LexicalScope {
        self.sem.scope(self.sem.decl(decl).scope)
    }
    // Convenience function returning whether the specified declaration is in
    // the current function.
    fn decl_in_cur_function(&self, decl: DeclId) -> bool {
        self.decl_scope(decl).parent_function == self.function_context().func_id
    }

    /// Return `true` if the `callee` is the `require` function.
    fn is_require(&mut self, lock: &'gc GCLock, node: &'gc ast::CallExpression<'gc>) -> bool {
        debug_assert!(
            matches!(self.mode, ResolverMode::Module { .. }),
            "is_require must only be called in module mode"
        );
        if let Node::Identifier(ident @ ast::Identifier { name, .. }) = node.callee {
            if *name == self.kw.ident_require && node.arguments.len() == 1 {
                // The identifier must not have a binding.
                // It has been resolved as an ambient global property.
                if let Some(id) = self.check_identifier_resolved(lock, ident, node.callee) {
                    return self.sem.decl(id).kind == DeclKind::UndeclaredGlobalProperty;
                }
            }
        }
        false
    }

    /// Create a new function, push a new function context, execute the callback
    /// and pop the function context.
    fn in_new_function<R, F: FnOnce(&mut Self) -> R>(
        &mut self,
        lock: &'gc GCLock,
        root: &'gc Node<'gc>,
        f: F,
    ) -> R {
        // Determine the parent function and inherit its strictness.
        let (parent_func_id, strict) = if self.func_stack.is_empty() {
            (None, lock.ctx().strict_mode())
        } else {
            let fid = self.function_context().func_id;
            (Some(fid), self.sem.function(fid).strict)
        };

        // Create the function.
        let (func_id, _) = self.sem.new_function(
            parent_func_id,
            self.current_scope,
            strict,
            matches!(root, Node::ArrowFunctionExpression(_)),
        );

        // Create the function context.
        self.func_stack.push(FunctionContext {
            node: root,
            func_id,
            decls: DeclCollector::run(lock, root),
            label_table: Default::default(),
        });

        // Save and reset validating_formal_params.
        let save_vfp = self.validating_formal_params;
        self.validating_formal_params = false;

        let res = f(self);

        self.validating_formal_params = save_vfp;
        self.func_stack.pop();

        res
    }

    /// Push a new binding scope, allocate a new lexical scope, run the callback,
    /// then restore the lexical scope and the binding scope.
    ///
    /// `scope_node` - the AST node to associate the scope with.
    fn in_new_scope<R, F: FnOnce(&mut Self) -> R>(
        &mut self,
        lock: &'gc GCLock,
        scope_node: &'gc Node<'gc>,
        f: F,
    ) -> R {
        // New binding table scope.
        self.binding_table.push_scope();
        // New lexical scope.
        let prev_scope = self.current_scope;
        let (new_scope, _) = self.sem.new_scope(
            self.function_context().func_id,
            prev_scope,
            prev_scope.map_or(0, |p| self.sem.scope(p).depth + 1),
        );
        self.current_scope = Some(new_scope);
        // Record the global scope depth.
        if prev_scope.is_none() {
            self.global_binding_scope_depth = Some(self.binding_table.current_scope_depth());
        }
        // Associate the new lexical scope with the node.
        self.sem
            .set_node_scope(NodeRc::from_node(lock, scope_node), new_scope);

        let res = f(self);

        // Restore the global scope depth.
        if prev_scope.is_none() {
            self.global_binding_scope_depth = None;
        }
        // Restore the current scope.
        self.current_scope = prev_scope;
        self.binding_table.pop_scope();

        res
    }

    /// Allocate a new label in this scope, updating internal state, and call `f`.
    /// Restore the original label and return the result of `f`.
    /// Reports an error if the label is already defined.
    fn with_new_label<R, F: FnOnce(&mut Self) -> R>(
        &mut self,
        lock: &'gc GCLock,
        identifier: Option<&'gc Node<'gc>>,
        statement: &'gc Node<'gc>,
        f: F,
    ) -> R {
        // New lexical scope.
        let prev_loop = self.current_loop;
        let prev_loop_or_switch = self.current_loop_or_switch;
        let new_label = self
            .sem
            .function_mut(self.function_context().func_id)
            .new_label();
        match statement {
            Node::LabeledStatement(_) => {}
            Node::SwitchStatement(_) => {
                self.current_loop_or_switch = Some(new_label);
            }
            Node::ForStatement(_)
            | Node::ForInStatement(_)
            | Node::ForOfStatement(_)
            | Node::WhileStatement(_)
            | Node::DoWhileStatement(_) => {
                // Must be a loop.
                self.current_loop = Some(new_label);
                self.current_loop_or_switch = Some(new_label);
            }
            _ => {
                unreachable!("Invalid label statement: {:?}", statement.variant());
            }
        };
        let mut inserted = None;
        if let Some(identifier) = identifier {
            use std::collections::hash_map::Entry;
            let key = node_cast!(Node::Identifier, identifier).name;
            match self.function_context_mut().label_table.entry(key) {
                Entry::Occupied(_) => {
                    lock.sm().error(
                        *identifier.range(),
                        format!("label {} is already defined", identifier.name()),
                    );
                }
                Entry::Vacant(entry) => {
                    // Determine the target statement. We need to check if it directly encloses
                    // a loop or another label enclosing a loop.
                    let mut target_statement = statement;
                    {
                        let mut cur_statement = statement;
                        while let Node::LabeledStatement(ast::LabeledStatement { body, .. }) =
                            cur_statement
                        {
                            if body.is_loop_statement() {
                                target_statement = body;
                                break;
                            }
                            cur_statement = body;
                        }
                    }
                    debug_assert!(
                        target_statement.is_loop_statement()
                            || matches!(target_statement, Node::LabeledStatement(_)),
                        "invalid target statement detected for loop: {:?}",
                        target_statement
                    );
                    entry.insert(Label {
                        id: new_label,
                        label: node_cast!(Node::Identifier, identifier),
                        target_statement,
                    });
                    inserted = Some(key);
                }
            };
        }

        let res = f(self);

        // Restore the current scope.
        self.current_loop_or_switch = prev_loop_or_switch;
        self.current_loop = prev_loop;
        if let Some(key) = &inserted {
            self.function_context_mut().label_table.remove(key);
        }

        res
    }

    #[allow(dead_code)]
    pub fn dump(&self, ctx: &ast::Context) {
        ctx.atom_table().in_debug_context(|| {
            println!("{:#?}", self.binding_table);
        });
    }
}

// Business logic.
impl<'gc> Resolver<'gc, '_> {
    fn visit_program(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>) {
        self.in_new_function(lock, node, |pself| {
            // Search for "use strict".
            if find_use_strict(lock, &node_cast!(Node::Program, node).body).is_some() {
                pself
                    .sem
                    .function_mut(pself.function_context().func_id)
                    .strict = true;
            }

            // Create the global scope.
            pself.in_new_scope(lock, node, |pself| {
                // If we are warning on undefined symbols, pre-define the known
                // globals to decrease the number of warnings.
                if lock.ctx().warn_undefined {
                    pself.declare_known_globals(lock, *node.range());
                }

                pself.process_collected_declarations(lock, node);
                node.visit_children(lock, pself);
            });
        });
    }

    fn visit_module(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>) {
        // Create a fake global function in order to have a scope in which to declare
        // the known globals.
        // TODO: Share the global AST node and scope between calls to `visit_module` to avoid
        // making a new node for every module.
        let global = builder::FunctionDeclaration::build_template(
            lock,
            template::FunctionDeclaration {
                metadata: Default::default(),
                id: None,
                params: NodeList::new(lock),
                body: builder::BlockStatement::build_template(
                    lock,
                    template::BlockStatement {
                        metadata: Default::default(),
                        body: NodeList::new(lock),
                    },
                ),
                type_parameters: None,
                return_type: None,
                predicate: None,
                generator: false,
                is_async: false,
            },
        );

        // Create the global function.
        self.in_new_function(lock, global, |pself| {
            // Create the global scope.
            pself.in_new_scope(lock, global, |pself| {
                // If we are warning on undefined symbols, pre-define the known
                // globals to decrease the number of warnings.
                if lock.ctx().warn_undefined {
                    pself.declare_known_globals(lock, *global.range());
                }

                // Create the module scope as a function.
                pself.in_new_function(lock, node, |pself| {
                    pself.in_new_scope(lock, node, |pself| {
                        // Search for "use strict".
                        if find_use_strict(lock, &node_cast!(Node::Module, node).body).is_some() {
                            pself
                                .sem
                                .function_mut(pself.function_context().func_id)
                                .strict = true;
                        }

                        pself.process_collected_declarations(lock, node);
                        node.visit_children(lock, pself);
                    })
                });
            })
        });
    }
    /// Declare the well known globals to avoid strict mode warnings about them.
    /// `program_range`'s start is used as a synthetic location for the identifiers.
    fn declare_known_globals(&mut self, lock: &'gc GCLock, program_range: SourceRange) {
        // A range for the synthetic Identifier nodes we will create.
        let range = SourceRange {
            file: program_range.file,
            start: program_range.start,
            end: program_range.start,
        };
        for s in KNOWN_GLOBALS {
            let name = lock.atom(*s);
            debug_assert!(
                self.binding_table.value(name).is_none(),
                "Duplicated well known global"
            );

            // Allocate a synthetic identifier.
            let ident_node = builder::Identifier::build_template(
                lock,
                template::Identifier {
                    metadata: TemplateMetadata {
                        range,
                        ..Default::default()
                    },
                    name,
                    type_annotation: None,
                    optional: false,
                },
            );

            let decl = self
                .sem
                .new_global(name, DeclKind::UndeclaredGlobalProperty);
            self.binding_table.insert(
                name,
                Binding {
                    decl,
                    ident: node_cast!(Node::Identifier, ident_node),
                },
            );
        }
    }

    fn visit_function_like(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>) {
        self.in_new_function(lock, node, |pself| {
            let body = node.function_like_body();
            // Search for "use strict".
            let use_strict = match body {
                Node::BlockStatement(b) => find_use_strict(lock, &b.body),
                _ => None,
            };
            // Set the strictness if we have to.
            if use_strict.is_some() {
                pself
                    .sem
                    .function_mut(pself.function_context().func_id)
                    .strict = true;
            }

            // Note that we are associating the new scope with the body. We are doing this
            // because function expressions need an extra scope for the name, and we
            // associate that with the function expression itself.
            pself.in_new_scope(lock, body, |pself| {
                // All parameter identifiers.
                let mut param_ids = SmallVec::<[&Node; 8]>::new();
                // Set to false if the parameter list contains binding patterns.
                let simple_param_list =
                    node.function_like_params()
                        .iter()
                        .fold(true, |simple, elem| {
                            Self::extract_declared_idents_from_id(lock, Some(elem), &mut param_ids);
                            simple && elem.variant() != NodeVariant::Pattern
                        });

                if let (false, Some(strict)) = (simple_param_list, use_strict) {
                    lock.sm().error(
                        *strict.range(),
                        "'use strict' not allowed inside function with non-simple parameter list",
                    );
                }

                // Whether parameters must be unique.
                let unique_params = !simple_param_list
                    || pself.function_strict_mode()
                    || node_isa!(Node::ArrowFunctionExpression, node);

                // Declare the parameters.
                for param_id_node in param_ids {
                    let param_id = node_cast!(Node::Identifier, param_id_node);
                    pself.validate_declaration_name(lock, DeclKind::Parameter, param_id);

                    let param_decl = pself.sem.new_decl(
                        pself.current_scope.unwrap(),
                        param_id.name,
                        DeclKind::Parameter,
                    );
                    pself
                        .sem
                        .set_ident_decl(NodeRc::from_node(lock, param_id_node), param_decl);

                    match pself.binding_table.get_mut(&param_id.name) {
                        // Check for parameter re-declaration.
                        Some(prev_binding)
                            if Some(pself.sem.decl(prev_binding.decl).scope)
                                == pself.current_scope =>
                        {
                            if unique_params {
                                lock.sm().error(
                                    *param_id_node.range(),
                                    format!(
                                        "cannot declare two parameters with the same name '{}'",
                                        lock.str(param_id.name)
                                    ),
                                );
                            }

                            // Update the name binding to point to the latest declaration.
                            prev_binding.decl = param_decl;
                            prev_binding.ident = param_id;
                        }
                        // Just add the new parameter.
                        _ => {
                            pself.binding_table.insert(
                                param_id.name,
                                Binding {
                                    decl: param_decl,
                                    ident: param_id,
                                },
                            );
                        }
                    }
                }

                // Do not visit the identifier node, because that would try to resolve it
                // in an incorrect scope!
                // if let Some(id) = node.function_like_id() {
                //     pself.call(lock, id, Some(node));
                // }

                // Visit the parameters before we have hoisted the body declarations.
                {
                    pself.validating_formal_params = true;
                    for param in node.function_like_params() {
                        param.visit(lock, pself, Some(Path::new(node, NodeField::param)));
                    }
                    pself.validating_formal_params = false;
                }

                if let Node::BlockStatement(_) = body {
                    if !pself.function_strict_mode() {
                        pself.promote_scoped_func_decls(lock, node);
                    }
                }
                pself.process_collected_declarations(lock, node);

                // Finally visit the body.
                body.visit(lock, pself, Some(Path::new(node, NodeField::body)));

                // Check for local eval and run the unresolver pass in non-strict mode.
                let func = pself.sem.function(pself.function_context().func_id);
                let scope = pself.sem.scope(func.scopes[0]);
                if scope.local_eval && !func.strict {
                    let depth = pself.sem.scope(func.scopes[0]).depth;
                    Unresolver::new(pself, depth).run(lock, node);
                }
            });
        });
    }

    fn visit_for_in_of(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>, left: &'gc Node<'gc>) {
        self.with_new_label(lock, None, node, |pself| {
            // Ensure the initializer is valid.
            if let Node::VariableDeclaration(vd) = left {
                let declarator =
                    node_cast!(Node::VariableDeclarator, vd.declarations.head().unwrap());
                if let Some(init) = declarator.init {
                    if init.is_pattern() {
                        lock.sm().error(
                            *init.range(),
                            "destructuring declaration cannot be initialized in for-in/for-of loop",
                        );
                    } else if !(matches!(node, Node::ForInStatement(_))
                        && !pself.function_strict_mode()
                        && vd.kind == ast::VariableDeclarationKind::Var)
                    {
                        lock.sm().error(
                            *init.range(),
                            "for-in/for-of variable declaration may not be initialized",
                        );
                    }
                }
            } else {
                pself.validate_assignment_target(lock, left);
            }
            // Only create a lexical scope if there are declarations in it.
            if let Some(decls) = pself.function_context().decls.scope_decls_for_node(node) {
                pself.in_new_scope(lock, node, |pself| {
                    pself.process_declarations(lock, decls.as_slice());
                    node.visit_children(lock, pself);
                });
            } else {
                node.visit_children(lock, pself);
            }
        });
    }

    fn visit_function_expression(
        &mut self,
        lock: &'gc GCLock,
        fe: &ast::FunctionExpression<'gc>,
        node: &'gc Node<'gc>,
    ) {
        // If there is a name, declare it.
        if let Some(node_id @ Node::Identifier(ident)) = fe.id {
            self.in_new_scope(lock, node, |pself| {
                if pself.validate_declaration_name(lock, DeclKind::FunctionExprName, ident) {
                    let decl = pself.sem.new_decl(
                        pself.current_scope.unwrap(),
                        ident.name,
                        DeclKind::FunctionExprName,
                    );
                    pself
                        .sem
                        .set_ident_decl(NodeRc::from_node(lock, node_id), decl);
                    pself
                        .binding_table
                        .insert(ident.name, Binding { decl, ident });
                }
                pself.visit_function_like(lock, node);
            });
        } else {
            self.visit_function_like(lock, node);
        }
    }

    fn visit_identifier(
        &mut self,
        lock: &'gc GCLock,
        ident: &'gc Identifier<'gc>,
        node: &'gc Node<'gc>,
        path: Path<'gc>,
    ) {
        match path.parent {
            // { identifier: ... }
            Node::Property(ast::Property {
                computed: false,
                key: child,
                ..
            })
            // expr.identifier
            | Node::MemberExpression(ast::MemberExpression {
                computed: false,
                property: child,
                ..
            }) if child.ptr_eq(node) => return,
            Node::MetaProperty(_)
            | Node::BreakStatement(_)
            | Node::ContinueStatement(_)
            | Node::LabeledStatement(_) => return,
            // typeof identifier
            Node::UnaryExpression(ast::UnaryExpression {
                operator: UnaryExpressionOperator::Typeof,
                ..
            }) => {
                self.resolve_identifier(lock, ident, node, true);
                return;
            }
            _ => {}
        }
        self.resolve_identifier(lock, ident, node, false);
    }

    fn visit_block_statement(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>, path: Path) {
        // Some nodes with attached BlockStatement have already dealt with the scope.
        if let Node::FunctionDeclaration(_)
        | Node::FunctionExpression(_)
        | Node::ArrowFunctionExpression(_)
        | Node::CatchClause(_) = path.parent
        {
            return node.visit_children(lock, self);
        }

        // Only create a lexical scope if there are declarations in it.
        if let Some(decls) = self.function_context().decls.scope_decls_for_node(node) {
            self.in_new_scope(lock, node, |pself| {
                pself.process_declarations(lock, decls.as_slice());
                node.visit_children(lock, pself);
            });
        } else {
            node.visit_children(lock, self);
        }
    }

    /// Look up the `ident` to see if it already has been resolved or has a binding assigned.
    /// Assigns the associated declaration if it exists.
    /// If the identifier is unresolvable, returns `None`.
    fn check_identifier_resolved(
        &mut self,
        lock: &'gc GCLock,
        ident: &'gc Identifier<'gc>,
        node: &'gc Node<'gc>,
    ) -> Option<DeclId> {
        let ptr = NodeRc::from_node(lock, node);
        if let Some(Resolution::Decl(decl)) = self.sem.ident_decl(&ptr) {
            // If identifier already resolved, pick the resolved declaration.
            Some(decl)
        } else if let Some(Resolution::Unresolvable) = self.sem.ident_decl(&ptr) {
            None
        } else if let Some(b) = self.binding_table.value(ident.name) {
            // If we can find the binding, assign the associated declaration and return it.
            self.sem.set_ident_decl(ptr, b.decl);
            Some(b.decl)
        } else {
            None
        }
    }

    fn resolve_identifier(
        &mut self,
        lock: &'gc GCLock,
        ident: &'gc Identifier<'gc>,
        node: &'gc Node<'gc>,
        in_typeof: bool,
    ) {
        let decl = self.check_identifier_resolved(lock, ident, node);

        // Is this "arguments" in a function?
        if ident.name == self.kw.ident_arguments && !self.function_context().func_id.is_global() {
            if decl.is_none() || !self.decl_in_cur_function(decl.unwrap()) {
                let args_decl = self
                    .sem
                    .func_arguments_decl(self.function_context().func_id, ident.name);
                self.sem
                    .set_ident_decl(NodeRc::from_node(lock, node), args_decl);
            }
            return;
        }

        if decl.is_some() {
            return;
        }

        // Emit warning.
        if !in_typeof && lock.ctx().warn_undefined && self.function_strict_mode() {
            lock.sm().warning(
                ident.metadata.range,
                format!(
                    "identifier '{}' was not declared in function '{}'",
                    lock.str(ident.name),
                    self.function_context().name_str(lock).unwrap_or("")
                ),
            );
        }

        // Declare an ambient global property.
        let global_decl = self
            .sem
            .new_global(ident.name, DeclKind::UndeclaredGlobalProperty);
        self.sem
            .set_ident_decl(NodeRc::from_node(lock, node), global_decl);
        self.binding_table
            .insert_into_scope(
                self.global_binding_scope_depth.unwrap(),
                ident.name,
                Binding {
                    decl: global_decl,
                    ident,
                },
            )
            .unwrap();
    }

    /// Declare all declarations optionally associated with `scope_node` by
    /// [`DeclCollector`] in the current scope.
    fn process_collected_declarations(&mut self, lock: &'gc GCLock, scope_node: &'gc Node<'gc>) {
        if let Some(scope_decls) = self
            .function_context()
            .decls
            .scope_decls_for_node(scope_node)
        {
            self.process_declarations(lock, scope_decls.as_slice());
        }
    }

    /// Declare all declarations from `scope_decls` in the current scope by
    /// calling [`Self::validate_and_declare_identifier()`].
    fn process_declarations(&mut self, lock: &'gc GCLock, scope_decls: &[&'gc Node<'gc>]) {
        for &decl in scope_decls {
            let mut idents = SmallVec::<[&Node; 4]>::new();
            let decl_kind = self.extract_idents_from_decl(lock, decl, &mut idents);

            for &id_node in &idents {
                self.validate_and_declare_identifier(lock, decl_kind, id_node);
            }
        }
    }

    /// Try to create a declaration of the specified kind and name in the current
    /// scope. If the declaration is invalid, print an error message without
    /// creating it.
    /// \param declKind the semantic declaration kind
    /// \param idNode the AST node containing the name
    /// \param declNode the AST node of the declaration. Used for function
    ///     declarations.
    fn validate_and_declare_identifier(
        &mut self,
        lock: &'gc GCLock,
        decl_kind: DeclKind,
        id_node: &'gc Node<'gc>,
    ) {
        let ident = node_cast!(Node::Identifier, id_node);

        if !self.validate_declaration_name(lock, decl_kind, ident) {
            return;
        }

        let mut prev_binding = self.binding_table.value(ident.name);

        // Redeclaration of `arguments` in non-strict mode is allowed at the function level,
        // so we don't need to declare a new variable.
        // We do need to check that this isn't the global function, because `arguments` is a valid
        // variable name in the global function in non-strict mode.
        if !self.function_strict_mode()
            && ident.name == self.kw.ident_arguments
            && decl_kind.is_var_like()
            && !self.function_context().func_id.is_global()
        {
            return;
        }

        let mut decl_id = None;

        // Ignore declarations in enclosing functions.
        if let Some(pb) = prev_binding {
            if !self.decl_in_cur_function(pb.decl) {
                prev_binding = None;
            }
        }

        // Handle re-declarations, ignoring ambient properties.
        if let Some(pb) = prev_binding {
            let prev_kind = self.sem.decl(pb.decl).kind;
            let same_scope = Some(self.sem.decl(pb.decl).scope) == self.current_scope;

            if prev_kind != DeclKind::UndeclaredGlobalProperty {
                // Check whether the redeclaration is invalid.
                // Note that since "var" declarations have been hoisted to the function
                // scope, we cannot catch cases where "var" follows something declared in a
                // surrounding lexical scope.
                //
                // ES5Catch, var
                //          -> valid, special case ES10 B.3.5, but we can't catch it here.
                // var|scopedFunction, var|scopedFunction
                //          -> always valid
                // let, var
                //          -> always invalid
                // let, scopedFunction
                //          -> invalid if same scope
                // var|scopedFunction|let, let
                //          -> invalid if the same scope

                if (prev_kind.is_let_like() && decl_kind.is_var_like())
                    || (prev_kind.is_let_like()
                        && decl_kind == DeclKind::ScopedFunction
                        && same_scope)
                    || (decl_kind.is_let_like() && same_scope)
                {
                    lock.sm().error(
                        ident.metadata.range,
                        format!("identifier '{}' is already declared", lock.str(ident.name)),
                    );
                    lock.sm()
                        .note(pb.ident.metadata.range, "previous declaration");
                    return;
                }

                // When to create a new declaration?
                //
                if prev_kind.is_var_like() && decl_kind.is_var_like() {
                    // Var, Var -> use prev
                    decl_id = Some(pb.decl);
                } else if prev_kind.is_var_like() && decl_kind.is_var_like_or_scoped_function() {
                    decl_id = if same_scope || !self.function_strict_mode() {
                        // Var, ScopedFunc -> if non-strict or same scope, then use prev,
                        //                    else declare new
                        Some(pb.decl)
                    } else {
                        None
                    };
                } else if prev_kind == DeclKind::ScopedFunction
                    // ScopedFunc, ScopedFunc same scope -> use prev
                    // ScopedFunc, ScopedFunc new scope -> declare new
                    && decl_kind == DeclKind::ScopedFunction
                {
                    decl_id = if same_scope { Some(pb.decl) } else { None };
                } else if prev_kind == DeclKind::ScopedFunction && decl_kind.is_var_like() {
                    // ScopedFunc, Var -> convert to var
                    debug_assert!(
                        same_scope,
                        "we can only encounter Var after ScopedFunction in the same scope"
                    );
                    // Since they are in the same scope, we can simply convert the existing
                    // ScopedFunction to Var.
                    decl_id = Some(pb.decl);
                    self.sem.decl_mut(pb.decl).kind = DeclKind::Var;
                } else {
                    decl_id = None;
                }
            }
        }

        if decl_id.is_none() {
            let new_decl = if decl_kind.is_global() {
                self.sem.new_global(ident.name, decl_kind)
            } else {
                self.sem
                    .new_decl(self.current_scope.unwrap(), ident.name, decl_kind)
            };
            self.binding_table.insert(
                ident.name,
                Binding {
                    decl: new_decl,
                    ident,
                },
            );
            decl_id = Some(new_decl);
        };

        self.sem
            .set_ident_decl(NodeRc::from_node(lock, id_node), decl_id.unwrap());
    }

    /// Ensure that the specified identifier is valid to be used in a declaration.
    /// Return true if valid, otherwise generate an error and return false.
    fn validate_declaration_name(
        &self,
        lock: &'gc GCLock,
        decl_kind: DeclKind,
        ident: &Identifier,
    ) -> bool {
        if self.function_strict_mode() {
            // - 'arguments' cannot be redeclared in strict mode.
            // - 'eval' cannot be redeclared in strict mode.
            if ident.name == self.kw.ident_arguments || ident.name == self.kw.ident_eval {
                lock.sm().error(
                    ident.metadata.range,
                    format!("cannot declare '{}' in strict mode", lock.str(ident.name)),
                );
                return false;
            }

            // Parameter cannot be named "let".
            if decl_kind == DeclKind::Parameter && ident.name == self.kw.ident_let {
                lock.sm().error(
                    ident.metadata.range,
                    "invalid parameter name 'let' in strict mode",
                );
                return false;
            }
        }

        if (decl_kind == DeclKind::Let || decl_kind == DeclKind::Const)
            && ident.name == self.kw.ident_let
        {
            // ES9.0 13.3.1.1
            // LexicalDeclaration : LetOrConst BindingList
            // It is a Syntax Error if the BoundNames of BindingList
            // contains "let".
            lock.sm().error(
                ident.metadata.range,
                "'let' is disallowed as a lexically bound name",
            );
            return false;
        }

        true
    }

    /// Ensure that the specified node is a valid target for an assignment, in
    /// other words it is an l-value, a Pattern (checked recursively) or an Empty
    /// (used by elision).
    fn validate_assignment_target(&self, lock: &GCLock, node: &Node) {
        match node {
            Node::Empty(_) => {}
            Node::AssignmentPattern(ast::AssignmentPattern { left, .. }) => {
                self.validate_assignment_target(lock, left)
            }
            Node::ArrayPattern(ast::ArrayPattern { elements, .. }) => {
                for element in elements.iter() {
                    self.validate_assignment_target(lock, element)
                }
            }
            Node::Property(ast::Property { value, .. }) => {
                self.validate_assignment_target(lock, value)
            }
            Node::ObjectPattern(ast::ObjectPattern { properties, .. }) => {
                for property in properties.iter() {
                    self.validate_assignment_target(lock, property)
                }
            }
            Node::RestElement(ast::RestElement { argument, .. }) => {
                self.validate_assignment_target(lock, argument)
            }
            _ => {
                if !self.is_lvalue(node) {
                    lock.sm()
                        .error(*node.range(), "invalid assignment left-hand side");
                }
            }
        };
    }

    /// Extract the list of declared identifiers in a declaration node and return
    /// the declaration kind of the node. Function declarations are always returned
    /// as DeclKind::ScopedFunction, so they can be distinguished.
    fn extract_idents_from_decl<A: smallvec::Array<Item = &'gc Node<'gc>>>(
        &self,
        lock: &'gc GCLock,
        decl: &'gc Node<'gc>,
        idents: &mut SmallVec<A>,
    ) -> DeclKind {
        match decl {
            Node::VariableDeclaration(n) => {
                for declarator in n.declarations {
                    Self::extract_declared_idents_from_id(
                        lock,
                        Some(node_cast!(Node::VariableDeclarator, declarator).id),
                        idents,
                    );
                }
                match n.kind {
                    VariableDeclarationKind::Var => {
                        if self.binding_table.current_scope_depth()
                            == self.global_binding_scope_depth.unwrap()
                        {
                            DeclKind::GlobalProperty
                        } else {
                            DeclKind::Var
                        }
                    }
                    VariableDeclarationKind::Let => DeclKind::Let,
                    VariableDeclarationKind::Const => DeclKind::Const,
                }
            }
            Node::FunctionDeclaration(n) => {
                Self::extract_declared_idents_from_id(lock, n.id, idents);
                if self.binding_table.current_scope_depth()
                    == self.global_binding_scope_depth.unwrap()
                {
                    DeclKind::GlobalProperty
                } else {
                    DeclKind::ScopedFunction
                }
            }
            Node::ClassDeclaration(n) => {
                Self::extract_declared_idents_from_id(lock, n.id, idents);
                DeclKind::Class
            }
            Node::ImportDeclaration(n) => {
                for spec in n.specifiers {
                    match spec {
                        Node::ImportSpecifier(nn) => {
                            Self::extract_declared_idents_from_id(lock, Some(nn.local), idents);
                        }
                        Node::ImportDefaultSpecifier(nn) => {
                            Self::extract_declared_idents_from_id(lock, Some(nn.local), idents);
                        }
                        Node::ImportNamespaceSpecifier(nn) => {
                            Self::extract_declared_idents_from_id(lock, Some(nn.local), idents);
                        }
                        nn => {
                            lock.sm().error(
                                *nn.range(),
                                format!("unsupported import specifier kind {}", nn.name()),
                            );
                        }
                    }
                }
                DeclKind::Import
            }
            n => {
                lock.sm().error(
                    *n.range(),
                    format!("unsupported declaration kind {}", n.name()),
                );
                DeclKind::Var
            }
        }
    }

    /// Extract the declared identifiers from a declaration AST node's "id" field.
    /// Normally that is just a single identifier, but it can be more in case of
    /// destructuring.
    fn extract_declared_idents_from_id<A: smallvec::Array<Item = &'gc Node<'gc>>>(
        lock: &'gc GCLock,
        node_opt: Option<&'gc Node<'gc>>,
        idents: &mut SmallVec<A>,
    ) {
        // The identifier is sometimes optional, in which case it is valid.
        if node_opt.is_none() {
            return;
        }
        let node = node_opt.unwrap();

        match node {
            Node::Identifier(_) => {
                idents.push(node);
            }
            Node::Empty(_) => {}
            Node::AssignmentPattern(n) => {
                Self::extract_declared_idents_from_id(lock, Some(n.left), idents);
            }
            Node::ArrayPattern(n) => {
                for elem in n.elements {
                    Self::extract_declared_idents_from_id(lock, Some(elem), idents);
                }
            }
            Node::RestElement(n) => {
                Self::extract_declared_idents_from_id(lock, Some(n.argument), idents);
            }
            Node::ObjectPattern(n) => {
                for prop in n.properties {
                    match prop {
                        Node::Property(nn) => {
                            Self::extract_declared_idents_from_id(lock, Some(nn.value), idents);
                        }
                        Node::RestElement(nn) => {
                            Self::extract_declared_idents_from_id(lock, Some(nn.argument), idents);
                        }
                        bn => lock.sm().error(
                            *bn.range(),
                            format!("unexpected ObjectPattern property {}", bn.name()),
                        ),
                    }
                }
            }
            _ => {
                lock.sm().error(
                    *node.range(),
                    format!("invalid destructuring target {}", node.name()),
                );
            }
        }
    }

    /// Mark the current scope and every one of its ancestor scopes as users of local `eval()`.
    /// Set the `can_rename` flag on every variable in the scopes to `false`.
    fn register_local_eval(&mut self) {
        let mut id = self.current_scope.unwrap();
        loop {
            // Flag this scope as having a local `eval` call.
            self.sem.scope_mut(id).local_eval = true;
            // Set every variable as not being able to rename.
            // TODO: It can be possible to avoid setting this flag if the variable is shadowed
            // in a scope between this one and the local `eval()` call, so investigate that
            // at a later time if necessary.
            for &decl_id in &self.sem.scopes.get(id).decls {
                self.sem.decls.get_mut(decl_id).can_rename = false;
            }
            match self.sem.scope(id).parent_scope {
                None => break,
                Some(p) => {
                    id = p;
                }
            };
        }
    }

    /// Return true if the `node` is an LValue: a member expression or an identifier which is a
    /// valid LValue.
    fn is_lvalue(&self, node: &Node) -> bool {
        match node {
            Node::MemberExpression(_) => true,

            Node::Identifier(id) if id.name == self.kw.ident_arguments => {
                !self.function_strict_mode()
            }
            Node::Identifier(id) if id.name == self.kw.ident_eval => !self.function_strict_mode(),
            Node::Identifier(_) => true,

            _ => false,
        }
    }

    fn promote_scoped_func_decls(&mut self, lock: &'gc GCLock, func_node: &'gc Node<'gc>) {
        debug_assert!(func_node.is_function_like());
        if self.function_context().decls.scoped_func_decls().is_empty() {
            // No scoped function declarations, nothing to promote.
            return;
        }
        ScopedFunctionPromoter::new(self).run(lock, func_node);
    }
}

impl<'gc> Visitor<'gc> for Resolver<'gc, '_> {
    fn call(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>, path: Option<Path<'gc>>) {
        match node {
            Node::Program(_) => {
                panic!("visited unexpected {}", node.name())
            }

            Node::FunctionDeclaration(..) => {
                // Collect hoisted function.
                self.sem
                    .scope_mut(self.current_scope.unwrap())
                    .hoisted_functions
                    .push(NodeRc::from_node(lock, node));
                self.visit_function_like(lock, node);
            }

            Node::FunctionExpression(fe) => self.visit_function_expression(lock, fe, node),
            Node::ArrowFunctionExpression(_) => self.visit_function_like(lock, node),

            Node::Identifier(ident) => self.visit_identifier(lock, ident, node, path.unwrap()),

            Node::AssignmentExpression(asgn) => {
                self.validate_assignment_target(lock, asgn.left);
                node.visit_children(lock, self);
            }

            Node::UpdateExpression(update) => {
                if !self.is_lvalue(update.argument) {
                    lock.sm().error(
                        *update.argument.range(),
                        "invalid operand in update operation",
                    );
                }
                node.visit_children(lock, self);
            }

            Node::BlockStatement(_) => self.visit_block_statement(lock, node, path.unwrap()),

            Node::SwitchStatement(switch) => {
                // Visit the discriminant before creating a new scope.
                switch.discriminant.visit(
                    lock,
                    self,
                    Some(Path::new(node, NodeField::discriminant)),
                );
                self.with_new_label(lock, None, node, |pself| {
                    pself.in_new_scope(lock, node, |pself| {
                        if let Some(decls) =
                            pself.function_context().decls.scope_decls_for_node(node)
                        {
                            pself.process_declarations(lock, &decls);
                        }
                        for case in switch.cases {
                            case.visit(lock, pself, Some(Path::new(node, NodeField::cases)));
                        }
                    });
                });
            }

            Node::ForInStatement(ast::ForInStatement { left, .. })
            | Node::ForOfStatement(ast::ForOfStatement { left, .. }) => {
                self.visit_for_in_of(lock, node, left)
            }

            Node::ForStatement(_) => {
                self.with_new_label(lock, None, node, |pself| {
                    // Only create a lexical scope if there are declarations in it.
                    if let Some(decls) = pself.function_context().decls.scope_decls_for_node(node) {
                        pself.in_new_scope(lock, node, |pself| {
                            pself.process_declarations(lock, decls.as_slice());
                            node.visit_children(lock, pself);
                        });
                    } else {
                        node.visit_children(lock, pself);
                    }
                });
            }

            Node::WhileStatement(_) | Node::DoWhileStatement(_) => {
                self.with_new_label(lock, None, node, |pself| {
                    node.visit_children(lock, pself);
                });
            }

            Node::LabeledStatement(labeled) => {
                self.with_new_label(lock, Some(labeled.label), node, |pself| {
                    node.visit_children(lock, pself);
                });
            }

            Node::BreakStatement(ast::BreakStatement {
                label: Some(label), ..
            }) => {
                let name = node_cast!(Node::Identifier, label).name;
                if self.function_context().label_table.get(&name).is_none() {
                    lock.sm().error(
                        *label.range(),
                        format!("label '{}' is not defined", lock.str(name)),
                    );
                }
            }
            Node::BreakStatement(ast::BreakStatement { label: None, .. }) => {
                if self.current_loop_or_switch.is_none() {
                    lock.sm()
                        .error(*node.range(), "'break' not within a loop or switch");
                }
            }

            Node::ContinueStatement(ast::ContinueStatement {
                label: Some(label_node),
                ..
            }) => {
                let name = node_cast!(Node::Identifier, label_node).name;
                match self.function_context().label_table.get(&name) {
                    Some(label) => {
                        if matches!(label.target_statement, Node::LabeledStatement(_)) {
                            lock.sm().error(
                                *label_node.range(),
                                format!(
                                    "'continue' label '{}' is not a loop label",
                                    lock.str(name)
                                ),
                            );
                        }
                    }
                    None => {
                        lock.sm().error(
                            *label_node.range(),
                            format!("label '{}' is not defined", lock.str(name)),
                        );
                    }
                }
            }
            Node::ContinueStatement(ast::ContinueStatement { label: None, .. }) => {
                if self.current_loop.is_none() {
                    lock.sm()
                        .error(*node.range(), "'continue' not within a loop or switch");
                }
            }

            Node::WithStatement(ast::WithStatement { body, .. }) => {
                node.visit_children(lock, self);
                // Run the Unresolver to avoid resolving to variables past the depth of the `with`.
                // Pass `depth + 1` because variables declared in this scope also cannot be trusted.
                let depth = self.sem.scope(self.current_scope.unwrap()).depth;
                Unresolver::new(self, depth + 1).run(lock, body);
            }

            Node::CatchClause(ast::CatchClause { param, body, .. }) => {
                self.in_new_scope(lock, node, |pself| {
                    if let Some(id_node @ Node::Identifier(_)) = param {
                        // For compatibility with ES5, we need to treat a single catch variable
                        // specially, see: B.3.5 VariableStatements in Catch Blocks
                        // https://www.ecma-international.org/ecma-262/10.0/index.html#sec-variablestatements-in-catch-blocks
                        pself.validate_and_declare_identifier(lock, DeclKind::ES5Catch, id_node);
                    } else {
                        let mut idents = SmallVec::<[&Node; 4]>::new();
                        Self::extract_declared_idents_from_id(lock, *param, &mut idents);
                        for id_node in idents {
                            pself.validate_and_declare_identifier(lock, DeclKind::Let, id_node);
                        }
                    }
                    // Process body's declarations, skip visiting it, visit its children.
                    pself.process_collected_declarations(lock, body);
                    body.visit_children(lock, pself);
                });
            }

            Node::MetaProperty(ast::MetaProperty {
                meta: Node::Identifier(meta),
                property: Node::Identifier(prop),
                ..
            }) => {
                // Validate "new.target"
                if meta.name == self.kw.ident_new
                    && prop.name == self.kw.ident_target
                    && self.function_context().func_id.is_global()
                {
                    // ES9.0 15.1.1:
                    // It is a Syntax Error if StatementList Contains NewTarget unless the
                    // source code containing NewTarget is eval code that is being processed
                    // by a direct eval.
                    lock.sm()
                        .error(*node.range(), "'new.target' outside of a function");
                }
            }

            Node::ImportDeclaration(ast::ImportDeclaration {
                source: Node::StringLiteral(ast::StringLiteral { value, .. }),
                ..
            }) => {
                node.visit_children(lock, self);
                if let ResolverMode::Module {
                    dependency_resolver,
                } = self.mode
                {
                    // Resolve `import`.
                    let target = String::from_utf16_lossy(lock.str_u16(*value));
                    match dependency_resolver.resolve_dependency(
                        lock,
                        self.file_id,
                        &target,
                        DependencyKind::Import,
                    ) {
                        Some(file_id) => {
                            self.sem.add_require(NodeRc::from_node(lock, node), file_id);
                        }
                        None => {
                            lock.sm().warning(
                                *node.range(),
                                format!("Unable to resolve import for {}", target),
                            );
                        }
                    }
                }
            }

            Node::ClassDeclaration(_) | Node::ClassExpression(_) => {
                let old_strict = self.function_strict_mode();
                *self.function_strict_mode_mut() = true;
                node.visit_children(lock, self);
                *self.function_strict_mode_mut() = old_strict;
            }

            Node::CallExpression(call @ ast::CallExpression { arguments, .. }) => {
                // Check for a direct call to local `eval()`.
                if let Node::Identifier(identifier) = call.callee {
                    if identifier.name == self.kw.ident_eval {
                        let is_eval = match self.binding_table.get(&identifier.name) {
                            None => true,
                            Some(name) => {
                                let decl = self.sem.decl(name.decl);
                                decl.scope.is_global() && decl.kind.is_var_like()
                            }
                        };
                        if is_eval {
                            self.register_local_eval();
                        }
                    }
                }

                node.visit_children(lock, self);
                if let ResolverMode::Module {
                    dependency_resolver,
                } = self.mode
                {
                    if self.is_require(lock, call) {
                        // Resolve `require()` call.
                        if let Some(Node::StringLiteral(ast::StringLiteral { value, .. })) =
                            arguments.head()
                        {
                            let target = String::from_utf16_lossy(lock.str_u16(*value));
                            match dependency_resolver.resolve_dependency(
                                lock,
                                self.file_id,
                                &target,
                                DependencyKind::Require,
                            ) {
                                Some(file_id) => {
                                    self.sem.add_require(NodeRc::from_node(lock, node), file_id);
                                }
                                None => {
                                    lock.sm().warning(
                                        *node.range(),
                                        format!("Unable to resolve require for {}", target),
                                    );
                                }
                            }
                        }
                    }
                }
            }

            Node::YieldExpression(_) => {
                match self.function_context().node {
                    Node::Program(_) => {
                        lock.sm()
                            .error(*node.range(), "'yield' not in a generator function");
                    }
                    Node::FunctionExpression(ast::FunctionExpression {
                        generator: false, ..
                    })
                    | Node::FunctionDeclaration(ast::FunctionDeclaration {
                        generator: false,
                        ..
                    }) => {
                        lock.sm()
                            .error(*node.range(), "'yield' not in a generator function");
                    }
                    _ => {}
                }
                node.visit_children(lock, self);
            }

            _ => {
                node.visit_children(lock, self);
            }
        }
    }
}

/// Scan a list of directives in the beginning of a program of function
/// (see ES5.1 4.1 - a directive is a statement consisting of a single
/// string literal).
/// \return the node containing "use strict" or nullptr.
#[allow(clippy::ptr_arg)]
fn find_use_strict<'gc>(lock: &'gc GCLock, body: &'gc NodeList<'gc>) -> Option<&'gc Node<'gc>> {
    /// "use strict" encoded as UTF-16.
    static USE_STRICT_UTF16: [u16; 10] = [
        'u' as u16, 's' as u16, 'e' as u16, ' ' as u16, 's' as u16, 't' as u16, 'r' as u16,
        'i' as u16, 'c' as u16, 't' as u16,
    ];

    // Scan until we encounter a non-directive.
    for node in *body {
        match node {
            Node::ExpressionStatement(ast::ExpressionStatement {
                directive: Some(d), ..
            }) => {
                if lock.str_u16(*d) == USE_STRICT_UTF16 {
                    return Some(node);
                }
            }
            _ => break,
        }
    }
    None
}

/// This class checks whether it is safe to promote block-scoped function
/// declarations to function scope. i.e. whether it is safe to replace one with
/// "var" without creating a conflict, and promotes them.
///
/// A conflict exists if a lex-like declaration is visible in the declaration
/// scope. The checker starts with a list of all block scoped function
/// declarations. Then it visits all scopes recursively, maintaining a scoped
/// table of let-like declarations with matching names. When it encounters a
/// block-scoped function declaration, it checks whether a matching let-like
/// declaration is visible. If not, it is safe to promote.
///
/// The input is a list of block-scoped function function declarations. The
/// ones that can be promoted are deleted from their own scope and added to the
/// function scope.
struct ScopedFunctionPromoter<'a, 'gc, 'mode> {
    resolver: &'a mut Resolver<'gc, 'mode>,

    /// The names of the scoped functions. We will ignore all other identifiers.
    func_names: HashSet<Atom>,

    /// The scoped function declarations. We remove each from this set once
    /// we encounter it.
    func_decls: HashSet<NodePtr<'gc>>,

    /// The currently lexically visible names.
    binding_table: ScopedHashMap<Atom, ()>,
}

impl<'a, 'gc, 'mode> ScopedFunctionPromoter<'a, 'gc, 'mode> {
    fn new(resolver: &'a mut Resolver<'gc, 'mode>) -> Self {
        ScopedFunctionPromoter {
            resolver,
            func_names: Default::default(),
            func_decls: Default::default(),
            binding_table: Default::default(),
        }
    }

    /// Run on the `func_node` and promote any necessary functions.
    fn run(&mut self, lock: &'gc GCLock, func_node: &'gc Node<'gc>) {
        debug_assert!(func_node.is_function_like());
        let decls = self.resolver.function_context().decls.scoped_func_decls();

        // Populate the sets.
        for node in decls {
            let func_decl = node_cast!(Node::FunctionDeclaration, node);
            self.func_names
                .insert(node_cast!(Node::Identifier, func_decl.id.unwrap()).name);
            self.func_decls.insert(NodePtr::from_node(node));
        }

        // The core of the loop doesn't descend into functions,
        // so we take care of the only time we have to examine a function scope.
        // Process the first decls here, because they're associated with the function like node,
        // not the BlockStatement that forms the body.
        // After processing those declarations, immediately start visiting the body's children,
        // because the body itself has been handled as part of the function's processing.
        self.process_declarations(lock, func_node);
        func_node.function_like_body().visit_children(lock, self);
    }

    /// Visit any statement starting a scope.
    fn visit_scope(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>) {
        self.binding_table.push_scope();
        self.process_declarations(lock, node);
        self.binding_table.pop_scope();
    }

    /// Process the declarations in a scope.
    /// This is the core of the algorithm, it updates the binding tables, etc.
    /// Promotes the scoped FunctionDeclarations to function scope if they don't conflict.
    fn process_declarations(&mut self, lock: &'gc GCLock, scope: &'gc Node<'gc>) {
        let decls = match self
            .resolver
            .function_context()
            .decls
            .scope_decls_for_node(scope)
        {
            None => return,
            Some(decls) => decls,
        };

        let mut idents = SmallVec::<[&Node; 4]>::new();

        // Whenever we encounter one of the scoped func decls we are trying to promote,
        // we store the entry here (so we can promote it if we want to).
        let mut found_decls = Vec::<&Node>::new();

        for &decl_node in decls.iter() {
            if let Node::FunctionDeclaration(_) = decl_node {
                if self.func_decls.contains(&decl_node.into()) {
                    // We encountered one of the candidate declarations.
                    // Add it to the found_decls list and move on.
                    found_decls.push(decl_node);
                }
                continue;
            }

            // Extract idents, report errors.
            idents.clear();
            let decl_kind = Self::extract_declared_idents(lock, decl_node, &mut idents);

            // We are only interested in let-like declarations.
            if !decl_kind.is_let_like() {
                continue;
            }

            // Remember only idents matching the set.
            for &ident_node in &idents {
                let ident = node_cast!(Node::Identifier, ident_node);
                if self.func_names.contains(&ident.name) {
                    self.binding_table.insert(ident.name, ());
                }
            }
        }

        if found_decls.is_empty() {
            // No work to do.
            return;
        }

        // Ensure we don't have the `Rc` alive when we do the actual promotion.
        drop(decls);

        // The decls list with the promoted function declarations omitted.
        let mut new_decls = ScopeDecls::new();

        for &node in &found_decls {
            // Remove it from the set, since we are no longer interested in it.
            self.func_decls.remove(&node.into());

            let func_decl = node_cast!(Node::FunctionDeclaration, node);
            match func_decl.id {
                None => {
                    // No name on the declaration, nothing to promote.
                    new_decls.push(node);
                    continue;
                }
                Some(id) => {
                    // Is there a visible let-like declaration with the same name?
                    // If so, it can't be promoted because it would shadow a `let`,
                    // so keep it in `new_decls` and move on.
                    if self
                        .binding_table
                        .contains_key(&node_cast!(Node::Identifier, id).name)
                    {
                        new_decls.push(node);
                        continue;
                    }
                }
            };

            // This block-scoped function declaration can (and should) be promoted.
            // 1. Don't add it to the new_decls list.
            // 2. Add it to the function scope list.
            self.resolver
                .function_context_mut()
                .decls
                .add_scope_decl_for_func(node);
        }

        self.resolver
            .function_context_mut()
            .decls
            .set_scope_decls_for_node(scope, Rc::new(new_decls));
    }

    /// Extract the list of declared identifiers in a declaration node into `idents`.
    /// Return the declaration kind of the node.
    /// Function declarations are always returned as `ScopedFunction`, so they can be distinguished.
    fn extract_declared_idents<A: smallvec::Array<Item = &'gc Node<'gc>>>(
        lock: &'gc GCLock,
        node: &'gc Node<'gc>,
        idents: &mut SmallVec<A>,
    ) -> DeclKind {
        match node {
            Node::VariableDeclaration(decl) => {
                for declarator in decl.declarations {
                    Resolver::extract_declared_idents_from_id(
                        lock,
                        Some(node_cast!(Node::VariableDeclarator, declarator).id),
                        idents,
                    )
                }
                match decl.kind {
                    VariableDeclarationKind::Let => DeclKind::Let,
                    VariableDeclarationKind::Const => DeclKind::Const,
                    VariableDeclarationKind::Var => DeclKind::Var,
                }
            }
            Node::FunctionDeclaration(decl) => {
                Resolver::extract_declared_idents_from_id(lock, decl.id, idents);
                DeclKind::ScopedFunction
            }
            Node::ClassDeclaration(decl) => {
                Resolver::extract_declared_idents_from_id(lock, decl.id, idents);
                DeclKind::Class
            }
            Node::ImportDeclaration(decl) => {
                for spec in decl.specifiers {
                    match spec {
                        Node::ImportSpecifier(spec) => {
                            Resolver::extract_declared_idents_from_id(
                                lock,
                                Some(spec.local),
                                idents,
                            );
                        }
                        Node::ImportDefaultSpecifier(spec) => {
                            Resolver::extract_declared_idents_from_id(
                                lock,
                                Some(spec.local),
                                idents,
                            );
                        }
                        Node::ImportNamespaceSpecifier(spec) => {
                            Resolver::extract_declared_idents_from_id(
                                lock,
                                Some(spec.local),
                                idents,
                            );
                        }
                        _ => {
                            unreachable!("Invalid import specifier: {:?}", node.variant());
                        }
                    }
                }
                DeclKind::Import
            }
            _ => {
                unreachable!("Invalid declaration kind: {:?}", node.variant());
            }
        }
    }
}

impl<'gc> Visitor<'gc> for ScopedFunctionPromoter<'_, 'gc, '_> {
    fn call(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>, _path: Option<Path<'gc>>) {
        match node {
            Node::SwitchStatement(_)
            | Node::BlockStatement(_)
            | Node::ForStatement(_)
            | Node::ForInStatement(_)
            | Node::ForOfStatement(_)
            | Node::WithStatement(_) => {
                self.visit_scope(lock, node);
                node.visit_children(lock, self);
            }
            _ => {
                // Do not descend into nested functions.
                if !node.is_function_like() {
                    node.visit_children(lock, self);
                }
            }
        };
    }
}

/// Visitor pass for marking variables as Unresolvable based on local `eval()` or `with`.
struct Unresolver<'a, 'gc, 'mode> {
    resolver: &'a mut Resolver<'gc, 'mode>,
    /// Depth of the scope which contains the construct which could shadow variables dynamically.
    /// e.g. the depth of the function containing a local `eval()`.
    depth: u32,
}

impl<'a, 'gc, 'mode> Unresolver<'a, 'gc, 'mode> {
    fn new(resolver: &'a mut Resolver<'gc, 'mode>, depth: u32) -> Self {
        Self { resolver, depth }
    }

    fn run(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>) {
        node.visit(lock, self, None);
    }
}

impl<'gc> Visitor<'gc> for Unresolver<'_, 'gc, '_> {
    fn call(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>, _path: Option<Path<'gc>>) {
        match node {
            Node::Identifier(_) => {
                let ident = NodeRc::from_node(lock, node);
                if let Some(Resolution::Decl(decl_id)) = self.resolver.sem.ident_decl(&ident) {
                    let decl = self.resolver.sem.decl(decl_id);
                    let scope = self.resolver.sem.scope(decl.scope);
                    // The depth of this identifier's declaration is less than the `eval`/`with`
                    // declaration that could shadow it, so we must declare this identifier as
                    // unresolvable.
                    if scope.depth < self.depth {
                        self.resolver.sem.set_ident_unresolvable(ident);
                    }
                }
                node.visit_children(lock, self);
            }
            _ => {
                node.visit_children(lock, self);
            }
        }
    }
}
