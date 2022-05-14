/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::completion_record::*;
use super::function::*;
use super::operations::*;
use super::runtime::*;
use crate::eval::jsvalue::JSValue;
use juno_support::declare_opaque_id;
use std::cmp::Ordering;
use std::collections::HashMap;

declare_opaque_id!(PropertyIndex);

#[derive(Clone, Debug)]
pub enum PropertyValue {
    Data {
        value: JSValue,
        writable: bool,
    },
    Accessor {
        get: Option<JSValue>,
        set: Option<JSValue>,
    },
}

#[derive(Clone, Debug)]
pub struct Property {
    value: PropertyValue,
    enumerable: bool,
    configurable: bool,
}

/// https://262.ecma-international.org/11.0/#sec-property-descriptor-specification-type
#[derive(Default, Clone)]
pub struct PropertyDescriptor {
    /// Property index populated by queries.
    pub index: Option<PropertyIndex>,

    pub value: Option<JSValue>,
    pub get: Option<JSValue>,
    pub set: Option<JSValue>,
    pub writable: Option<bool>,
    pub enumerable: Option<bool>,
    pub configurable: Option<bool>,
}

/// Internal slots for objects.
/// Sorted alphabetically for convenience.
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum InternalSlotName {
    BooleanData,
    DateValue,
    Environment,
    ErrorData,
    Extensible,
    HomeObject,
    NumberData,
    ParameterMap,
    Prototype,
    RegExpMatcher,
    StringData,
    ThisMode,
}

#[derive(Debug)]
pub enum InternalSlotValue {
    Empty,
    Value(JSValue),
    ThisMode(ThisMode),
    LexicalEnvironment(LexicalEnvAddr),
}

impl From<JSValue> for InternalSlotValue {
    fn from(val: JSValue) -> Self {
        InternalSlotValue::Value(val)
    }
}

impl From<ThisMode> for InternalSlotValue {
    fn from(val: ThisMode) -> Self {
        InternalSlotValue::ThisMode(val)
    }
}

pub struct ObjectMethods {
    pub get_prototype_of: fn(&Runtime, ObjectAddr) -> &JSValue,
    pub set_prototype_of: fn(&mut Runtime, ObjectAddr, JSValue) -> bool,
    pub is_extensible: fn(&Runtime, ObjectAddr) -> bool,
    pub prevent_extensions: fn(&mut Runtime, ObjectAddr),
    pub get_own_property: fn(&Runtime, ObjectAddr, &JSValue) -> Option<PropertyDescriptor>,
    pub define_own_property: fn(&mut Runtime, ObjectAddr, &JSValue, &PropertyDescriptor) -> bool,
    pub has_property: fn(&Runtime, ObjectAddr, &JSValue) -> bool,
    pub get: fn(&mut Runtime, ObjectAddr, &JSValue, &JSValue) -> CompletionRecord,
    pub set: fn(&mut Runtime, ObjectAddr, &JSValue, JSValue, &JSValue) -> CompletionRecord,
    pub delete: fn(&mut Runtime, ObjectAddr, &JSValue) -> bool,
    pub own_property_keys: fn(&Runtime, ObjectAddr) -> Vec<JSValue>,
}

pub struct JSObject {
    pub methods: &'static ObjectMethods,

    keys: Vec<JSValue>,
    values: Vec<Property>,

    internal_slots: HashMap<InternalSlotName, InternalSlotValue>,

    /// If this is a function, these will be populated.
    func: Option<JSFunction>,
}

impl PropertyDescriptor {
    pub fn new(index: Option<PropertyIndex>) -> Self {
        Self {
            index,
            ..Default::default()
        }
    }

    pub fn is_empty(&self) -> bool {
        self.value.is_none()
            && self.get.is_none()
            && self.set.is_none()
            && self.writable.is_none()
            && self.enumerable.is_none()
            && self.configurable.is_none()
    }

    /// https://262.ecma-international.org/11.0/#sec-isaccessordescriptor
    pub fn is_accessor_descriptor(&self) -> bool {
        !(self.get.is_none() && self.set.is_none())
    }
    /// https://262.ecma-international.org/11.0/#sec-isdatadescriptor
    pub fn is_data_descriptor(&self) -> bool {
        !(self.value.is_none() && self.writable.is_none())
    }
    /// https://262.ecma-international.org/11.0/#sec-isgenericdescriptor
    pub fn is_generic_descriptor(&self) -> bool {
        !self.is_accessor_descriptor() && !self.is_data_descriptor()
    }
}

