/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::completion_record::CompletionRecord;
use super::completion_record::NormalCompletion;
use super::execution_context::ExecutionContext;
use super::jsvalue::JSValue;
use super::operations::to_object;
use super::runtime::EnvRecordAddr;
use super::runtime::Runtime;

#[derive(Clone, Debug, PartialEq)]
pub enum ReferenceBase {
    Value(JSValue),
    EnvRec(EnvRecordAddr),
}

#[derive(Clone, Debug, PartialEq)]
pub struct Reference {
    base: ReferenceBase,
    name: JSValue,
    strict: bool,
    this_value: Option<JSValue>,
}

impl Reference {
    /// Make a "Reference" type.
    pub fn value(base: JSValue, name: JSValue, strict: bool) -> Self {
        Reference {
            base: ReferenceBase::Value(base),
            name,
            strict,
            this_value: None,
        }
    }

    /// Make a "Reference" type with base environment record.
    pub fn env_rec(base: EnvRecordAddr, name: JSValue, strict: bool) -> Self {
        Reference {
            base: ReferenceBase::EnvRec(base),
            name,
            strict,
            this_value: None,
        }
    }
}

/// https://262.ecma-international.org/11.0/#sec-reference-specification-type
impl Reference {
    pub fn get_base(&self) -> &ReferenceBase {
        &self.base
    }
    pub fn get_referenced_name(&self) -> &JSValue {
        &self.name
    }
    pub fn is_strict_reference(&self) -> bool {
        self.strict
    }
    pub fn has_primitive_base(&self) -> bool {
        matches!(
            self.base,
            ReferenceBase::Value(JSValue::Boolean(_))
                | ReferenceBase::Value(JSValue::String(_))
                | ReferenceBase::Value(JSValue::Symbol(_))
                | ReferenceBase::Value(JSValue::BigInt(_))
                | ReferenceBase::Value(JSValue::Number(_))
        )
    }
    pub fn get_base_value(&self) -> &JSValue {
        match &self.base {
            ReferenceBase::Value(v) => v,
            _ => unreachable!(),
        }
    }
    pub fn get_base_env_record(&self) -> EnvRecordAddr {
        match self.base {
            ReferenceBase::EnvRec(r) => r,
            _ => unreachable!(),
        }
    }
    pub fn is_property_reference(&self) -> bool {
        matches!(self.base, ReferenceBase::Value(JSValue::Object(_))) || self.has_primitive_base()
    }
    pub fn is_unresolvable_reference(&self) -> bool {
        matches!(self.base, ReferenceBase::Value(JSValue::Undefined))
    }
    pub fn is_super_reference(&self) -> bool {
        self.this_value.is_some()
    }

    /// https://262.ecma-international.org/11.0/#sec-getvalue
    pub fn get_value(run: &mut Runtime, v: CompletionRecord) -> CompletionRecord {
        // 1. ReturnIfAbrupt(V).
        let v = v?;
        // 2. If Type(V) is not Reference, return V.
        let v = match v {
            NormalCompletion::Empty => panic!("get_value() value cannot be empty"),
            NormalCompletion::Value(_) => return CompletionRecord::Ok(v),
            NormalCompletion::Reference(r) => r,
        };
        // 4.
        if v.is_unresolvable_reference() {
            return run.reference_error("Unresolvable reference");
        }
        // 5.
        if v.is_property_reference() {
            let base_value = v.get_base_value();
            // 5.a.
            let oaddr = if v.has_primitive_base() {
                NormalCompletion::unwrap_value(to_object(run, base_value)?).cast_object()
            } else {
                base_value.cast_object()
            };
            (run.object(oaddr).methods.get)(run, oaddr, v.get_referenced_name(), v.get_this_value())
        } else {
            let recaddr = v.get_base_env_record();
            (run.env_record(recaddr).methods.get_binding_value)(
                run,
                recaddr,
                v.get_referenced_name().cast_string(),
                v.is_strict_reference(),
            )
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-putvalue
    pub fn put_value(
        run: &mut Runtime,
        v: CompletionRecord,
        w: CompletionRecord,
    ) -> CompletionRecord {
        let v = v?;
        let w = w?;
        let v = if let NormalCompletion::Reference(r) = v {
            r
        } else {
            return run.reference_error("Reference required");
        };
        if v.is_unresolvable_reference() {
            if v.is_strict_reference() {
                return run.reference_error("Unresolvable reference");
            }
            let global_obj = ExecutionContext::get_global_object(run);
            run.set(global_obj, v.get_referenced_name(), w.unwrap_value(), false)
        } else if v.is_property_reference() {
            let base_value = v.get_base_value();
            let oaddr = if v.has_primitive_base() {
                NormalCompletion::unwrap_value(to_object(run, base_value)?).cast_object()
            } else {
                base_value.cast_object()
            };
            let success = (run.object(oaddr).methods.set)(
                run,
                oaddr,
                v.get_referenced_name(),
                w.unwrap_value(),
                v.get_this_value(),
            )?
            .unwrap_value();
            if success == JSValue::Boolean(false) && v.is_strict_reference() {
                return run.type_error(format!("Can't set property {}", v.get_referenced_name()));
            }
            Ok(NormalCompletion::Empty)
        } else {
            let recaddr = v.get_base_env_record();
            (run.env_record(recaddr).methods.set_mutable_binding)(
                run,
                recaddr,
                v.get_referenced_name().cast_string(),
                w.unwrap_value(),
                v.is_strict_reference(),
            )
        }
    }

    pub fn get_this_value(&self) -> &JSValue {
        debug_assert!(self.is_property_reference());
        if self.is_super_reference() {
            self.this_value.as_ref().unwrap()
        } else {
            self.get_base_value()
        }
    }

    pub fn initialize_referenced_binding(
        run: &mut Runtime,
        v: CompletionRecord,
        w: CompletionRecord,
    ) -> CompletionRecord {
        let v = v?;
        let w = w?;
        let v = v.unwrap_reference();
        let recaddr = v.get_base_env_record();
        (run.env_record(recaddr).methods.initialize_binding)(
            run,
            recaddr,
            v.get_referenced_name().cast_string(),
            w.unwrap_value(),
        )
    }
}
