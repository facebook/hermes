/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::rc::Rc;

use super::completion_record::*;
use super::jsvalue::*;
use super::runtime::*;
use crate::eval::jsobject::PropertyDescriptor;
use crate::eval::operations::to_boolean;

#[derive(Debug)]
pub struct Binding {
    pub mutable: bool,
    pub deletable: bool,
    pub strict: bool,
    pub value: Option<JSValue>,
}

#[derive(Debug)]
pub struct DeclarativeEnv {
    pub names: Vec<Rc<JSString>>,
    pub bindings: Vec<Binding>,
}

#[derive(Debug)]
pub struct ObjectEnv {
    pub binding_object: Option<ObjectAddr>,
    pub with_environment: bool,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ThisBindingStatus {
    Lexical,
    Initialized,
    Uninitialized,
}

#[derive(Debug)]
pub struct FunctionEnv {
    pub this_value: JSValue,
    pub this_binding_status: ThisBindingStatus,
    pub function_object: JSValue,
    pub home_object: JSValue,
    pub new_target: JSValue,
}

#[derive(Debug)]
pub struct GlobalEnv {
    pub global_this_value: JSValue,
    pub var_names: Vec<Rc<JSString>>,
}

pub struct EnvironmentMethods {
    pub has_binding: fn(&mut Runtime, EnvRecordAddr, &Rc<JSString>) -> CompletionRecord,
    pub create_mutable_binding:
        fn(&mut Runtime, EnvRecordAddr, Rc<JSString>, bool) -> CompletionRecord,
    pub create_immutable_binding:
        fn(&mut Runtime, EnvRecordAddr, Rc<JSString>, bool) -> CompletionRecord,
    pub initialize_binding:
        fn(&mut Runtime, EnvRecordAddr, &Rc<JSString>, JSValue) -> CompletionRecord,
    pub set_mutable_binding:
        fn(&mut Runtime, EnvRecordAddr, &Rc<JSString>, JSValue, bool) -> CompletionRecord,
    pub get_binding_value: fn(&mut Runtime, EnvRecordAddr, &Rc<JSString>, bool) -> CompletionRecord,
    pub get_this_binding: fn(&mut Runtime, EnvRecordAddr) -> CompletionRecord,
    pub delete_binding: fn(&mut Runtime, EnvRecordAddr, &Rc<JSString>) -> CompletionRecord,
    pub has_this_binding: fn(&Runtime, EnvRecordAddr) -> bool,
    pub has_super_binding: fn(&Runtime, EnvRecordAddr) -> bool,
    pub with_base_object: fn(&Runtime, EnvRecordAddr) -> Option<ObjectAddr>,
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum EnvironmentRecordKind {
    /// Declarative
    Declarative,
    /// Declarative + Function
    Function,
    /// Object
    Object,
    /// Declarative + Object
    Global,
}

// declarative_record: DeclarativeEnv,
pub struct EnvironmentRecord {
    pub methods: &'static EnvironmentMethods,
    /// This field used for debugging.
    pub kind: EnvironmentRecordKind,

