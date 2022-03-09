/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::addr::*;
use super::completion_record::*;
use super::environment_record::*;
use super::jsobject::*;
use super::operations::*;
use crate::eval::jsvalue::{JSString, JSSymbol, JSValue};
use std::fmt::Display;

pub type Realm = ();
pub type ScriptOrModule = ();

pub enum WellKnownSymbol {
    Unscopables,
}

/// https://262.ecma-international.org/11.0/#table-22
pub struct ExecutionContext {
    /// The currently executing function, or none if executing script/module.
    pub function: Option<ObjectAddr>,
    pub realm: Realm,
    pub script_or_module: Option<ScriptOrModule>,

    lex_env: EnvRecordAddr,
    var_env: EnvRecordAddr,
}

pub struct Runtime {
    objects: Vec<JSObject>,
    env_records: Vec<EnvironmentRecord>,
    contexts: Vec<ExecutionContext>,
    well_known_symbols: Box<[JSValue]>,
}

impl Runtime {
    pub fn new() -> Self {
        let well_known_symbols =
            vec![JSValue::Symbol(JSSymbol::new_with_str("@@unscopables"))].into_boxed_slice();

        Runtime {
            objects: Default::default(),
            env_records: Default::default(),
            contexts: Default::default(),
            well_known_symbols,
        }
    }

    pub fn reference_error<S: Display>(&mut self, msg: S) -> JSResult {
        Ok(Some(JSValue::String(JSString::from_str(
            format!("ReferenceError: {}", msg).as_str(),
        ))))
    }
    pub fn type_error<S: Display>(&mut self, msg: S) -> JSResult {
        Ok(Some(JSValue::String(JSString::from_str(
            format!("TypeError: {}", msg).as_str(),
        ))))
    }

    pub fn object(&self, addr: ObjectAddr) -> &JSObject {
        &self.objects[addr.as_usize()]
    }
    pub fn object_mut(&mut self, addr: ObjectAddr) -> &mut JSObject {
        &mut self.objects[addr.as_usize()]
    }
    pub fn env_record(&self, addr: EnvRecordAddr) -> &EnvironmentRecord {
        &self.env_records[addr.as_usize()]
    }
    pub fn env_record_mut(&mut self, addr: EnvRecordAddr) -> &mut EnvironmentRecord {
        &mut self.env_records[addr.as_usize()]
    }

    pub fn well_known_symbol(&self, which: WellKnownSymbol) -> JSValue {
        self.well_known_symbols[which as usize].clone()
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
    /// https://262.ecma-international.org/11.0/#sec-get-o-p
    pub fn get(&mut self, oaddr: ObjectAddr, p: &JSValue) -> JSResult {
        debug_assert!(is_property_key(p));
        (self.object(oaddr).methods.get)(self, oaddr, p, &JSValue::Object(oaddr))
    }

    /// https://262.ecma-international.org/11.0/#sec-set-o-p-v-throw
    pub fn set(&mut self, oaddr: ObjectAddr, p: &JSValue, v: JSValue, throw: bool) -> JSResult {
        debug_assert!(is_property_key(p));
        let success = (self.object(oaddr).methods.set)(self, oaddr, p, v, &JSValue::Object(oaddr))?;
        if success == Some(JSValue::Boolean(false)) && throw {
            self.type_error("property set error")
        } else {
            Ok(Some(JSValue::Boolean(true)))
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
    ) -> JSResult {
        debug_assert!(is_property_key(p));
        if !(self.object(oaddr).methods.define_own_property)(self, oaddr, p, desc) {
            self.type_error("DefineProperty error")
        } else {
            Ok(Some(JSValue::Boolean(true)))
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
    pub fn call(&mut self, f: &JSValue, v: &JSValue, args: &[JSValue]) -> JSResult {
        unimplemented!()
    }
}
