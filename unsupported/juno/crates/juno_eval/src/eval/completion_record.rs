/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::jsvalue::*;
use std::rc::Rc;

#[derive(Debug, Clone)]
pub enum AbruptCompletion {
    Break(Option<Rc<JSString>>),
    Continue(Option<Rc<JSString>>),
    Return(JSValue),
    Throw(JSValue),
}

pub type CompletionRecord = Result<Option<JSValue>, AbruptCompletion>;

/// https://262.ecma-international.org/11.0/#sec-updateempty
fn update_empty(cr: CompletionRecord, value: JSValue) -> CompletionRecord {
    match cr {
        Ok(None) => Ok(Some(value)),
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
