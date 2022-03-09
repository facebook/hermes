/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::jsvalue::*;
use std::rc::Rc;

pub type JSResult = Result<Option<JSValue>, JSValue>;

pub enum CompletionRecord {
    Normal(Option<JSValue>),
    Break(Option<Rc<JSString>>),
    Continue(Option<Rc<JSString>>),
    Return(JSValue),
    Throw(JSValue),
}

impl CompletionRecord {
    /// https://262.ecma-international.org/11.0/#sec-updateempty
    fn update_empty(self, value: JSValue) -> Self {
        match self {
            CompletionRecord::Normal(None) => Self::Normal(Some(value)),
            CompletionRecord::Break(None) => {
                Self::Break(Some(jsvalue_cast!(JSValue::String, value)))
            }
            CompletionRecord::Continue(None) => {
                Self::Continue(Some(jsvalue_cast!(JSValue::String, value)))
            }
            _ => self,
        }
    }
}
