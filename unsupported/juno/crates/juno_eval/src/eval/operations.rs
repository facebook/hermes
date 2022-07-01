/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::jsvalue::*;
use crate::eval::completion_record::CompletionRecord;
use crate::eval::completion_record::NormalCompletion;
use crate::eval::runtime::Runtime;
use num::Zero;

/// https://262.ecma-international.org/11.0/#array-index
///
/// An integer index is a String-valued property key that is a canonical numeric
/// String (see 7.1.21) and whose numeric value is either +0 or a positive
/// integer `≤ 2**53 - 1`. An array index is an integer index whose numeric value
/// i is in the range `+0 ≤ i < 2**32 - 1`.
pub fn string_to_array_index(s: &[u16]) -> Option<u32> {
    const V0: u16 = '0' as u16;
    const V9: u16 = '9' as u16;
    match s {
        // Empty string is invalid
        []
        // Leading "0" is invalid.
        | [V0, _, ..] => return None,
        // A single "0" is just that.
        [V0] => return Some(0u32),
        _ => ()
    }

    let mut res: u32 = 0;
    for ch in s {
        if *ch < V0 || *ch > V9 {
            return None;
        }
        let tmp = (res as u64) * 10 + (*ch - V0) as u64;
        if tmp >= 0xFFFFFFFF {
            return None;
        }
        res = tmp as u32;
    }

    Some(res)
}

/// https://262.ecma-international.org/11.0/#sec-toboolean
pub fn to_boolean(a: &JSValue) -> bool {
    match a {
        JSValue::Undefined | JSValue::Null => false,
        JSValue::Boolean(b) => *b,
        JSValue::String(s) => s.len() != 0,
        JSValue::Object(_) | JSValue::Symbol(_) => true,
        JSValue::Number(n) => !(n.is_zero() || n.is_nan()),
        JSValue::BigInt(bi) => !bi.is_zero(),
    }
}

/// https://262.ecma-international.org/11.0/#sec-toobject
pub fn to_object(run: &mut Runtime, a: &JSValue) -> CompletionRecord {
    match a {
        JSValue::Undefined | JSValue::Null => {
            run.type_error("Cannot convert undefined or null to object")
        }
        JSValue::Boolean(_)
        | JSValue::String(_)
        | JSValue::Symbol(_)
        | JSValue::Number(_)
        | JSValue::BigInt(_) => unimplemented!("ToObject()"),
        JSValue::Object(_) => Ok(NormalCompletion::Value(a.clone())),
    }
}

/// https://262.ecma-international.org/11.0/#sec-ispropertykey
pub fn is_property_key(a: &JSValue) -> bool {
    matches!(a, JSValue::String(_) | JSValue::Symbol(_))
}

/// https://262.ecma-international.org/11.0/#sec-samevalue
pub fn same_value(x: &JSValue, y: &JSValue) -> bool {
    if std::mem::discriminant(x) != std::mem::discriminant(y) {
        return false;
    }
    match (x, y) {
        (JSValue::Number(xn), JSValue::Number(yn)) => number_same_value(*xn, *yn),
        (JSValue::BigInt(xn), JSValue::BigInt(yn)) => bigint_same_value(xn, yn),
        _ => same_value_non_numeric(x, y),
    }
}

/// https://262.ecma-international.org/11.0/#sec-samevaluenonnumeric
pub fn same_value_non_numeric(x: &JSValue, y: &JSValue) -> bool {
    debug_assert!(!matches!(x, JSValue::Number(_) | JSValue::BigInt(_)));
    debug_assert!(std::mem::discriminant(x) == std::mem::discriminant(y));

    match (x, y) {
        (JSValue::Undefined, _) | (JSValue::Null, _) => true,
        (JSValue::String(xs), JSValue::String(ys)) => *xs == *ys,
        (JSValue::Boolean(xb), JSValue::Boolean(yb)) => xb == yb,
        (JSValue::Symbol(xs), JSValue::Symbol(ys)) => *xs == *ys,
        (JSValue::Object(xo), JSValue::Object(yo)) => *xo == *yo,
        _ => panic!("invalid value type"),
    }
}

/// https://262.ecma-international.org/11.0/#sec-numeric-types-number-sameValue
pub fn number_same_value(x: f64, y: f64) -> bool {
    x.to_bits() == y.to_bits()
}

/// https://262.ecma-international.org/11.0/#sec-numeric-types-bigint-sameValue
pub fn bigint_same_value(x: &num::BigInt, y: &num::BigInt) -> bool {
    x.eq(y)
}
