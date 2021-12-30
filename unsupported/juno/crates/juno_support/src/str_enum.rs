/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// Create a public enum with simple variants which can be converted
/// back and forth from a `&str` simply.
/// ```ignore
/// define_str_enum!(Foo,
///   ErrorType,
///   (Variant1, "String1"),
///   (Variant2, "String2")
/// );
/// ```
/// will create a `Foo` enum with two variants and their string counterparts.
/// `ErrorType` is the type of the error when a parse fails.
#[macro_export]
macro_rules! define_str_enum {
    ($name:ident, $error:ident, $(($variant:ident, $string:expr)),+ $(,)?) => {
        #[derive(Debug, Copy, Clone, PartialEq, Eq, PartialOrd, Ord)]
        pub enum $name {
            $($variant),+
        }

        /// Implementing FromStr allows us to use the `.parse()` function.
        impl std::str::FromStr for $name {
            type Err = $error;

            fn from_str(value: &str) -> Result<Self, Self::Err> {
                match value {
                    $($string => Ok(Self::$variant),)+
                    _ => Err($error),
                }
            }
        }

        impl std::convert::TryFrom<&str> for $name {
            type Error = $error;

            fn try_from(value: &str) -> Result<Self, Self::Error> {
                use std::str::FromStr;
                Self::from_str(value)
            }
        }

        impl $name {
            pub fn as_str(&self) -> &'static str {
                match self {
                    $(Self::$variant => $string,)+
                }
            }
        }

    }
}
