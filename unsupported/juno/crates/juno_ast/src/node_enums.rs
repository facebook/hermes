/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Simple enums used as children in AST nodes.

use juno_support::define_str_enum;
use thiserror::Error;

use crate::node_child::NodeChild;

#[derive(Debug, Copy, Clone, Error)]
#[error("Invalid string property for AST node")]
pub struct TryFromStringError;

define_str_enum!(
    UnaryExpressionOperator,
    TryFromStringError,
    (Delete, "delete"),
    (Void, "void"),
    (Typeof, "typeof"),
    (Plus, "+"),
    (Minus, "-"),
    (BitNot, "~"),
    (Not, "!"),
);

define_str_enum!(
    BinaryExpressionOperator,
    TryFromStringError,
    (LooseEquals, "=="),
    (LooseNotEquals, "!="),
    (StrictEquals, "==="),
    (StrictNotEquals, "!=="),
    (Less, "<"),
    (LessEquals, "<="),
    (Greater, ">"),
    (GreaterEquals, ">="),
    (LShift, "<<"),
    (RShift, ">>"),
    (RShift3, ">>>"),
    (Plus, "+"),
    (Minus, "-"),
    (Mult, "*"),
    (Div, "/"),
    (Mod, "%"),
    (BitOr, "|"),
    (BitXor, "^"),
    (BitAnd, "&"),
    (Exp, "**"),
    (In, "in"),
    (Instanceof, "instanceof"),
);

define_str_enum!(
    LogicalExpressionOperator,
    TryFromStringError,
    (And, "&&"),
    (Or, "||"),
    (NullishCoalesce, "??"),
);

define_str_enum!(
    UpdateExpressionOperator,
    TryFromStringError,
    (Increment, "++"),
    (Decrement, "--"),
);

define_str_enum!(
    AssignmentExpressionOperator,
    TryFromStringError,
    (Assign, "="),
    (LShiftAssign, "<<="),
    (RShiftAssign, ">>="),
    (RShift3Assign, ">>>="),
    (PlusAssign, "+="),
    (MinusAssign, "-="),
    (MultAssign, "*="),
    (DivAssign, "/="),
    (ModAssign, "%="),
    (BitOrAssign, "|="),
    (BitXorAssign, "^="),
    (BitAndAssign, "&="),
    (ExpAssign, "**="),
    (LogicalOrAssign, "||="),
    (LogicalAndAssign, "&&="),
    (NullishCoalesceAssign, "??="),
);

define_str_enum!(
    VariableDeclarationKind,
    TryFromStringError,
    (Var, "var"),
    (Let, "let"),
    (Const, "const"),
);

define_str_enum!(
    PropertyKind,
    TryFromStringError,
    (Init, "init"),
    (Get, "get"),
    (Set, "set"),
);

define_str_enum!(
    MethodDefinitionKind,
    TryFromStringError,
    (Method, "method"),
    (Constructor, "constructor"),
    (Get, "get"),
    (Set, "set"),
);

define_str_enum!(
    ImportKind,
    TryFromStringError,
    (Value, "value"),
    (Type, "type"),
    (Typeof, "typeof"),
);

define_str_enum!(
    ExportKind,
    TryFromStringError,
    (Value, "value"),
    (Type, "type"),
);

impl NodeChild<'_> for UnaryExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for BinaryExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for LogicalExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for UpdateExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for AssignmentExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for VariableDeclarationKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for PropertyKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for MethodDefinitionKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for ImportKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for ExportKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
