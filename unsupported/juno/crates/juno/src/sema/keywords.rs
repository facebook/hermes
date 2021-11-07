/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::ast::{Atom, AtomTable};

pub(super) struct Keywords {
    pub ident_arguments: Atom,
    pub ident_eval: Atom,
    pub ident_let: Atom,
    pub ident_new: Atom,
    pub ident_target: Atom,
}

impl Keywords {
    pub fn new(tab: &AtomTable) -> Self {
        Keywords {
            ident_arguments: tab.atom("arguments"),
            ident_eval: tab.atom("eval"),
            ident_let: tab.atom("let"),
            ident_new: tab.atom("new"),
            ident_target: tab.atom("target"),
        }
    }
}