    pub decl: DeclarativeEnv,
    pub func: FunctionEnv,
    pub obj: ObjectEnv,
    pub glob: GlobalEnv,
}

impl EnvironmentRecord {
    pub fn new(kind: EnvironmentRecordKind) -> EnvironmentRecord {
        EnvironmentRecord {
            methods: match kind {
                EnvironmentRecordKind::Declarative => &DECL_ENV_METHODS,
                EnvironmentRecordKind::Function => &FUNCTION_ENV_METHODS,
                EnvironmentRecordKind::Object => &OBJECT_ENV_METHODS,
                EnvironmentRecordKind::Global => &GLOBAL_ENV_METHODS,
            },
            kind,
            decl: DeclarativeEnv {
                names: Vec::new(),
                bindings: Vec::new(),
            },
            func: FunctionEnv {
                this_value: JSValue::Undefined,
                this_binding_status: ThisBindingStatus::Uninitialized,
                function_object: JSValue::Undefined,
                home_object: JSValue::Undefined,
                new_target: JSValue::Undefined,
            },
            obj: ObjectEnv {
                binding_object: None,
                with_environment: false,
            },
            glob: GlobalEnv {
                global_this_value: JSValue::Undefined,
                var_names: Vec::new(),
            },
        }
    }
}

fn unreachable_get_this_binding(_run: &mut Runtime, _eaddr: EnvRecordAddr) -> CompletionRecord {
    unimplemented!()
}

static DECL_ENV_METHODS: EnvironmentMethods = EnvironmentMethods {
    has_binding: DeclarativeEnv::has_binding,
    create_mutable_binding: DeclarativeEnv::create_mutable_binding,
    create_immutable_binding: DeclarativeEnv::create_immutable_binding,
    initialize_binding: DeclarativeEnv::initialize_binding,
    set_mutable_binding: DeclarativeEnv::set_mutable_binding,
    get_binding_value: DeclarativeEnv::get_binding_value,
    get_this_binding: unreachable_get_this_binding,
    delete_binding: DeclarativeEnv::delete_binding,
    has_this_binding: DeclarativeEnv::has_this_binding,
    has_super_binding: DeclarativeEnv::has_super_binding,
    with_base_object: DeclarativeEnv::with_base_object,
};

static OBJECT_ENV_METHODS: EnvironmentMethods = EnvironmentMethods {
    has_binding: ObjectEnv::has_binding,
    create_mutable_binding: ObjectEnv::create_mutable_binding,
    create_immutable_binding: ObjectEnv::create_immutable_binding,
    initialize_binding: ObjectEnv::initialize_binding,
    set_mutable_binding: ObjectEnv::set_mutable_binding,
    get_binding_value: ObjectEnv::get_binding_value,
    get_this_binding: unreachable_get_this_binding,
    delete_binding: ObjectEnv::delete_binding,
    has_this_binding: ObjectEnv::has_this_binding,
    has_super_binding: ObjectEnv::has_super_binding,
    with_base_object: ObjectEnv::with_base_object,
};

static FUNCTION_ENV_METHODS: EnvironmentMethods = EnvironmentMethods {
    has_binding: DeclarativeEnv::has_binding,
    create_mutable_binding: DeclarativeEnv::create_mutable_binding,
    create_immutable_binding: DeclarativeEnv::create_immutable_binding,
    initialize_binding: DeclarativeEnv::initialize_binding,
    set_mutable_binding: DeclarativeEnv::set_mutable_binding,
    get_binding_value: DeclarativeEnv::get_binding_value,
    get_this_binding: FunctionEnv::get_this_binding,
    delete_binding: DeclarativeEnv::delete_binding,
    has_this_binding: FunctionEnv::has_this_binding,
    has_super_binding: FunctionEnv::has_super_binding,
    with_base_object: DeclarativeEnv::with_base_object,
};

static GLOBAL_ENV_METHODS: EnvironmentMethods = EnvironmentMethods {
    has_binding: GlobalEnv::has_binding,
    create_mutable_binding: GlobalEnv::create_mutable_binding,
    create_immutable_binding: GlobalEnv::create_immutable_binding,
    initialize_binding: GlobalEnv::initialize_binding,
    set_mutable_binding: GlobalEnv::set_mutable_binding,
    get_binding_value: GlobalEnv::get_binding_value,
    get_this_binding: GlobalEnv::get_this_binding,
    delete_binding: GlobalEnv::delete_binding,
    has_this_binding: GlobalEnv::has_this_binding,
    has_super_binding: GlobalEnv::has_super_binding,
    with_base_object: GlobalEnv::with_base_object,
};

impl DeclarativeEnv {
    fn find_binding(&self, name: &JSString) -> Option<usize> {
        self.names.iter().position(|v| **v == *name)
    }

    fn binding(&self, index: usize) -> &Binding {
        &self.bindings[index]
    }
    fn binding_mut(&mut self, index: usize) -> &mut Binding {
        &mut self.bindings[index]
    }

    fn create_binding(
        &mut self,
        name: Rc<JSString>,
        mutable: bool,
        deletable: bool,
        strict: bool,
    ) -> usize {
        debug_assert!(self.find_binding(&name).is_none());
        self.names.push(name);
        self.bindings.push(Binding {
            mutable,
            deletable,
            strict,
            value: None,
        });
        self.bindings.len() - 1
    }