impl InternalSlotValue {
    pub fn get_value(&self) -> &JSValue {
        if let InternalSlotValue::Value(val) = self {
            return val;
        }
        panic!("Internal slot is not a value.");
    }
    pub fn get_lexical_environment(&self) -> LexicalEnvAddr {
        if let InternalSlotValue::LexicalEnvironment(val) = self {
            return *val;
        }
        panic!("Internal slot is not a lexical env.");
    }
}

static OBJECT_METHODS: ObjectMethods = ObjectMethods {
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-getprototypeof
    get_prototype_of: JSObject::ordinary_get_prototype_of,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-setprototypeof-v
    set_prototype_of: JSObject::ordinary_set_prototype_of,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-isextensible
    is_extensible: JSObject::ordinary_is_extensible,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-preventextensions
    prevent_extensions: JSObject::ordinary_prevent_extensions,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-getownproperty-p
    get_own_property: JSObject::ordinary_get_own_property,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-defineownproperty-p-desc
    define_own_property: JSObject::ordinary_define_own_property,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-hasproperty-p
    has_property: JSObject::ordinary_has_property,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-get-p-receiver
    get: JSObject::ordinary_get,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-set-p-v-receiver
    set: JSObject::ordinary_set,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-delete-p
    delete: JSObject::ordinary_delete,
    /// https://262.ecma-international.org/11.0/#sec-ordinary-object-internal-methods-and-internal-slots-ownpropertykeys
    own_property_keys: JSObject::ordinary_own_property_keys,
};

impl JSObject {
    /// Make a new basic object (to be used by MakeBasicObject) with the OBJECT_METHODS methods.
    pub fn new_basic_object(internal_slots_list: &[InternalSlotName]) -> JSObject {
        let mut internal_slots = HashMap::new();
        for slot in internal_slots_list {
            internal_slots.insert(*slot, InternalSlotValue::Empty);
        }
        JSObject {
            methods: &OBJECT_METHODS,
            keys: Default::default(),
            values: Default::default(),
            internal_slots,
            func: None,
        }
    }

    pub fn function(&self) -> Option<&JSFunction> {
        self.func.as_ref()
    }
    pub fn is_function(&self) -> bool {
        self.func.is_some()
    }

    pub fn get_internal_slot(&self, slot: InternalSlotName) -> Option<&InternalSlotValue> {
        self.internal_slots.get(&slot)
    }
    pub fn set_internal_slot(&mut self, slot: InternalSlotName, value: InternalSlotValue) {
        self.internal_slots.insert(slot, value);
    }

    fn property(&self, index: PropertyIndex) -> &Property {
        &self.values[index.as_usize()]
    }
    fn property_mut(&mut self, index: PropertyIndex) -> &mut Property {
        &mut self.values[index.as_usize()]
    }

    fn prototype(&self) -> &JSValue {
        self.internal_slots
            .get(&InternalSlotName::Prototype)
            .unwrap()
            .get_value()
    }
    fn extensible(&self) -> bool {
        *jsvalue_cast!(
            JSValue::Boolean,
            self.internal_slots
                .get(&InternalSlotName::Extensible)
                .unwrap()
                .get_value()
        )
    }

    fn find_own_property(&self, p: &JSValue) -> Option<PropertyIndex> {
        self.keys
            .iter()
            .position(|k| *k == *p)
            .map(PropertyIndex::new)
    }

    fn add_property(&mut self, key: JSValue, prop: Property) -> PropertyIndex {
        self.keys.push(key);
        self.values.push(prop);
        PropertyIndex::new(self.values.len() - 1)
    }

    fn remove_property_by_index(&mut self, index: PropertyIndex) {
        self.keys.remove(index.as_usize());
        self.values.remove(index.as_usize());
    }
}

impl JSObject {
    /// https://262.ecma-international.org/11.0/#sec-ordinarygetprototypeof
    pub fn ordinary_get_prototype_of(run: &Runtime, oaddr: ObjectAddr) -> &JSValue {
        run.object(oaddr).prototype()
    }
    /// https://262.ecma-international.org/11.0/#sec-ordinarysetprototypeof
    fn ordinary_set_prototype_of(run: &mut Runtime, oaddr: ObjectAddr, value: JSValue) -> bool {
        debug_assert!(matches!(value, JSValue::Object(_) | JSValue::Null));

        let o = run.object(oaddr);
        if same_value(&value, o.prototype()) {
            return true;
        }
        if !o.extensible() {
            return false;
        }

        // Check for cycles.
        let mut p = &value;
        loop {
            match p {
                JSValue::Null => break,
                JSValue::Object(po) if *po == oaddr => {
                    // Cycle detected
                    return false;
                }
                JSValue::Object(pa) => {
                    let po = run.object(*pa);
                    if po.methods.get_prototype_of as usize
                        != Self::ordinary_get_prototype_of as usize
                    {
                        break;
                    }
                    p = po.prototype();
                }
                _ => panic!("Invalid prototype value"),
            }
        }

        run.object_mut(oaddr)
            .internal_slots
            .insert(InternalSlotName::Prototype, value.into());
        true
    }

