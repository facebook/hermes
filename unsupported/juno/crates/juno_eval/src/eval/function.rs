/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::completion_record::*;
use super::jsobject::*;
use super::runtime::*;

#[derive(Debug, PartialEq, Eq, Hash, Clone, Copy)]
pub enum ConstructorKind {
    Base,
    Derived,
}

#[derive(Debug, PartialEq, Eq, Hash, Clone, Copy)]
pub enum ThisMode {
    Lexical,
    Strict,
    Global,
}

pub struct FunctionMethods {
    pub call: fn() -> CompletionRecord,
    pub construct: Option<fn() -> CompletionRecord>,
}

pub struct FunctionSlots {
    pub env: EnvRecordAddr,
    pub constructor_kind: ConstructorKind,
    pub realm: Realm,
    pub this_mode: ThisMode,
    pub strict: bool,
    pub home_object: Option<ObjectAddr>,
    pub is_class_constructor: bool,
}

pub struct JSFunction {
    methods: &'static FunctionMethods,
    slots: FunctionSlots,
}

impl JSObject {
    pub fn call_impl() -> CompletionRecord {
        unimplemented!()
    }
}
