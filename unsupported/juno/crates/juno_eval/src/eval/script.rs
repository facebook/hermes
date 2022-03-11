/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::completion_record::*;
use super::execution_context::*;
use super::jsvalue::*;
use super::runtime::*;
use juno_ast::NodeRc;

/// https://262.ecma-international.org/11.0/#script-record
#[derive(Debug)]
pub struct ScriptRecord {
    realm: Option<Realm>,
    environment: Option<LexicalEnvAddr>,
    ecmascript_code: NodeRc,
}

impl ScriptRecord {
    pub fn new(ast: NodeRc) -> Self {
        Self {
            realm: None,
            environment: None,
            ecmascript_code: ast,
        }
    }
}

/// https://262.ecma-international.org/11.0/#sec-globaldeclarationinstantiation
pub fn global_declaration_instantiation(_run: &mut Runtime, _ast: &NodeRc) -> CompletionRecord {
    todo!();
}

/// https://262.ecma-international.org/11.0/#sec-runtime-semantics-scriptevaluation
pub fn script_evaluation(run: &mut Runtime, ast: &NodeRc) -> CompletionRecord {
    // Let globalEnv be scriptRecord.[[Realm]].[[GlobalEnv]].

    // Let scriptContext be a new ECMAScript code execution context.
    // Set the Function of scriptContext to null.
    // Set the Realm of scriptContext to scriptRecord.[[Realm]].
    // Set the ScriptOrModule of scriptContext to scriptRecord.
    // Set the VariableEnvironment of scriptContext to globalEnv.
    // Set the LexicalEnvironment of scriptContext to globalEnv.
    let script_context =
        ExecutionContext::new_script(ScriptOrModule::Script(ScriptRecord::new(ast.clone())));

    // Suspend the currently running execution context.
    // Push scriptContext onto the execution context stack; scriptContext is now the running execution context.
    run.contexts_mut().push(script_context);

    // Let scriptBody be scriptRecord.[[ECMAScriptCode]].
    // Let result be GlobalDeclarationInstantiation(scriptBody, globalEnv).
    let result = global_declaration_instantiation(run, ast);

    // If result.[[Type]] is normal, then
    //     Set result to the result of evaluating scriptBody.
    // If result.[[Type]] is normal and result.[[Value]] is empty, then
    //     Set result to NormalCompletion(undefined).
    let result = if result.is_ok() {
        let result = evaluate_program(run, ast);
        if let Ok(NormalCompletion::Empty) = result {
            Ok(NormalCompletion::Value(JSValue::Undefined))
        } else {
            result
        }
    } else {
        result
    };

    // Suspend scriptContext and remove it from the execution context stack.
    run.contexts_mut().pop();
    // Assert: The execution context stack is not empty.
    // Resume the context that is now on the top of the execution context stack as the running execution context.
    // Return Completion(result).
    result
}