    fn ordinary_is_extensible(run: &Runtime, oaddr: ObjectAddr) -> bool {
        run.object(oaddr).extensible()
    }

    fn ordinary_prevent_extensions(run: &mut Runtime, oaddr: ObjectAddr) {
        run.object_mut(oaddr)
            .internal_slots
            .insert(InternalSlotName::Extensible, JSValue::Boolean(false).into());
    }

    /// https://262.ecma-international.org/11.0/#sec-ordinarygetownproperty
    fn ordinary_get_own_property(
        run: &Runtime,
        oaddr: ObjectAddr,
        p: &JSValue,
    ) -> Option<PropertyDescriptor> {
        debug_assert!(is_property_key(p));
        let o = run.object(oaddr);
        match o.find_own_property(p) {
            None => None,
            Some(index) => {
                let mut d = PropertyDescriptor::new(Some(index));
                let x = &o.values[index.as_usize()];
                match x {
                    Property {
                        value: PropertyValue::Data { value, writable },
                        ..
                    } => {
                        d.value = Some(value.clone());
                        d.writable = Some(*writable);
                    }
                    Property {
                        value: PropertyValue::Accessor { get, set },
                        ..
                    } => {
                        d.get = get.clone();
                        d.set = set.clone();
                    }
                }
                d.enumerable = Some(x.enumerable);
                d.configurable = Some(x.configurable);
                Some(d)
            }
        }
    }

    fn ordinary_define_own_property(
        run: &mut Runtime,
        oaddr: ObjectAddr,
        p: &JSValue,
        desc: &PropertyDescriptor,
    ) -> bool {
        let methods = run.object(oaddr).methods;
        let current = (methods.get_own_property)(run, oaddr, p);
        let extensible = (methods.is_extensible)(run, oaddr);
        Self::validate_and_apply_property_descriptor(
            run,
            Some(oaddr),
            p,
            extensible,
            desc,
            current.as_ref(),
        )
    }

