/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::rc::Rc;

use super::jsvalue::*;
use super::reference::Reference;

#[derive(Debug, Clone)]
pub enum AbruptCompletion {
    Break(Option<Rc<JSString>>),
    Continue(Option<Rc<JSString>>),
    Return(JSValue),
    Throw(JSValue),
}

#[derive(Debug, Clone, PartialEq)]
pub enum NormalCompletion {
    Empty,
    Value(JSValue),
    Reference(Reference),
}

pub type CompletionRecord = Result<NormalCompletion, AbruptCompletion>;

/// https://262.ecma-international.org/11.0/#sec-updateempty
fn update_empty(cr: CompletionRecord, value: JSValue) -> CompletionRecord {
    match cr {
        Ok(NormalCompletion::Empty) => Ok(NormalCompletion::Value(value)),
        Err(AbruptCompletion::Break(None)) => Err(AbruptCompletion::Break(Some(jsvalue_cast!(
            JSValue::String,
            value
        )))),
        Err(AbruptCompletion::Continue(None)) => Err(AbruptCompletion::Continue(Some(
            jsvalue_cast!(JSValue::String, value),
        ))),
        _ => cr,
    }
}

impl NormalCompletion {
    pub fn unwrap_value(self) -> JSValue {
        if let NormalCompletion::Value(val) = self {
            return val;
        }
        panic!("Attempting to unwrap non-value.");
    }
    pub fn unwrap_reference(self) -> Reference {
        if let NormalCompletion::Reference(r) = self {
            return r;
        }
        panic!("Attempting to unwrap non-reference.");
    }
}