    fn initialize_binding_impl(&mut self, index: usize, value: JSValue) {
        debug_assert!(
            self.bindings[index].value.is_none(),
            "binding must be uninitialized"
        );
        self.bindings[index].value = Some(value);
    }

    fn remove_binding(&mut self, index: usize) {
        self.names.remove(index);
        self.bindings.remove(index);
    }
}

//noinspection RsSelfConvention
impl DeclarativeEnv {
    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-hasbinding-n
    fn has_binding(run: &mut Runtime, eaddr: EnvRecordAddr, n: &Rc<JSString>) -> CompletionRecord {
        Ok(NormalCompletion::Value(JSValue::Boolean(
            run.env_record(eaddr).decl.find_binding(n).is_some(),
        )))
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-createmutablebinding-n-d
    fn create_mutable_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: Rc<JSString>,
        deletable: bool,
    ) -> CompletionRecord {
        run.env_record_mut(eaddr)
            .decl
            .create_binding(name, true, deletable, false);
        Ok(NormalCompletion::Value(JSValue::Boolean(true)))
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-createimmutablebinding-n-s
    fn create_immutable_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: Rc<JSString>,
        strict: bool,
    ) -> CompletionRecord {
        run.env_record_mut(eaddr)
            .decl
            .create_binding(name, false, false, strict);
        Ok(NormalCompletion::Value(JSValue::Boolean(true)))
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-initializebinding-n-v
    fn initialize_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        v: JSValue,
    ) -> CompletionRecord {
        let dcl_rec = &mut run.env_record_mut(eaddr).decl;
        dcl_rec.initialize_binding_impl(dcl_rec.find_binding(name).expect("binding must exist"), v);
        Ok(NormalCompletion::Empty)
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-setmutablebinding-n-v-s
    pub fn set_mutable_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        value: JSValue,
        mut strict: bool,
    ) -> CompletionRecord {
        let dcl_rec = &mut run.env_record_mut(eaddr).decl;
        let bindex = dcl_rec.find_binding(name);
        if bindex.is_none() {
            if strict {
                return run.reference_error(format!("undefined variable '{}'", name));
            }
            let index = dcl_rec.create_binding(name.clone(), true, true, false);
            dcl_rec.initialize_binding_impl(index, value);
            return Ok(NormalCompletion::Empty);
        }
        let index = bindex.unwrap();
        let binding = dcl_rec.binding_mut(index);
        if binding.strict {
            strict = true;
        }
        if binding.value.is_none() {
            return run.reference_error(format!("uninitialized variable '{}'", name));
        }
        if binding.mutable {
            binding.value = Some(value);
        } else if strict {
            return run.reference_error(format!(
                "attempting to modify immutable variable '{}'",
                name
            ));
        }

        Ok(NormalCompletion::Empty)
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-getbindingvalue-n-s
    pub fn get_binding_value(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        _strict: bool,
    ) -> CompletionRecord {
        let dcl_rec = &run.env_record(eaddr).decl;
        let index = dcl_rec.find_binding(&*name).expect("binding must exist");
        let binding = dcl_rec.binding(index);
        if let Some(v) = &binding.value {
            Ok(NormalCompletion::Value(v.clone()))
        } else {
            return run.reference_error(format!("uninitialized variable '{}'", name));
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-deletebinding-n
    fn delete_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
    ) -> CompletionRecord {
        let dcl_rec = &mut run.env_record_mut(eaddr).decl;
        let index = dcl_rec.find_binding(name).expect("binding must exist");
        if !dcl_rec.binding(index).deletable {
            return Ok(NormalCompletion::Value(JSValue::Boolean(false)));
        }
        dcl_rec.remove_binding(index);
        Ok(NormalCompletion::Value(JSValue::Boolean(true)))
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-hasthisbinding
    fn has_this_binding(_run: &Runtime, _eaddr: EnvRecordAddr) -> bool {
        false
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-hassuperbinding
    fn has_super_binding(_run: &Runtime, _eaddr: EnvRecordAddr) -> bool {
        false
    }

    /// https://262.ecma-international.org/11.0/#sec-declarative-environment-records-withbaseobject
    fn with_base_object(_run: &Runtime, _eaddr: EnvRecordAddr) -> Option<ObjectAddr> {
        None
    }
}

impl FunctionEnv {
    /// https://262.ecma-international.org/11.0/#sec-bindthisvalue
    fn bind_this_value(run: &mut Runtime, eaddr: EnvRecordAddr, v: JSValue) -> CompletionRecord {
        // 1. Let envRec be the function Environment Record for which the method was invoked.
        // 2. Assert: envRec.[[ThisBindingStatus]] is not lexical.
        let env_rec = run.env_record_mut(eaddr);
        // 3. If envRec.[[ThisBindingStatus]] is initialized, throw a ReferenceError exception.
        if env_rec.func.this_binding_status == ThisBindingStatus::Initialized {
            return run.reference_error("this binding is already initialized");
        }
        // 4. Set envRec.[[ThisValue]] to V.
        env_rec.func.this_value = v.clone();
        // 5. Set envRec.[[ThisBindingStatus]] to initialized.
        env_rec.func.this_binding_status = ThisBindingStatus::Initialized;
        // 6. Return V.
        Ok(NormalCompletion::Value(v))
    }

    /// https://262.ecma-international.org/11.0/#sec-getsuperbase
    fn get_super_base(run: &mut Runtime, eaddr: EnvRecordAddr) -> CompletionRecord {
        // 1. Let envRec be the function Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. Let home be envRec.[[HomeObject]].
        let home = &env_rec.func.home_object;
        match home {
            // 3. If home has the value undefined, return undefined.
            JSValue::Undefined => Ok(NormalCompletion::Value(JSValue::Undefined)),
            // 4. Assert: Type(home) is Object.
            // 5. Return ? home.[[GetPrototypeOf]]().
            JSValue::Object(obj_addr) => {
                let obj = run.object(*obj_addr);
                let proto = (obj.methods.get_prototype_of)(run, *obj_addr);
                Ok(NormalCompletion::Value(proto.clone()))
            }
            _ => unreachable!(),
        }
    }
}

impl FunctionEnv {
    /// https://262.ecma-international.org/11.0/#sec-function-environment-records-hasthisbinding
    fn has_this_binding(run: &Runtime, eaddr: EnvRecordAddr) -> bool {
        // 1. Let envRec be the function Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. If envRec.[[ThisBindingStatus]] is lexical, return false; otherwise, return true.
        env_rec.func.this_binding_status != ThisBindingStatus::Lexical
    }

    /// https://262.ecma-international.org/11.0/#sec-function-environment-records-hassuperbinding
    fn has_super_binding(run: &Runtime, eaddr: EnvRecordAddr) -> bool {
        // 1. Let envRec be the function Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. If envRec.[[ThisBindingStatus]] is lexical, return false.
        if env_rec.func.this_binding_status == ThisBindingStatus::Lexical {
            return false;
        }
        // 3. If envRec.[[HomeObject]] has the value undefined, return false; otherwise, return true.
        !matches!(env_rec.func.home_object, JSValue::Undefined)
    }

    /// https://262.ecma-international.org/11.0/#sec-function-environment-records-getthisbinding
    fn get_this_binding(run: &mut Runtime, eaddr: EnvRecordAddr) -> CompletionRecord {
        // 1. Let envRec be the function Environment Record for which the method was invoked.
        // 2. Assert: envRec.[[ThisBindingStatus]] is not lexical.
        let env_rec = run.env_record_mut(eaddr);
        // 3. If envRec.[[ThisBindingStatus]] is uninitialized, throw a ReferenceError exception.
        let binding_status = env_rec.func.this_binding_status;
        if binding_status == ThisBindingStatus::Uninitialized {
            return run.reference_error("this binding is uninitialized");
        }
        // 4. Return envRec.[[ThisValue]].
        Ok(NormalCompletion::Value(env_rec.func.this_value.clone()))
    }
}

impl ObjectEnv {
    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-hasbinding-n
    fn has_binding(run: &mut Runtime, eaddr: EnvRecordAddr, n: &Rc<JSString>) -> CompletionRecord {
        let env_rec = run.env_record(eaddr);
        let nv = JSValue::String(n.clone());
        if !run.has_property(env_rec.obj.binding_object.unwrap(), &nv) {
            return Ok(NormalCompletion::Value(JSValue::Boolean(false)));
        }
        if !env_rec.obj.with_environment {
            return Ok(NormalCompletion::Value(JSValue::Boolean(true)));
        }
        let unscopables_sym = run.well_known_symbol(WellKnownSymbol::Unscopables);
        let binding_object_addr = env_rec.obj.binding_object.unwrap();
        let unscopables = run
            .get(binding_object_addr, &unscopables_sym)?
            .unwrap_value();
        if let JSValue::Object(unsc_addr) = unscopables {
            let blocked = to_boolean(&run.get(unsc_addr, &nv)?.unwrap_value());
            if blocked {
                return Ok(NormalCompletion::Value(JSValue::Boolean(false)));
            }
        }
        Ok(NormalCompletion::Value(JSValue::Boolean(true)))
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-createmutablebinding-n-d
    fn create_mutable_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: Rc<JSString>,
        deletable: bool,
    ) -> CompletionRecord {
        let bindings_addr = run.env_record(eaddr).obj.binding_object.unwrap();
        run.define_property_or_throw(
            bindings_addr,
            &JSValue::String(name),
            &PropertyDescriptor {
                value: Some(JSValue::Undefined),
                writable: Some(true),
                enumerable: Some(true),
                configurable: Some(deletable),
                ..Default::default()
            },
        )
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-createimmutablebinding-n-s
    fn create_immutable_binding(
        _run: &mut Runtime,
        _eaddr: EnvRecordAddr,
        _name: Rc<JSString>,
        _strict: bool,
    ) -> CompletionRecord {
        panic!("ObjectEnv::create_immutable_binding should never be used")
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-initializebinding-n-v
    fn initialize_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        v: JSValue,
    ) -> CompletionRecord {
        Self::set_mutable_binding(run, eaddr, name, v, false)
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-setmutablebinding-n-v-s
    //noinspection RsSelfConvention
    fn set_mutable_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        v: JSValue,
        strict: bool,
    ) -> CompletionRecord {
        let bindings_addr = run.env_record(eaddr).obj.binding_object.unwrap();
        run.set(bindings_addr, &JSValue::String(name.clone()), v, strict)
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-getbindingvalue-n-s
    //noinspection RsSelfConvention
    fn get_binding_value(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        strict: bool,
    ) -> CompletionRecord {
        let bindings_addr = run.env_record(eaddr).obj.binding_object.unwrap();
        let p = JSValue::String(name.clone());
        if !run.has_property(bindings_addr, &p) {
            if !strict {
                Ok(NormalCompletion::Value(JSValue::Undefined))
            } else {
                run.reference_error(format!("{} is not defined", &p))
            }
        } else {
            run.get(bindings_addr, &p)
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-deletebinding-n
    fn delete_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
    ) -> CompletionRecord {
        let bindings_addr = run.env_record(eaddr).obj.binding_object.unwrap();
        let p = JSValue::String(name.clone());
        Ok(NormalCompletion::Value(JSValue::Boolean((run
            .object(bindings_addr)
            .methods
            .delete)(
            run,
            bindings_addr,
            &p,
        ))))
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-hasthisbinding
    fn has_this_binding(_run: &Runtime, _eaddr: EnvRecordAddr) -> bool {
        false
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-hassuperbinding
    fn has_super_binding(_run: &Runtime, _eaddr: EnvRecordAddr) -> bool {
        false
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-withbaseobject
    fn with_base_object(run: &Runtime, eaddr: EnvRecordAddr) -> Option<ObjectAddr> {
        let env_rec = run.env_record(eaddr);
        if env_rec.obj.with_environment {
            Some(env_rec.obj.binding_object.unwrap())
        } else {
            None
        }
    }
}

impl GlobalEnv {
    /// https://262.ecma-international.org/11.0/#sec-hasvardeclaration
    fn has_var_declaration(run: &Runtime, eaddr: EnvRecordAddr, n: &Rc<JSString>) -> bool {
        // 1. Let envRec be the global Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. Let varDeclaredNames be envRec.[[VarNames]].
        // 3. If varDeclaredNames contains N, return true.
        // 4. Return false.
        env_rec.glob.var_names.contains(n)
    }

    /// https://262.ecma-international.org/11.0/#sec-haslexicaldeclaration
    fn has_lexical_declaration(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        n: &Rc<JSString>,
    ) -> CompletionRecord {
        // 1. Let envRec be the global Environment Record for which the method was invoked.
        // 2. Let DclRec be envRec.[[DeclarativeRecord]].
        // 3. Return DclRec.HasBinding(N).
        DeclarativeEnv::has_binding(run, eaddr, n)
    }

    /// https://262.ecma-international.org/11.0/#sec-hasrestrictedglobalproperty
    fn has_restricted_global_property(
        run: &Runtime,
        eaddr: EnvRecordAddr,
        n: &Rc<JSString>,
    ) -> bool {
        // 1. Let envRec be the global Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. Let ObjRec be envRec.[[ObjectRecord]].
        // 3. Let globalObject be the binding object for ObjRec.
        let global_obj_addr = env_rec.obj.binding_object.unwrap();
        let global_obj = run.object(global_obj_addr);
        // 4. Let existingProp be ? globalObject.[[GetOwnProperty]](N).
        let prop = JSValue::String(n.clone());
        let existing_prop = (global_obj.methods.get_own_property)(run, global_obj_addr, &prop);
        #[allow(clippy::match_like_matches_macro)]
        match existing_prop {
            // 5. If existingProp is undefined, return false.
            // 6. If existingProp.[[Configurable]] is true, return false.
            None
            | Some(PropertyDescriptor {
                configurable: Some(true),
                ..
            }) => false,
            // 7. Return true.
            _ => true,
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-candeclareglobalvar
    fn can_declare_global_var(run: &Runtime, eaddr: EnvRecordAddr, n: &Rc<JSString>) -> bool {
        // 1. Let envRec be the global Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. Let ObjRec be envRec.[[ObjectRecord]].
        // 3. Let globalObject be the binding object for ObjRec.
        let global_obj_addr = env_rec.obj.binding_object.unwrap();
        let prop = JSValue::String(n.clone());
        // 4. Let hasProperty be ? HasOwnProperty(globalObject, N).
        // 5. If hasProperty is true, return true.
        if run.has_own_property(global_obj_addr, &prop) {
            return true;
        }
        // 6. Return ? IsExtensible(globalObject).
        run.is_extensible(global_obj_addr)
    }

    /// https://262.ecma-international.org/11.0/#sec-candeclareglobalfunction
    fn can_declare_global_function(run: &Runtime, eaddr: EnvRecordAddr, n: &Rc<JSString>) -> bool {
        // 1. Let envRec be the global Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. Let ObjRec be envRec.[[ObjectRecord]].
        // 3. Let globalObject be the binding object for ObjRec.
        let global_obj_addr = env_rec.obj.binding_object.unwrap();
        let global_obj = run.object(global_obj_addr);
        let prop = JSValue::String(n.clone());
        // 4. Let existingProp be ? globalObject.[[GetOwnProperty]](N).
        let existing_prop = (global_obj.methods.get_own_property)(run, global_obj_addr, &prop);
        match existing_prop {
            // 5. If existingProp is undefined, return ? IsExtensible(globalObject).
            None => run.is_extensible(global_obj_addr),
            // 6. If existingProp.[[Configurable]] is true, return true.
            Some(PropertyDescriptor {
                configurable: Some(true),
                ..
            }) => true,
            // 7. If IsDataDescriptor(existingProp) is true and existingProp has attribute values { [[Writable]]: true,
            // [[Enumerable]]: true }, return true.
            Some(
                desc @ PropertyDescriptor {
                    writable: Some(true),
                    ..
                },
            ) => desc.is_data_descriptor(),
            // 8. Return false.
            _ => false,
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-createglobalvarbinding
    fn create_global_var_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        n: &Rc<JSString>,
        d: bool,
    ) -> CompletionRecord {
        // 1. Let envRec be the global Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. Let ObjRec be envRec.[[ObjectRecord]].
        // 3. Let globalObject be the binding object for ObjRec.
        let global_obj_addr = env_rec.obj.binding_object.unwrap();
        let prop = JSValue::String(n.clone());
        // 4. Let hasProperty be ? HasOwnProperty(globalObject, N).
        let has_property = run.has_own_property(global_obj_addr, &prop);
        // 5. Let extensible be ? IsExtensible(globalObject).
        let extensible = run.is_extensible(global_obj_addr);
        // 6. If hasProperty is false and extensible is true, then
        // a. Perform ? ObjRec.CreateMutableBinding(N, D).
        // b. Perform ? ObjRec.InitializeBinding(N, undefined).
        if !has_property && extensible {
            ObjectEnv::create_mutable_binding(run, eaddr, n.clone(), d)?;
            ObjectEnv::initialize_binding(run, eaddr, n, JSValue::Undefined)?;
        }
        // 7. Let varDeclaredNames be envRec.[[VarNames]].
        let var_names = &mut run.env_record_mut(eaddr).glob.var_names;
        // 8. If varDeclaredNames does not contain N, then
        if !var_names.contains(n) {
            // a. Append N to varDeclaredNames.
            var_names.push(n.clone())
        }
        // 9. Return NormalCompletion(empty).
        Ok(NormalCompletion::Empty)
    }

    /// https://262.ecma-international.org/11.0/#sec-createglobalfunctionbinding
    fn create_global_function_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        n: &Rc<JSString>,
        v: &JSValue,
        d: bool,
    ) -> CompletionRecord {
        // 1. Let envRec be the global Environment Record for which the method was invoked.
        let env_rec = run.env_record(eaddr);
        // 2. Let ObjRec be envRec.[[ObjectRecord]].
        // 3. Let globalObject be the binding object for ObjRec.
        let global_obj_addr = env_rec.obj.binding_object.unwrap();
        let global_obj = run.object(global_obj_addr);
        let prop = JSValue::String(n.clone());
        // 4. Let existingProp be ? globalObject.[[GetOwnProperty]](N).
        let existing_prop = (global_obj.methods.get_own_property)(run, global_obj_addr, &prop);
        let desc = match existing_prop {
            // 5. If existingProp is undefined or existingProp.[[Configurable]] is true, then
            // a. Let desc be the PropertyDescriptor { [[Value]]: V, [[Writable]]: true, [[Enumerable]]: true, [[Configurable]]: D }.
            None
            | Some(PropertyDescriptor {
                configurable: Some(true),
                ..
            }) => PropertyDescriptor {
                value: Some(v.clone()),
                writable: Some(true),
                enumerable: Some(true),
                configurable: Some(d),
                ..Default::default()
            },
            // 6. Else,
            // a. Let desc be the PropertyDescriptor { [[Value]]: V }.
            _ => PropertyDescriptor {
                value: Some(v.clone()),
                ..Default::default()
            },
        };
        // 7. Perform ? DefinePropertyOrThrow(globalObject, N, desc).
        // 8. Record that the binding for N in ObjRec has been initialized.
        run.define_property_or_throw(global_obj_addr, &prop, &desc)?;
        // 9. Perform ? Set(globalObject, N, V, false).
        run.set(global_obj_addr, &prop, v.clone(), false)?;
        // 10. Let varDeclaredNames be envRec.[[VarNames]].
        let var_names = &mut run.env_record_mut(eaddr).glob.var_names;
        // 11. If varDeclaredNames does not contain N, then
        if !var_names.contains(n) {
            // a. Append N to varDeclaredNames.
            var_names.push(n.clone())
        }
        // 12. Return NormalCompletion(empty).
        Ok(NormalCompletion::Empty)
    }
}

impl GlobalEnv {
    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-hasbinding-n
    fn has_binding(run: &mut Runtime, eaddr: EnvRecordAddr, n: &Rc<JSString>) -> CompletionRecord {
        if let NormalCompletion::Value(JSValue::Boolean(true)) =
            DeclarativeEnv::has_binding(run, eaddr, n)?
        {
            return Ok(NormalCompletion::Value(JSValue::Boolean(true)));
        }
        ObjectEnv::has_binding(run, eaddr, n)
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-createmutablebinding-n-d
    fn create_mutable_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: Rc<JSString>,
        deletable: bool,
    ) -> CompletionRecord {
        if let NormalCompletion::Value(JSValue::Boolean(true)) =
            DeclarativeEnv::has_binding(run, eaddr, &name)?
        {
            return run.type_error(format!("{} is already defined", &name));
        }
        DeclarativeEnv::create_mutable_binding(run, eaddr, name, deletable)
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-createimmutablebinding-n-s
    fn create_immutable_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: Rc<JSString>,
        strict: bool,
    ) -> CompletionRecord {
        if let NormalCompletion::Value(JSValue::Boolean(true)) =
            DeclarativeEnv::has_binding(run, eaddr, &name)?
        {
            return run.type_error(format!("{} is already defined", &name));
        }
        DeclarativeEnv::create_immutable_binding(run, eaddr, name, strict)
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-initializebinding-n-v
    fn initialize_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        v: JSValue,
    ) -> CompletionRecord {
        if let NormalCompletion::Value(JSValue::Boolean(true)) =
            DeclarativeEnv::has_binding(run, eaddr, name)?
        {
            DeclarativeEnv::initialize_binding(run, eaddr, name, v)
        } else {
            ObjectEnv::initialize_binding(run, eaddr, name, v)
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-setmutablebinding-n-v-s
    //noinspection RsSelfConvention
    fn set_mutable_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        v: JSValue,
        strict: bool,
    ) -> CompletionRecord {
        if let NormalCompletion::Value(JSValue::Boolean(true)) =
            DeclarativeEnv::has_binding(run, eaddr, name)?
        {
            DeclarativeEnv::set_mutable_binding(run, eaddr, name, v, strict)
        } else {
            ObjectEnv::set_mutable_binding(run, eaddr, name, v, strict)
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-getbindingvalue-n-s
    fn get_binding_value(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
        strict: bool,
    ) -> CompletionRecord {
        if let NormalCompletion::Value(JSValue::Boolean(true)) =
            DeclarativeEnv::has_binding(run, eaddr, name)?
        {
            DeclarativeEnv::get_binding_value(run, eaddr, name, strict)
        } else {
            ObjectEnv::get_binding_value(run, eaddr, name, strict)
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-getthisbinding
    fn get_this_binding(run: &mut Runtime, eaddr: EnvRecordAddr) -> CompletionRecord {
        let env_rec = run.env_record(eaddr);
        Ok(NormalCompletion::Value(
            env_rec.glob.global_this_value.clone(),
        ))
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-deletebinding-n
    fn delete_binding(
        run: &mut Runtime,
        eaddr: EnvRecordAddr,
        name: &Rc<JSString>,
    ) -> CompletionRecord {
        if let NormalCompletion::Value(JSValue::Boolean(true)) =
            DeclarativeEnv::has_binding(run, eaddr, name)?
        {
            return DeclarativeEnv::delete_binding(run, eaddr, name);
        }

        let global_object = run.env_record(eaddr).obj.binding_object.unwrap();
        if run.has_own_property(global_object, &JSValue::String(name.clone())) {
            let status = ObjectEnv::delete_binding(run, eaddr, name)?;
            if NormalCompletion::Value(JSValue::Boolean(true)) == status {
                let var_names = &mut run.env_record_mut(eaddr).glob.var_names;
                if let Some(pos) = var_names.iter().position(|e| *e == *name) {
                    var_names.remove(pos);
                }
            }
            return Ok(status);
        }

        Ok(NormalCompletion::Value(JSValue::Boolean(true)))
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-hasthisbinding
    fn has_this_binding(_run: &Runtime, _eaddr: EnvRecordAddr) -> bool {
        true
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-hassuperbinding
    fn has_super_binding(_run: &Runtime, _eaddr: EnvRecordAddr) -> bool {
        false
    }

    /// https://262.ecma-international.org/11.0/#sec-global-environment-records-withbaseobject
    fn with_base_object(_run: &Runtime, _eaddr: EnvRecordAddr) -> Option<ObjectAddr> {
        None
    }
}
