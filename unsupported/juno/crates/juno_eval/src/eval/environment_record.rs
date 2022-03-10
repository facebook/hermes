/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::addr::*;
use super::completion_record::*;
use super::jsvalue::*;
use super::runtime::*;
use crate::eval::jsobject::PropertyDescriptor;
use crate::eval::operations::to_boolean;
use std::rc::Rc;

#[derive(Debug)]
struct Binding {
    mutable: bool,
    deletable: bool,
    strict: bool,
    value: Option<JSValue>,
}

#[derive(Debug)]
pub struct DeclarativeEnv {
    names: Vec<Rc<JSString>>,
    bindings: Vec<Binding>,
}

#[derive(Debug)]
pub struct ObjectEnv {
    binding_object: ObjectAddr,
    with_environment: bool,
}

#[derive(Debug)]
enum ThisBindingStatus {
    Lexical,
    Initialized,
    Uninitialized,
}

#[derive(Debug)]
pub struct FunctionEnv {
    this_value: JSValue,
    this_binding_status: ThisBindingStatus,
    function_object: JSValue,
    home_object: JSValue,
    new_target: JSValue,
}

#[derive(Debug)]
pub struct GlobalEnv {
    global_this_value: JSValue,
    var_names: Vec<Rc<JSString>>,
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
    pub delete_binding: fn(&mut Runtime, EnvRecordAddr, &Rc<JSString>) -> CompletionRecord,
    pub has_this_binding: fn(&Runtime, EnvRecordAddr) -> bool,
    pub has_super_binding: fn(&Runtime, EnvRecordAddr) -> bool,
    pub with_base_object: fn(&Runtime, EnvRecordAddr) -> Option<ObjectAddr>,
}

// declarative_record: DeclarativeEnv,
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
    methods: &'static EnvironmentMethods,
    /// This field used for debugging.
    kind: EnvironmentRecordKind,

    decl: DeclarativeEnv,
    func: FunctionEnv,
    obj: ObjectEnv,
    glob: GlobalEnv,
}

static DECL_ENV_METHODS: EnvironmentMethods = EnvironmentMethods {
    has_binding: DeclarativeEnv::has_binding,
    create_mutable_binding: DeclarativeEnv::create_mutable_binding,
    create_immutable_binding: DeclarativeEnv::create_immutable_binding,
    initialize_binding: DeclarativeEnv::initialize_binding,
    set_mutable_binding: DeclarativeEnv::set_mutable_binding,
    get_binding_value: DeclarativeEnv::get_binding_value,
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
    delete_binding: ObjectEnv::delete_binding,
    has_this_binding: ObjectEnv::has_this_binding,
    has_super_binding: ObjectEnv::has_super_binding,
    with_base_object: ObjectEnv::with_base_object,
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
            run.env_record(eaddr).decl.find_binding(&n).is_some(),
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
        dcl_rec
            .initialize_binding_impl(dcl_rec.find_binding(&name).expect("binding must exist"), v);
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

        return Ok(NormalCompletion::Empty);
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
        let index = dcl_rec.find_binding(&name).expect("binding must exist");
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
    fn with_base_object(run: &Runtime, eaddr: EnvRecordAddr) -> Option<ObjectAddr> {
        None
    }
}

impl ObjectEnv {
    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-hasbinding-n
    fn has_binding(run: &mut Runtime, eaddr: EnvRecordAddr, n: &Rc<JSString>) -> CompletionRecord {
        let env_rec = &run.env_record(eaddr);
        let nv = JSValue::String(n.clone());
        if !run.has_property(env_rec.obj.binding_object, &nv) {
            return Ok(NormalCompletion::Value(JSValue::Boolean(false)));
        }
        if !env_rec.obj.with_environment {
            return Ok(NormalCompletion::Value(JSValue::Boolean(true)));
        }
        let unscopables = run
            .get(
                env_rec.obj.binding_object,
                &run.well_known_symbol(WellKnownSymbol::Unscopables),
            )?
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
        let bindings_addr = run.env_record(eaddr).obj.binding_object;
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
        Self::set_mutable_binding(run, eaddr, &name, v, false)
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
        let bindings_addr = run.env_record(eaddr).obj.binding_object;
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
        let bindings_addr = run.env_record(eaddr).obj.binding_object;
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
        let bindings_addr = run.env_record(eaddr).obj.binding_object;
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
            Some(env_rec.obj.binding_object)
        } else {
            None
        }
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
    //noinspection RsSelfConvention
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

        let global_object = run.env_record(eaddr).obj.binding_object;
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
    fn with_base_object(run: &Runtime, eaddr: EnvRecordAddr) -> Option<ObjectAddr> {
        None
    }
}
