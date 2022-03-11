/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::rc::Rc;

use super::completion_record::*;
use super::environment_record::*;
use super::function::*;
use super::jsobject::*;
use super::jsvalue::*;
use super::reference::*;
use super::runtime::*;

#[derive(Debug)]
pub struct LexicalEnvironment {
    env_record: EnvRecordAddr,
    outer: Option<LexicalEnvAddr>,
}

impl LexicalEnvironment {
    pub fn new(env_record: EnvRecordAddr, outer: Option<LexicalEnvAddr>) -> Self {
        LexicalEnvironment { env_record, outer }
    }
}

impl LexicalEnvironment {
    /// https://262.ecma-international.org/11.0/#sec-getidentifierreference
    pub fn get_identifier_reference(
        run: &mut Runtime,
        lex: Option<LexicalEnvAddr>,
        name: &Rc<JSString>,
        strict: bool,
    ) -> CompletionRecord {
        match lex {
            None => {
                // 1. If lex is the value null, then
                // a. Return a value of type Reference whose base value component is undefined,
                // whose referenced name component is name,
                // and whose strict reference flag is strict.
                Ok(NormalCompletion::Reference(Reference::value(
                    JSValue::Undefined,
                    JSValue::String(name.clone()),
                    strict,
                )))
            }
            Some(lex) => {
                // 2. Let envRec be lex's EnvironmentRecord.
                let env_rec = run.lexical_env(lex).env_record;
                // 3. Let exists be ? envRec.HasBinding(name).
                let exists = (run.env_record(env_rec).methods.has_binding)(run, env_rec, name)?;
                if let NormalCompletion::Value(JSValue::Boolean(true)) = exists {
                    // 4. If exists is true, then
                    // a. Return a value of type Reference whose base value component is envRec,
                    // whose referenced name component is name,
                    // and whose strict reference flag is strict.
                    Ok(NormalCompletion::Reference(Reference::env_rec(
                        env_rec,
                        JSValue::String(name.clone()),
                        strict,
                    )))
                } else {
                    // 5. Else,
                    // a. Let outer be the value of lex's outer environment reference.
                    let outer = run.lexical_env(lex).outer;
                    // b. Return ? GetIdentifierReference(outer, name, strict).
                    Ok(Self::get_identifier_reference(run, outer, name, strict)?)
                }
            }
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-newdeclarativeenvironment
    pub fn new_declarative_environment(
        run: &mut Runtime,
        e: Option<LexicalEnvAddr>,
    ) -> LexicalEnvAddr {
        // 1. Let env be a new Lexical Environment.
        // 2. Let envRec be a new declarative Environment Record containing no bindings.
        // 3. Set env's EnvironmentRecord to envRec.
        // 4. Set the outer lexical environment reference of env to E.
        // 5. Return env.
        let env_rec = run.new_env_record(EnvironmentRecordKind::Declarative);
        run.new_lexical_env(env_rec, e)
    }

    /// https://262.ecma-international.org/11.0/#sec-newobjectenvironment
    pub fn new_object_environment(
        run: &mut Runtime,
        o: ObjectAddr,
        e: Option<LexicalEnvAddr>,
    ) -> LexicalEnvAddr {
        // 1. Let env be a new Lexical Environment.
        // 2. Let envRec be a new object Environment Record containing O as the binding object.
        let env_rec = run.new_env_record(EnvironmentRecordKind::Declarative);
        // 3. Set env's EnvironmentRecord to envRec.
        run.env_record_mut(env_rec).obj.binding_object = Some(o);
        // 4. Set the outer lexical environment reference of env to E.
        // 5. Return env.
        run.new_lexical_env(env_rec, e)
    }

    /// https://262.ecma-international.org/11.0/#sec-newfunctionenvironment
    pub fn new_function_environment(
        run: &mut Runtime,
        f: &JSValue,
        new_target: &JSValue,
    ) -> LexicalEnvAddr {
        // 1. Assert: F is an ECMAScript function.
        let f = *jsvalue_cast!(JSValue::Object, f);
        // 2. Assert: Type(newTarget) is Undefined or Object.
        // 3. Let env be a new Lexical Environment.
        // 4. Let envRec be a new function Environment Record containing no bindings.
        let env_rec = run.new_env_record(EnvironmentRecordKind::Function);
        // 5. set envrec.[[functionobject]] to f.
        run.env_record_mut(env_rec).func.function_object = JSValue::Object(f);
        if let InternalSlotValue::ThisMode(ThisMode::Lexical) = run
            .object(f)
            .get_internal_slot(InternalSlotName::ThisMode)
            .unwrap()
        {
            // 6. If F.[[ThisMode]] is lexical, set envRec.[[ThisBindingStatus]] to lexical.
            run.env_record_mut(env_rec).func.this_binding_status = ThisBindingStatus::Lexical;
        } else {
            // 7. Else, set envRec.[[ThisBindingStatus]] to uninitialized.
            run.env_record_mut(env_rec).func.this_binding_status = ThisBindingStatus::Uninitialized;
        }
        // 8. Let home be F.[[HomeObject]].
        let home = run
            .object(f)
            .get_internal_slot(InternalSlotName::HomeObject)
            .unwrap()
            .get_value();

        // 9. Set envRec.[[HomeObject]] to home.
        run.env_record_mut(env_rec).func.home_object = home.clone();

        // 10. Set envRec.[[NewTarget]] to newTarget.
        run.env_record_mut(env_rec).func.new_target = new_target.clone();

        // 11. Set env's EnvironmentRecord to envRec.
        // 12. Set the outer lexical environment reference of env to F.[[Environment]].
        // 13. Return env.
        run.new_lexical_env(
            env_rec,
            Some(
                run.object(f)
                    .get_internal_slot(InternalSlotName::Environment)
                    .unwrap()
                    .get_lexical_environment(),
            ),
        )
    }

    /// https://262.ecma-international.org/11.0/#sec-newglobalenvironment
    pub fn new_global_environment(
        run: &mut Runtime,
        g: ObjectAddr,
        this_value: JSValue,
    ) -> LexicalEnvAddr {
        // 1. Let env be a new Lexical Environment.
        // 2. Let objRec be a new object Environment Record containing G as the binding object.
        // 3. Let dclRec be a new declarative Environment Record containing no bindings.

        // 4. Let globalRec be a new global Environment Record.
        let global_rec = run.new_env_record(EnvironmentRecordKind::Global);

        // 5. Set globalRec.[[ObjectRecord]] to objRec.
        run.env_record_mut(global_rec).obj.binding_object = Some(g);

        // 6. Set globalRec.[[GlobalThisValue]] to thisValue.
        run.env_record_mut(global_rec).glob.global_this_value = this_value;

        // 7. Set globalRec.[[DeclarativeRecord]] to dclRec.
        // (Done implicitly by the ctor).

        // 8. Set globalRec.[[VarNames]] to a new empty List.
        // (Done implicitly by the ctor).

        // 9. Set env's EnvironmentRecord to globalRec.
        // 10. Set the outer lexical environment reference of env to null.
        // 11. Return env.
        run.new_lexical_env(global_rec, None)
    }
}
