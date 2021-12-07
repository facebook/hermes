/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::ast::{Atom, NodeRc};
use std::collections::HashMap;
use std::num::NonZeroU32;

macro_rules! declare_opaque_id {
    ($name:ident) => {
        #[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
        pub struct $name(NonZeroU32);
        impl $name {
            #[inline]
            fn new(v: usize) -> Self {
                debug_assert!(v < u32::MAX as usize);
                unsafe { Self::new_unchecked(v) }
            }
            #[inline]
            const unsafe fn new_unchecked(v: usize) -> Self {
                Self(NonZeroU32::new_unchecked((v + 1) as u32))
            }
            fn as_usize(self) -> usize {
                (self.0.get() - 1) as usize
            }
        }
    };
}

declare_opaque_id!(DeclId);
declare_opaque_id!(LexicalScopeId);
declare_opaque_id!(FunctionInfoId);

impl LexicalScopeId {
    pub const GLOBAL_SCOPE_ID: LexicalScopeId = unsafe { LexicalScopeId::new_unchecked(0) };
    pub fn is_global(self) -> bool {
        self == Self::GLOBAL_SCOPE_ID
    }
}

impl FunctionInfoId {
    pub const GLOBAL_FUNCTION_ID: FunctionInfoId = unsafe { FunctionInfoId::new_unchecked(0) };
    pub fn is_global(self) -> bool {
        self == Self::GLOBAL_FUNCTION_ID
    }
}

#[derive(PartialOrd, PartialEq, Eq, Copy, Clone)]
pub enum DeclKind {
    // ==== Let-like declarations ===
    Let,
    Const,
    Class,
    Import,
    /// A single catch variable declared like this "catch (e)", see
    /// ES10 B.3.5 VariableStatements in Catch Blocks
    ES5Catch,

    // ==== other declarations ===
    FunctionExprName,
    /// Function declaration visible only in its lexical scope.
    ScopedFunction,

    // ==== Var-like declarations ===
    /// "var" in function scope.
    Var,
    Parameter,
    /// "var" in global scope.
    GlobalProperty,
    UndeclaredGlobalProperty,
}

impl DeclKind {
    /// Return true if this kind of declaration is function scope (and can be
    /// re-declared).
    pub fn is_var_like(self) -> bool {
        self >= Self::Var
    }
    pub fn is_var_like_or_scoped_function(self) -> bool {
        self >= Self::ScopedFunction
    }
    /// Return true if this kind of declaration is lexically scoped (and cannot
    /// be re-declared).
    pub fn is_let_like(self) -> bool {
        self <= Self::ES5Catch
    }
    /// Return true if this kind of declaration is a global property.
    pub fn is_global(self) -> bool {
        self >= Self::GlobalProperty
    }
}

pub enum Special {
    NotSpecial,
    Arguments,
    Eval,
}

pub struct Decl {
    /// The declared identifier.
    pub name: Atom,
    /// What kind of declaration it is.
    pub kind: DeclKind,
    /// If this is a special declaration, identify which one.
    pub special: Special,
    /// Set to true if this is a function initialized in this scope.
    pub function_in_scope: bool,
    /// The lexical scope of the declaration. Could be nullptr for special
    /// declarations, since they are technically unscoped.
    pub scope: LexicalScopeId,
}

pub struct LexicalScope {
    /// The function owning this lexical scope.
    pub parent_function: FunctionInfoId,
    /// The enclosing lexical scope (it could be in another function).
    pub parent_scope: Option<LexicalScopeId>,
    /// All declarations made in this scope.
    pub decls: Vec<DeclId>,
    /// A list of functions that need to be hoisted and materialized before we
    /// can generate the rest of the scope.
    pub hoisted_functions: Vec<NodeRc>,
}

pub struct FunctionInfo {
    /// The function surrounding this function.
    pub parent_function: Option<FunctionInfoId>,
    /// The enclosing lexical scope.
    pub parent_scope: Option<LexicalScopeId>,
    /// True if the function is strict mode.
    pub strict: bool,
    /// All lexical scopes in this function. The first one is the function scope.
    pub scopes: Vec<LexicalScopeId>,
    /// The implicitly declared "arguments" object. It is declared only if it is used.
    pub arguments_decl: Option<DeclId>,
}

#[derive(Default)]
pub struct SemContext {
    decls: Vec<Decl>,
    scopes: Vec<LexicalScope>,
    funcs: Vec<FunctionInfo>,
    /// Resolved identifiers: declarations associated with Identifier AST nodes.
    ident_decls: HashMap<NodeRc, DeclId>,
    /// Lexical scopes associated with AST nodes. Usually BlockStatement, but
    /// occasionally others.
    node_scopes: HashMap<NodeRc, LexicalScopeId>,
}

impl SemContext {
    pub(super) fn new_function(
        &mut self,
        parent_function: Option<FunctionInfoId>,
        parent_scope: Option<LexicalScopeId>,
        strict: bool,
    ) -> (FunctionInfoId, &FunctionInfo) {
        self.funcs.push(FunctionInfo {
            parent_function,
            parent_scope,
            strict,
            scopes: Default::default(),
            arguments_decl: Default::default(),
        });
        (
            FunctionInfoId::new(self.funcs.len() - 1),
            self.funcs.last().unwrap(),
        )
    }