    /// https://262.ecma-international.org/11.0/#sec-validateandapplypropertydescriptor
    fn validate_and_apply_property_descriptor(
        run: &mut Runtime,
        oaddr: Option<ObjectAddr>,
        p: &JSValue,
        extensible: bool,
        desc: &PropertyDescriptor,
        current: Option<&PropertyDescriptor>,
    ) -> bool {
        debug_assert!(oaddr.is_none() || is_property_key(p));
        match current {
            None => {
                if !extensible {
                    return false;
                }
                if let Some(oaddr) = oaddr {
                    if desc.is_generic_descriptor() || desc.is_data_descriptor() {
                        run.object_mut(oaddr).add_property(
                            p.clone(),
                            Property {
                                value: PropertyValue::Data {
                                    value: desc.value.clone().unwrap_or(JSValue::Undefined),
                                    writable: desc.writable.unwrap_or(false),
                                },
                                enumerable: desc.enumerable.unwrap_or(false),
                                configurable: desc.configurable.unwrap_or(false),
                            },
                        );
                    } else {
                        run.object_mut(oaddr).add_property(
                            p.clone(),
                            Property {
                                value: PropertyValue::Accessor {
                                    get: desc.get.clone(),
                                    set: desc.set.clone(),
                                },
                                enumerable: desc.enumerable.unwrap_or(false),
                                configurable: desc.configurable.unwrap_or(false),
                            },
                        );
                    }
                }
            }
            Some(current) => {
                if desc.is_empty() {
                    return true;
                }
                if current.configurable == Some(false) {
                    if desc.configurable == Some(true) {
                        return false;
                    }
                    if desc.enumerable.is_some() && desc.enumerable != current.enumerable {
                        return false;
                    }
                }
                if !desc.is_generic_descriptor() {
                    // No further validation is required.
                } else if current.is_data_descriptor() != desc.is_data_descriptor() {
                    if current.configurable == Some(false) {
                        return false;
                    }
                    if let Some(oaddr) = oaddr {
                        if current.is_data_descriptor() {
                            run.object_mut(oaddr)
                                .property_mut(current.index.unwrap())
                                .value = PropertyValue::Accessor {
                                get: None,
                                set: None,
                            };
                        } else {
                            run.object_mut(oaddr)
                                .property_mut(current.index.unwrap())
                                .value = PropertyValue::Data {
                                value: Default::default(),
                                writable: false,
                            };
                        }
                    }
                } else if current.is_data_descriptor() && desc.is_data_descriptor() {
                    if current.configurable == Some(false) && current.writable == Some(false) {
                        if desc.writable == Some(true) {
                            return false;
                        }
                        if let (Some(old_value), Some(new_value)) = (&current.value, &desc.value) {
                            if !same_value(old_value, new_value) {
                                return false;
                            }
                        }
                        return true;
                    }
                } else {
                    debug_assert!(
                        current.is_accessor_descriptor() && desc.is_accessor_descriptor()
                    );
                    if current.configurable == Some(false) {
                        match (&desc.set, &current.set) {
                            (Some(_), None) => return false,
                            (Some(n), Some(o)) if !same_value(n, o) => return false,
                            _ => {}
                        }
                        match (&desc.get, &current.get) {
                            (Some(_), None) => return false,
                            (Some(n), Some(o)) if !same_value(n, o) => return false,
                            _ => {}
                        }
                        return true;
                    }
                }

                if let Some(oaddr) = oaddr {
                    let prop = run.object_mut(oaddr).property_mut(current.index.unwrap());
                    match &mut prop.value {
                        PropertyValue::Data { value, writable } => {
                            if let Some(v) = &desc.value {
                                *value = v.clone();
                            }
                            if let Some(v) = desc.writable {
                                *writable = v;
                            }
                        }
                        PropertyValue::Accessor { get, set } => {
                            if desc.get.is_some() {
                                *get = desc.get.clone();
                            }
                            if desc.set.is_some() {
                                *set = desc.set.clone();
                            }
                        }
                    }
                    if let Some(v) = desc.enumerable {
                        prop.enumerable = v;
                    }
                    if let Some(v) = desc.configurable {
                        prop.configurable = v;
                    }
                }
            }
        }
        true
    }

