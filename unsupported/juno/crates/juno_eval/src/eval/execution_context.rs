/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::runtime::*;

/// https://262.ecma-international.org/11.0/#table-22
#[derive(Debug)]
pub struct ExecutionContext {
    /// The currently executing function, or none if executing script/module.
    pub function: Option<ObjectAddr>,
    pub realm: Realm,
    pub script_or_module: Option<ScriptOrModule>,

    lex_env: LexicalEnvAddr,
    var_env: EnvRecordAddr,
}

impl ExecutionContext {
    pub fn new_script(_script_or_module: ScriptOrModule) -> Self {
        unimplemented!();
        // ExecutionContext {
        //     function: None,
        //     realm: (),
        //     script_or_module: Some(script_or_module),
        //
        //     lex_env: todo!(),
        //     var_env: todo!(),
        // }
    }
}

impl ExecutionContext {
    /// https://262.ecma-international.org/11.0/#sec-getactivescriptormodule
    /// "Return null" here will return `None`.
    pub fn get_active_script_or_module(run: &Runtime) -> Option<&ScriptOrModule> {
        // 1. If the execution context stack is empty, return null.
        if run.contexts().is_empty() {
            return None;
        }

        // 2. Let ec be the topmost execution context on the execution context stack
        //   whose ScriptOrModule component is not null.
        // 3. If no such execution context exists, return null.
        //   Otherwise, return ec's ScriptOrModule.
        run.contexts()
            .iter()
            .rev()
            .find_map(|c| c.script_or_module.as_ref())
    }

    /// https://262.ecma-international.org/11.0/#sec-getglobalobject
    pub fn get_global_object(run: &Runtime) -> ObjectAddr {
        run.global()
    }
}