    pub(super) fn new_scope(
        &mut self,
        in_function: FunctionInfoId,
        parent_scope: Option<LexicalScopeId>,
    ) -> (LexicalScopeId, &LexicalScope) {
        self.scopes.push(LexicalScope {
            parent_function: in_function,
            parent_scope,
            decls: Default::default(),
            hoisted_functions: Default::default(),
        });
        let scope_id = LexicalScopeId::new(self.scopes.len() - 1);

        self.funcs[in_function.as_usize()].scopes.push(scope_id);

        (scope_id, self.scopes.last().unwrap())
    }

    pub(super) fn new_decl_special(
        &mut self,
        scope: LexicalScopeId,
        name: Atom,
        kind: DeclKind,
        special: Special,
    ) -> DeclId {
        self.decls.push(Decl {
            name,
            kind,
            special,
            function_in_scope: false,
            scope,
        });
        let decl_id = DeclId::new(self.decls.len() - 1);
        self.scopes[scope.as_usize()].decls.push(decl_id);
        decl_id
    }
    pub(super) fn new_decl(&mut self, scope: LexicalScopeId, name: Atom, kind: DeclKind) -> DeclId {
        self.new_decl_special(scope, name, kind, Special::NotSpecial)
    }

    pub(super) fn new_global(&mut self, name: Atom, kind: DeclKind) -> DeclId {
        self.new_decl(
            self.global_scope_id()
                .expect("global scope has not been created"),
            name,
            kind,
        )
    }

    pub fn all_ident_decls(&self) -> &HashMap<NodeRc, DeclId> {
        &self.ident_decls
    }
    pub(super) fn set_ident_decl(&mut self, node: NodeRc, decl: DeclId) {
        self.ident_decls.insert(node, decl);
    }
    pub fn ident_decl(&self, node: &NodeRc) -> Option<DeclId> {
        self.ident_decls.get(node).copied()
    }

    pub fn all_node_scopes(&self) -> &HashMap<NodeRc, LexicalScopeId> {
        &self.node_scopes
    }
    pub(super) fn set_node_scope(&mut self, node: NodeRc, scope: LexicalScopeId) {
        self.node_scopes.insert(node, scope);
    }
    pub fn node_scope(&self, node: NodeRc) -> Option<LexicalScopeId> {
        self.node_scopes.get(&node).copied()
    }

    pub fn all_decls(&self) -> &[Decl] {
        self.decls.as_slice()
    }
    pub fn decl(&self, id: DeclId) -> &Decl {
        &self.decls[id.as_usize()]
    }
    pub(super) fn decl_mut(&mut self, id: DeclId) -> &mut Decl {
        &mut self.decls[id.as_usize()]
    }

    pub fn all_scopes(&self) -> &[LexicalScope] {
        self.scopes.as_slice()
    }
    pub fn scope(&self, id: LexicalScopeId) -> &LexicalScope {
        &self.scopes[id.as_usize()]
    }
    pub(super) fn scope_mut(&mut self, id: LexicalScopeId) -> &mut LexicalScope {
        &mut self.scopes[id.as_usize()]
    }

    pub fn all_functions(&self) -> &[FunctionInfo] {
        self.funcs.as_slice()
    }
    pub fn function(&self, id: FunctionInfoId) -> &FunctionInfo {
        &self.funcs[id.as_usize()]
    }
    pub(super) fn function_mut(&mut self, id: FunctionInfoId) -> &mut FunctionInfo {
        &mut self.funcs[id.as_usize()]
    }

    /// Return the id of the global scope in the context. This may seem
    /// redundant, since the ID is constant. The idea here that the global scope
    /// may not have been created yet.
    pub fn global_scope_id(&self) -> Option<LexicalScopeId> {
        if self.scopes.is_empty() {
            None
        } else {
            Some(LexicalScopeId::GLOBAL_SCOPE_ID)
        }
    }

    /// Return the id of the global function in the context. This may seem
    /// redundant, since the ID is constant. The idea here that the global
    /// function may not have been created yet.
    pub fn global_function_id(&self) -> Option<FunctionInfoId> {
        if self.funcs.is_empty() {
            None
        } else {
            Some(FunctionInfoId::GLOBAL_FUNCTION_ID)
        }
    }

    /// Return the special arguments declaration in the specified function.
    /// It may be None if "arguments" hasn't been used.
    pub fn func_arguments_opt(&self, func: FunctionInfoId) -> Option<DeclId> {
        self.function(func).arguments_decl
    }
    /// Return or create the special arguments declaration in the specified
    /// function.
    /// - `name`: the object doesn't have access to the atom table, so it has
    ///   to be passed in.
    pub(super) fn func_arguments_decl(&mut self, func: FunctionInfoId, name: Atom) -> DeclId {
        if let Some(d) = self.function(func).arguments_decl {
            return d;
        }

        let decl = self.new_decl_special(
            *self
                .function(func)
                .scopes
                .first()
                .expect("Function must contain a scope"),
            name,
            DeclKind::Var,
            Special::Arguments,
        );
        self.function_mut(func).arguments_decl = Some(decl);

        decl
    }
}