    /// https://262.ecma-international.org/11.0/#sec-ordinaryhasproperty
    fn ordinary_has_property(run: &Runtime, oaddr: ObjectAddr, p: &JSValue) -> bool {
        debug_assert!(is_property_key(p));
        let methods = run.object(oaddr).methods;
        if (methods.get_own_property)(run, oaddr, p).is_some() {
            return true;
        }
        let parent = (methods.get_prototype_of)(run, oaddr);
        if let JSValue::Object(pa) = parent {
            (methods.has_property)(run, *pa, p)
        } else {
            false
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-object-environment-records-setmutablebinding-n-v-s
    fn ordinary_get(
        run: &mut Runtime,
        oaddr: ObjectAddr,
        p: &JSValue,
        receiver: &JSValue,
    ) -> CompletionRecord {
        debug_assert!(is_property_key(p));
        match (run.object(oaddr).methods.get_own_property)(run, oaddr, p) {
            None => match (run.object(oaddr).methods.get_prototype_of)(run, oaddr) {
                JSValue::Object(paddr) => {
                    // Copy to avoid the borrows checker.
                    let paddr = *paddr;
                    (run.object(paddr).methods.get)(run, paddr, p, receiver)
                }
                _ => Ok(NormalCompletion::Value(JSValue::Undefined)),
            },
            Some(desc) => {
                if desc.is_data_descriptor() {
                    Ok(NormalCompletion::Value(desc.value.unwrap()))
                } else {
                    debug_assert!(desc.is_accessor_descriptor());
                    match desc.get {
                        None => Ok(NormalCompletion::Value(JSValue::Undefined)),
                        Some(getter) => run.call(&getter, receiver, &[]),
                    }
                }
            }
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-ordinaryset
    fn ordinary_set(
        run: &mut Runtime,
        oaddr: ObjectAddr,
        p: &JSValue,
        v: JSValue,
        receiver: &JSValue,
    ) -> CompletionRecord {
        debug_assert!(is_property_key(p));
        let own_desc = (run.object(oaddr).methods.get_own_property)(run, oaddr, p);
        Self::ordinary_set_with_own_descriptor(run, oaddr, p, v, receiver, own_desc.as_ref())
    }

    /// https://262.ecma-international.org/11.0/#sec-ordinarysetwithowndescriptor
    fn ordinary_set_with_own_descriptor(
        run: &mut Runtime,
        oaddr: ObjectAddr,
        p: &JSValue,
        v: JSValue,
        receiver: &JSValue,
        opt_own_desc: Option<&PropertyDescriptor>,
    ) -> CompletionRecord {
        debug_assert!(is_property_key(p));
        let own_desc = match opt_own_desc {
            None => {
                if let JSValue::Object(parent_addr) =
                    (run.object(oaddr).methods.get_prototype_of)(run, oaddr)
                {
                    // Avoid the borrows checker.
                    let parent_addr = *parent_addr;
                    return (run.object(parent_addr).methods.set)(run, parent_addr, p, v, receiver);
                }
                PropertyDescriptor {
                    value: Some(JSValue::Undefined),
                    writable: Some(true),
                    enumerable: Some(true),
                    configurable: Some(true),
                    ..Default::default()
                }
            }
            Some(d) => (*d).clone(),
        };

        if own_desc.is_data_descriptor() {
            if own_desc.writable == Some(false) {
                return Ok(NormalCompletion::Value(JSValue::Boolean(false)));
            }
            let recv_addr = match receiver {
                JSValue::Object(a) => *a,
                _ => return Ok(NormalCompletion::Value(JSValue::Boolean(false))),
            };
            let res = if let Some(existing_descriptor) =
                (run.object(recv_addr).methods.get_own_property)(run, recv_addr, p)
            {
                if existing_descriptor.is_accessor_descriptor()
                    || existing_descriptor.writable == Some(false)
                {
                    return Ok(NormalCompletion::Value(JSValue::Boolean(false)));
                }
                let value_desc = PropertyDescriptor {
                    value: Some(v),
                    ..Default::default()
                };
                (run.object(recv_addr).methods.define_own_property)(run, recv_addr, p, &value_desc)
            } else {
                run.create_data_property(recv_addr, p, v)
            };
            return Ok(NormalCompletion::Value(JSValue::Boolean(res)));
        }
        debug_assert!(own_desc.is_accessor_descriptor());
        if let Some(setter) = &own_desc.set {
            run.call(setter, receiver, &[v])
        } else {
            Ok(NormalCompletion::Value(JSValue::Boolean(false)))
        }
    }

    /// https://262.ecma-international.org/11.0/#sec-ordinarydelete
    pub fn ordinary_delete(run: &mut Runtime, oaddr: ObjectAddr, p: &JSValue) -> bool {
        debug_assert!(is_property_key(p));
        if let Some(desc) = (run.object(oaddr).methods.get_own_property)(run, oaddr, p) {
            if desc.configurable != Some(true) {
                return false;
            }
            run.object_mut(oaddr)
                .remove_property_by_index(desc.index.unwrap());
        }
        true
    }

    /// https://262.ecma-international.org/11.0/#sec-ordinaryownpropertykeys
    pub fn ordinary_own_property_keys(run: &Runtime, oaddr: ObjectAddr) -> Vec<JSValue> {
        let mut res = run.object(oaddr).keys.clone();

        // Sort: indexes first, strings next, symbols last.
        res.sort_by(|a, b| {
            let a_index = if let JSValue::String(s) = a {
                string_to_array_index(s.as_u16_slice())
            } else {
                None
            };
            let b_index = if let JSValue::String(s) = b {
                string_to_array_index(s.as_u16_slice())
            } else {
                None
            };

            match (a_index, b_index) {
                (Some(ai), Some(bi)) => ai.cmp(&bi),
                (Some(_), _) => Ordering::Less,
                (_, Some(_)) => Ordering::Greater,
                _ => match (a, b) {
                    (JSValue::String(_), JSValue::Symbol(_)) => Ordering::Less,
                    (JSValue::Symbol(_), JSValue::String(_)) => Ordering::Greater,
                    _ => Ordering::Equal,
                },
            }
        });

        res
    }

    // https://262.ecma-international.org/11.0/#sec-ordinaryobjectcreate
    pub fn ordinary_object_create(
        run: &mut Runtime,
        proto: JSValue,
        additional_internal_slots_list: Option<&[InternalSlotName]>,
    ) -> ObjectAddr {
        let mut internal_slots_list =
            vec![InternalSlotName::Prototype, InternalSlotName::Extensible];
        if let Some(additional) = additional_internal_slots_list {
            internal_slots_list.extend(additional);
        }
        let o = run.make_basic_object(&internal_slots_list);
        run.object_mut(o)
            .internal_slots
            .insert(InternalSlotName::Prototype, proto.into());
        o
    }
}
