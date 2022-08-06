/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::cmp::Ordering;
use std::fmt::Debug;
use std::fmt::Display;
use std::fmt::Formatter;
use std::hash::Hash;
use std::hash::Hasher;
use std::rc::Rc;

use super::runtime::*;

#[derive(Clone, Debug, PartialEq)]
pub enum JSValue {
    Undefined,
    Null,
    Boolean(bool),
    String(Rc<JSString>),
    Symbol(Rc<JSSymbol>),
    Number(f64),
    BigInt(Rc<num::BigInt>),
    Object(ObjectAddr),
}

#[macro_export]
macro_rules! jsvalue_cast {
    ($type:path, $value:expr) => {
        match $value {
            $type(v) => v,
            _ => panic!(
                "jsvalue cast to {} from {}",
                stringify!($kind),
                $value.name()
            ),
        }
    };
}

impl Default for JSValue {
    fn default() -> Self {
        JSValue::Undefined
    }
}

impl JSValue {
    pub fn cast_object(&self) -> ObjectAddr {
        *jsvalue_cast!(JSValue::Object, self)
    }
    pub fn cast_string(&self) -> &Rc<JSString> {
        jsvalue_cast!(JSValue::String, self)
    }
}

impl Display for JSValue {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            JSValue::Undefined => write!(f, "undefined"),
            JSValue::Null => write!(f, "null"),
            JSValue::Boolean(b) => Display::fmt(b, f),
            JSValue::String(s) => write!(f, "'{}'", s),
            JSValue::Symbol(s) => write!(f, "Symbol({:p})", Rc::as_ptr(s)),
            JSValue::Number(n) => Display::fmt(n, f),
            JSValue::BigInt(b) => Display::fmt(b, f),
            JSValue::Object(o) => write!(f, "Object({:?})", *o),
        }
    }
}

impl JSValue {
    pub fn name(&self) -> &'static str {
        match self {
            JSValue::Undefined => "Undefined",
            JSValue::Null => "Null",
            JSValue::Boolean(_) => "Boolean",
            JSValue::String(_) => "String",
            JSValue::Symbol(_) => "Symbol",
            JSValue::Number(_) => "Number",
            JSValue::BigInt(_) => "BigInt",
            JSValue::Object(_) => "Object",
        }
    }
}

/// A JavaScript string. The second value is a representation for debugging.
#[derive(Clone, Debug, Eq)]
pub struct JSString(Box<[u16]>, Box<str>);

impl JSString {
    pub fn new(value: Box<[u16]>) -> Rc<Self> {
        let s = String::from_utf16_lossy(&*value);
        Rc::new(JSString(value, s.into_boxed_str()))
    }
    pub fn from_str(s: &str) -> Rc<Self> {
        let u: Vec<u16> = s.encode_utf16().collect();
        Rc::new(JSString(
            u.into_boxed_slice(),
            s.to_owned().into_boxed_str(),
        ))
    }
    pub fn len(&self) -> usize {
        self.0.len()
    }
    pub fn as_u16_slice(&self) -> &[u16] {
        self.0.as_ref()
    }
}

impl Display for JSString {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        std::fmt::Display::fmt(&self.1, f)
    }
}

impl PartialEq for JSString {
    fn eq(&self, other: &Self) -> bool {
        self.0.eq(&other.0)
    }
}

impl PartialOrd for JSString {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.0.partial_cmp(&other.0)
    }
}

impl Ord for JSString {
    fn cmp(&self, other: &Self) -> Ordering {
        self.0.cmp(&other.0)
    }
}

impl Hash for JSString {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.0.hash(state)
    }
}

/// A JSSymbol contains an optional description.
#[derive(Clone, Debug, Eq)]
pub struct JSSymbol(Option<Rc<JSString>>);

impl PartialEq for JSSymbol {
    fn eq(&self, other: &Self) -> bool {
        std::ptr::eq(self, other)
    }
}

impl JSSymbol {
    pub fn new_with_str(desc: &str) -> Rc<JSSymbol> {
        Rc::new(JSSymbol(Some(JSString::from_str(desc))))
    }
}
