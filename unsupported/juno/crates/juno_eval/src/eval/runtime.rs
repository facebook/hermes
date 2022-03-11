/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::addr::*;
use super::completion_record::*;
use super::environment_record::*;
use super::execution_context::*;
use super::jsobject::*;
use super::operations::*;
use super::script::*;
use crate::eval::jsvalue::{JSString, JSSymbol, JSValue};
use juno_ast::NodeRc;
use std::fmt::Display;

pub type Realm = ();

#[derive(Debug)]
pub enum ScriptOrModule {
    Script(ScriptRecord),
    Module(),
}

declare_opaque_id!(ObjectAddr);
declare_opaque_id!(EnvRecordAddr);
declare_opaque_id!(LexicalEnvAddr);

pub enum WellKnownSymbol {
    Unscopables,
}

/// Internal slots for objects.
/// Sorted alphabetically for convenience.
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum InternalSlot {
    BooleanData,
    DateValue,
    ErrorData,
    Extensible,
    HomeObject,
    NumberData,
    ParameterMap,
    Prototype,
    RegExpMatcher,
    StringData,
}

pub struct Runtime {
    objects: Vec<JSObject>,
    env_records: Vec<EnvironmentRecord>,
    contexts: Vec<ExecutionContext>,
    well_known_symbols: Box<[JSValue]>,
    global: JSValue,
}

impl Runtime {
    pub fn new() -> Self {
        let well_known_symbols =
            vec![JSValue::Symbol(JSSymbol::new_with_str("@@unscopables"))].into_boxed_slice();

        let mut run = Runtime {
            objects: Default::default(),
            env_records: Default::default(),
            contexts: Default::default(),
            well_known_symbols,
            global: JSValue::Undefined,
        };

        run.init_global();

        run
    }

    pub fn reference_error<S: Display>(&mut self, msg: S) -> CompletionRecord {
        Err(AbruptCompletion::Throw(JSValue::String(
            JSString::from_str(format!("ReferenceError: {}", msg).as_str()),
        )))
    }
    pub fn type_error<S: Display>(&mut self, msg: S) -> CompletionRecord {
        Err(AbruptCompletion::Throw(JSValue::String(
            JSString::from_str(format!("TypeError: {}", msg).as_str()),
        )))
    }

    pub fn global(&self) -> ObjectAddr {
        jsvalue_cast!(JSValue::Object, self.global)
    }

    pub fn object(&self, addr: ObjectAddr) -> &JSObject {
        &self.objects[addr.as_usize()]
    }
    pub fn object_mut(&mut self, addr: ObjectAddr) -> &mut JSObject {
        &mut self.objects[addr.as_usize()]
    }
    pub fn new_env_record(&mut self, kind: EnvironmentRecordKind) -> EnvRecordAddr {
        let newAddr = EnvRecordAddr::new(self.env_records.len());
        self.env_records.push(EnvironmentRecord::new(kind));
        newAddr
    }
    pub fn env_record(&self, addr: EnvRecordAddr) -> &EnvironmentRecord {
        &self.env_records[addr.as_usize()]
    }
    pub fn env_record_mut(&mut self, addr: EnvRecordAddr) -> &mut EnvironmentRecord {
        &mut self.env_records[addr.as_usize()]
    }

    pub fn contexts(&self) -> &[ExecutionContext] {
        &self.contexts
    }
    pub fn contexts_mut(&mut self) -> &mut Vec<ExecutionContext> {
        &mut self.contexts
    }

    pub fn well_known_symbol(&self, which: WellKnownSymbol) -> JSValue {
        self.well_known_symbols[which as usize].clone()
    }

    /// https://262.ecma-international.org/11.0/#sec-global-object
    fn init_global(&mut self) {
        let proto = JSObject::ordinary_object_create(self, JSValue::Null, None);
        let global = JSObject::ordinary_object_create(self, JSValue::Object(proto), None);
        self.global = JSValue::Object(global);
    }
}

impl Runtime {
    pub fn is_extensible(&self, oaddr: ObjectAddr) -> bool {
        (self.object(oaddr).methods.is_extensible)(self, oaddr)
    }
}

/// Operations on Objects
/// https://262.ecma-international.org/11.0/#sec-operations-on-objects
impl Runtime {
    /// https://262.ecma-international.org/11.0/#sec-makebasicobject
    pub fn make_basic_object(&mut self, internal_slots_list: &[InternalSlot]) -> ObjectAddr {
        self.objects
            .push(JSObject::new_basic_object(internal_slots_list));
        ObjectAddr::new(self.objects.len() - 1)
    }

    /// https://262.ecma-international.org/11.0/#sec-get-o-p
    pub fn get(&mut self, oaddr: ObjectAddr, p: &JSValue) -> CompletionRecord {
        debug_assert!(is_property_key(p));
        (self.object(oaddr).methods.get)(self, oaddr, p, &JSValue::Object(oaddr))
    }

    /// https://262.ecma-international.org/11.0/#sec-set-o-p-v-throw
    pub fn set(
        &mut self,
        oaddr: ObjectAddr,
        p: &JSValue,
        v: JSValue,
        throw: bool,
    ) -> CompletionRecord {
        debug_assert!(is_property_key(p));
        let success = (self.object(oaddr).methods.set)(self, oaddr, p, v, &JSValue::Object(oaddr))?;
        if success == NormalCompletion::Value(JSValue::Boolean(false)) && throw {
            self.type_error("property set error")
        } else {
            Ok(NormalCompletion::Value(JSValue::Boolean(true)))
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-createdataproperty
    pub fn create_data_property(&mut self, oaddr: ObjectAddr, p: &JSValue, v: JSValue) -> bool {
        debug_assert!(is_property_key(p));
        let new_desc = PropertyDescriptor {
            value: Some(v),
            writable: Some(true),
            enumerable: Some(true),
            configurable: Some(true),
            ..Default::default()
        };
        (self.object(oaddr).methods.define_own_property)(self, oaddr, p, &new_desc)
    }

    /// https://262.ecma-international.org/11.0/#sec-definepropertyorthrow
    pub fn define_property_or_throw(
        &mut self,
        oaddr: ObjectAddr,
        p: &JSValue,
        desc: &PropertyDescriptor,
    ) -> CompletionRecord {
        debug_assert!(is_property_key(p));
        if !(self.object(oaddr).methods.define_own_property)(self, oaddr, p, desc) {
            self.type_error("DefineProperty error")
        } else {
            Ok(NormalCompletion::Value(JSValue::Boolean(true)))
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-hasproperty
    pub fn has_property(&self, oaddr: ObjectAddr, p: &JSValue) -> bool {
        debug_assert!(is_property_key(p));
        (self.object(oaddr).methods.has_property)(self, oaddr, p)
    }

    /// https://262.ecma-international.org/11.0/#sec-hasownproperty
    pub fn has_own_property(&self, oaddr: ObjectAddr, p: &JSValue) -> bool {
        debug_assert!(is_property_key(p));
        (self.object(oaddr).methods.get_own_property)(self, oaddr, p).is_some()
    }

    /// https://262.ecma-international.org/11.0/#sec-call
    pub fn call(&mut self, f: &JSValue, v: &JSValue, args: &[JSValue]) -> CompletionRecord {
        unimplemented!()
    }
}

pub fn evaluate_program(run: &mut Runtime, ast: &NodeRc) -> CompletionRecord {
    todo!()
}
