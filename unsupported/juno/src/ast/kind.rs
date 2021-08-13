/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::{Node, NodeChild, NodeLabel, NodeList, NodePtr, StringLiteral, Visitor};

/// Generate boilerplate code for the `NodeKind` enum.
/// The macro matches structures like this:
/// ```
/// gen_nodekind_enum! {
/// NodeKind {
///     Node1[parent] {
///       field1: type1[constraint_a, constraint_b],
///       field2: type2,
///     },
///     Node2,
/// }
/// }
/// ```
/// and creates the necessary enum and supporting function implementations
/// to go along with it in order to avoid excessive copy/paste boilerplate.
/// Parents and constraints can be any member of `NodeVariant`,
/// which includes any constructible `NodeKind`s` as well as interfaces like
/// `Statement`, `Expression`, etc.
/// If multiple constraints are provided, at least one must be satisfied.
/// The `null` constraint is encoded via `Option`, it need not be listed explicitly.
macro_rules! gen_nodekind_enum {
    ($name:ident {
        $(
            $kind:ident $([ $parent:ident ])? $({
                $(
                    $field:ident : $type:ty
                    $( [ $( $constraint:ident ),* ] )?
                ),*
                $(,)?
            })?
        ),*
        $(,)?
    }) => {
        // The kind of an AST node.
        // Matching against this enum allows traversal of the AST.
        // Each variant of the enum must only have fields of the following types:
        // * `NodePtr`
        // * `Option<NodePtr>`
        // * `NodeList`
        // * `StringLiteral`
        // * `NodeLabel`
        // * `bool`
        // * `f64`
        #[derive(Debug)]
        pub enum $name {
            // Create each field in the enum.
            $(
                $kind $({
                    $($field : $type),*
                })?
            ),*
        }

        impl $name {
            /// Visit the child fields of this kind.
            /// `node` is the node for which this is the kind.
            pub fn visit_children<V: Visitor>(&self, visitor: &mut V, node: &Node) {
                match self {
                    $(
                        Self::$kind $({$($field),*})? => {
                            $($(
                                $field.visit(visitor, node);
                            )*)?
                        }
                    ),*
                }
            }

            pub fn variant(&self) -> NodeVariant {
                match self {
                    $(
                        Self::$kind { .. } => NodeVariant::$kind
                    ),*
                }
            }

            /// Check whether this is a valid kind for `node`.
            pub fn validate<'n>(&self, node: &'n Node) -> bool {
                match self {
                    $(
                        Self::$kind $({$($field),*})? => {
                            // Run the validation for each child.
                            // Use `true &&` to make it work when there's no children.
                            true $(&& $(
                                $field.validate(node, &[$($(NodeVariant::$constraint),*)?])
                            )&&*)?
                        }
                    ),*
                }
            }

            pub fn name(&self) -> &'static str {
                match self {
                    $(
                        Self::$kind { .. } => {
                            stringify!($kind)
                        }
                    ),*
                }
            }
        }

        /// Just type information on the node without any of the children.
        /// Used for performing tasks based only on the type of the AST node
        /// without having to know more about it.
        /// Includes "abstract" nodes which cannot be truly constructed.
        #[derive(Debug, Copy, Clone, Eq, PartialEq)]
        pub enum NodeVariant {
            Expression,
            Statement,
            Declaration,
            Literal,
            Pattern,
            LVal,
            $($kind),*
        }

        impl NodeVariant {
            /// The `parent` of the variant in ESTree, used for validation.
            /// Return `None` if there is no parent.
            pub fn parent(&self) -> Option<NodeVariant> {
                match self {
                    Self::Expression => None,
                    Self::Statement => None,
                    Self::Declaration => Some(Self::Statement),
                    Self::Literal => Some(Self::Expression),
                    Self::Pattern => Some(Self::LVal),
                    Self::LVal => Some(Self::Expression),
                    $(
                        Self::$kind => {
                            None$(.or(Some(Self::$parent)))?
                        }
                    ),*
                }
            }
        }
    };
}

gen_nodekind_enum! {
NodeKind {
    Empty,
    Metadata,
    Program {
        body: NodeList[Directive, Statement],
    },
    FunctionExpression[Expression] {
        id: Option<NodePtr>[Identifier],
        params: NodeList[Pattern],
        body: NodePtr[BlockStatement],
        type_parameters: Option<NodePtr>[TypeParameterDeclaration],
        return_type: Option<NodePtr>[TypeAnnotation],
        predicate: Option<NodePtr>[InferredPredicate, DeclaredPredicate],
        generator: bool,
        is_async: bool,
    },
    ArrowFunctionExpression[Expression] {
        id: Option<NodePtr>[Identifier],
        params: NodeList[Pattern],
        body: NodePtr[BlockStatement],
        type_parameters: Option<NodePtr>[TypeParameterDeclaration],
        return_type: Option<NodePtr>[TypeAnnotation],
        predicate: Option<NodePtr>[InferredPredicate, DeclaredPredicate],
        expression: bool,
        is_async: bool,
    },
    FunctionDeclaration[Declaration] {
        id: Option<NodePtr>[Identifier],
        params: NodeList[Pattern],
        body: NodePtr[BlockStatement],
        type_parameters: Option<NodePtr>[TypeParameterDeclaration],
        return_type: Option<NodePtr>[TypeAnnotation],
        predicate: Option<NodePtr>[InferredPredicate, DeclaredPredicate],
        generator: bool,
        is_async: bool,
    },
    WhileStatement[Statement] {
        body: NodePtr[Statement],
        test: NodePtr[Expression],
    },
    DoWhileStatement[Statement] {
        body: NodePtr[Statement],
        test: NodePtr[Expression],
    },
    ForInStatement[Statement] {
        left: NodePtr[VariableDeclaration, Pattern],
        right: NodePtr[Expression],
        body: NodePtr[Statement],
    },
    ForOfStatement[ForInStatement] {
        left: NodePtr[VariableDeclaration, Pattern],
        right: NodePtr[Expression],
        body: NodePtr[Statement],
        is_await: bool,
    },
    ForStatement[Statement] {
        init: Option<NodePtr>[VariableDeclaration, Expression],
        test: Option<NodePtr>[Expression],
        update: Option<NodePtr>[Expression],
        body: NodePtr[Statement],
    },
    DebuggerStatement[Statement],
    EmptyStatement[Statement],
    BlockStatement[Statement] {
        body: NodeList[Statement],
    },
    BreakStatement[Statement] {
        label: Option<NodePtr>[Identifier],
    },
    ContinueStatement[Statement] {
        label: Option<NodePtr>[Identifier],
    },
    ThrowStatement[Statement] {
        argument: NodePtr[Expression],
    },
    ReturnStatement[Statement] {
        argument: Option<NodePtr>[Expression],
    },
    WithStatement[Statement] {
        object: NodePtr[Expression],
        body: NodePtr[Statement],
    },
    SwitchStatement[Statement] {
        discriminant: NodePtr[Expression],
        cases: NodeList[SwitchCase],
    },
    LabeledStatement[Statement] {
        label: NodePtr[Identifier],
        body: NodePtr[Statement],
    },
    ExpressionStatement[Statement] {
        expression: NodePtr[Expression],
        directive: Option<StringLiteral>,
    },
    TryStatement[Statement] {
        block: NodePtr[BlockStatement],
        handler: Option<NodePtr>[CatchClause],
        finalizer: Option<NodePtr>[BlockStatement],
    },
    IfStatement[Statement] {
        test: NodePtr[Expression],
        consequent: NodePtr[Statement],
        alternate: Option<NodePtr>[Statement],
    },
    NullLiteral[Literal],
    BooleanLiteral[Literal] {
        value: bool,
    },
    StringLiteral[Literal] {
        value: StringLiteral,
    },
    NumericLiteral[Literal] {
        value: f64,
    },
    RegExpLiteral[Literal] {
        pattern: NodeLabel,
        flags: NodeLabel,
    },
    ThisExpression[Expression],
    Super,
    SequenceExpression[Expression] {
        expressions: NodeList[Expression],
    },
    ObjectExpression[Expression] {
        properties: NodeList[Property],
    },
    ArrayExpression[Expression] {
        elements: NodeList[Expression, SpreadElement],
        trailing_comma: bool,
    },
    SpreadElement {
        argument: NodePtr[Expression],
    },
    NewExpression[Expression] {
        callee: NodePtr[Expression],
        type_arguments: Option<NodePtr>[TypeParameterInstantiation],
        arguments: NodeList[Expression, SpreadElement],
    },
    YieldExpression[Expression] {
        argument: Option<NodePtr>[Expression],
        delegate: bool,
    },
    AwaitExpression[Expression] {
        argument: NodePtr[Expression],
    },
    ImportExpression[Expression] {
        source: NodePtr[Expression],
        attributes: Option<NodePtr>[Expression],
    },
    CallExpression[Expression] {
        callee: NodePtr[Expression, Super],
        type_arguments: Option<NodePtr>[TypeParameterInstantiation],
        arguments: NodeList[Expression],
    },
    OptionalCallExpression[Expression] {
        callee: NodePtr[Expression, Super],
        type_arguments: Option<NodePtr>[TypeParameterInstantiation],
        arguments: NodeList[Expression],
        optional: bool,
    },
    AssignmentExpression[Expression] {
        operator: NodeLabel,
        left: NodePtr[LVal],
        right: NodePtr[Expression],
    },
    UnaryExpression[Expression] {
        operator: NodeLabel,
        argument: NodePtr[Expression],
        prefix: bool,
    },
    UpdateExpression[Expression] {
        operator: NodeLabel,
        argument: NodePtr[Expression],
        prefix: bool,
    },
    MemberExpression[LVal] {
        object: NodePtr[Expression],
        property: NodePtr[Expression],
        computed: bool,
    },
    OptionalMemberExpression[Expression] {
        object: NodePtr[Expression],
        property: NodePtr[Expression],
        computed: bool,
        optional: bool,
    },
    LogicalExpression[Expression] {
        left: NodePtr[Expression],
        right: NodePtr[Expression],
        operator: NodeLabel,
    },
    ConditionalExpression[Expression] {
        test: NodePtr[Expression],
        alternate: NodePtr[Expression],
        consequent: NodePtr[Expression],
    },
    BinaryExpression[Expression] {
        left: NodePtr[Expression],
        right: NodePtr[Expression],
        operator: NodeLabel,
    },
    Directive[Statement] {
        value: NodePtr,
    },
    DirectiveLiteral[Literal] {
        value: StringLiteral,
    },
    Identifier[Pattern] {
        name: NodeLabel,
        type_annotation: Option<NodePtr>[TypeAnnotation],
        optional: bool,
    },
    PrivateName[LVal] {
        id: NodePtr[Identifier],
    },
    MetaProperty[LVal] {
        meta: NodePtr[Identifier],
        property: NodePtr[Identifier],
    },
    SwitchCase {
        test: Option<NodePtr>[Expression],
        consequent: NodeList[Statement],
    },
    CatchClause {
        param: Option<NodePtr>[Pattern],
        body: NodePtr[BlockStatement],
    },
    VariableDeclarator[Declaration] {
        init: Option<NodePtr>[Expression],
        id: NodePtr[Pattern],
    },
    VariableDeclaration[Declaration] {
        kind: NodeLabel,
        declarations: NodeList[VariableDeclarator],
    },
    TemplateLiteral[Expression] {
        quasis: NodeList[TemplateElement],
        expressions: NodeList[Expression],
    },
    TaggedTemplateExpression[Expression] {
        tag: NodePtr[Expression],
        quasi: NodePtr[TemplateLiteral],
    },
    TemplateElement {
        tail: bool,
        cooked: Option<StringLiteral>,
        raw: StringLiteral,
    },
    Property {
        key: NodePtr[Literal, Identifier, Expression],
        value: NodePtr[Expression],
        kind: NodeLabel,
        computed: bool,
        method: bool,
        shorthand: bool,
    },
    ClassDeclaration[Declaration] {
        id: Option<NodePtr>[Identifier],
        type_parameters: Option<NodePtr>[TypeParameterDeclaration],
        super_class: Option<NodePtr>[Expression],
        super_type_parameters: Option<NodePtr>[TypeParameterDeclaration],
        implements: NodeList[ClassImplements],
        decorators: NodeList,
        body: NodePtr[ClassBody],
    },
    ClassExpression[Expression] {
        id: Option<NodePtr>[Identifier],
        type_parameters: Option<NodePtr>[TypeParameterDeclaration],
        super_class: Option<NodePtr>[Expression],
        super_type_parameters: Option<NodePtr>[TypeParameterDeclaration],
        implements: NodeList[ClassImplements],
        decorators: NodeList,
        body: NodePtr[ClassBody],
    },
    ClassBody {
        body: NodeList[ClassProperty, ClassPrivateProperty, MethodDefinition],
    },
    ClassProperty {
        key: NodePtr[Expression],
        value: Option<NodePtr>[Expression],
        computed: bool,
        is_static: bool,
        declare: bool,
        optional: bool,
        variance: Option<NodePtr>[Variance],
        type_annotation: Option<NodePtr>[TypeAnnotation],
    },
    ClassPrivateProperty {
        key: NodePtr[PrivateName],
        value: Option<NodePtr>[Expression],
        is_static: bool,
        declare: bool,
        optional: bool,
        variance: Option<NodePtr>[Variance],
        type_annotation: Option<NodePtr>[TypeAnnotation],
    },
    MethodDefinition {
        key: NodePtr[Expression],
        value: NodePtr[FunctionExpression],
        kind: NodeLabel,
        computed: bool,
        is_static: bool,
    },
    ImportDeclaration[Declaration] {
        specifiers: NodeList[ImportSpecifier],
        source: NodePtr[StringLiteral],
        attributes: Option<NodeList>[ImportAttribute],
        import_kind: NodeLabel,
    },
    ImportSpecifier {
        imported: NodePtr[Identifier],
        local: NodePtr[Identifier],
        import_kind: NodeLabel,
    },
    ImportDefaultSpecifier {
        local: NodePtr[Identifier],
    },
    ImportNamespaceSpecifier {
        local: NodePtr[Identifier],
    },
    ImportAttribute {
        key: NodePtr[StringLiteral],
        value: NodePtr[Expression],
    },
    ExportNamedDeclaration[Declaration] {
        declaration: Option<NodePtr>[Declaration],
        specifiers: NodeList[ExportSpecifier],
        source: Option<NodePtr>[StringLiteral],
        export_kind: NodeLabel,
    },
    ExportSpecifier {
        exported: NodePtr[Identifier],
        local: NodePtr[Identifier],
    },
    ExportNamespaceSpecifier {
        exported: NodePtr[Identifier],
    },
    ExportDefaultDeclaration[Declaration] {
        declaration: NodePtr[Declaration],
    },
    ExportAllDeclaration[Declaration] {
        source: NodePtr[StringLiteral],
        export_kind: NodeLabel,
    },
    ObjectPattern[Pattern] {
        properties: NodeList[Property],
        type_annotation: Option<NodePtr>[TypeAnnotation],
    },
    ArrayPattern[Pattern] {
        elements: NodeList[Pattern],
        type_annotation: Option<NodePtr>[TypeAnnotation],
    },
    RestElement[Pattern] {
        argument: NodePtr[Pattern],
    },
    AssignmentPattern[Pattern] {
        left: NodePtr[Pattern],
        right: NodePtr[Expression],
    },

    JSXIdentifier {
        name: NodeLabel,
    },
    JSXMemberExpression {
        object: NodePtr,
        property: NodePtr,
    },
    JSXNamespacedName {
        namespace: NodePtr,
        name: NodePtr,
    },
    JSXEmptyExpression,
    JSXExpressionContainer {
        expression: NodePtr,
    },
    JSXSpreadChild {
        expression: NodePtr,
    },
    JSXOpeningElement {
        name: NodePtr,
        attributes: NodeList,
        self_closing: bool,
    },
    JSXClosingElement {
        name: NodePtr,
    },
    JSXAttribute {
        name: NodePtr,
        value: Option<NodePtr>,
    },
    JSXSpreadAttribute {
        argument: NodePtr,
    },
    JSXText {
        value: StringLiteral,
        raw: StringLiteral,
    },
    JSXElement {
        opening_element: NodePtr,
        children: NodeList,
        closing_element: Option<NodePtr>,
    },
    JSXFragment {
        opening_fragment: NodePtr,
        children: NodeList,
        closing_fragment: NodePtr,
    },
    JSXOpeningFragment,
    JSXClosingFragment,

    ExistsTypeAnnotation,
    EmptyTypeAnnotation,
    StringTypeAnnotation,
    NumberTypeAnnotation,
    StringLiteralTypeAnnotation {
        value: StringLiteral,
    },
    NumberLiteralTypeAnnotation {
        value: f64,
        raw: NodeLabel,
    },
    BooleanTypeAnnotation,
    BooleanLiteralTypeAnnotation {
        value: bool,
        raw: NodeLabel,
    },
    NullLiteralTypeAnnotation,
    SymbolTypeAnnotation,
    AnyTypeAnnotation,
    MixedTypeAnnotation,
    VoidTypeAnnotation,
    FunctionTypeAnnotation {
        params: NodeList,
        this: Option<NodePtr>,
        return_type: NodePtr,
        rest: Option<NodePtr>,
        type_parameters: Option<NodePtr>,
    },
    FunctionTypeParam {
        name: Option<NodePtr>,
        type_annotation: NodePtr,
        optional: bool,
    },
    NullableTypeAnnotation {
        type_annotation: NodePtr,
    },
    QualifiedTypeIdentifier {
        qualification: NodePtr,
        id: NodePtr,
    },
    TypeofTypeAnnotation {
        argument: NodePtr,
    },
    TupleTypeAnnotation {
        types: NodeList,
    },
    ArrayTypeAnnotation {
        element_type: NodePtr,
    },
    UnionTypeAnnotation {
        types: NodeList,
    },
    IntersectionTypeAnnotation {
        types: NodeList,
    },
    GenericTypeAnnotation {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
    },
    IndexedAccessType {
        object_type: NodePtr,
        index_type: NodePtr,
    },
    OptionalIndexedAccessType {
        object_type: NodePtr,
        index_type: NodePtr,
        optional: bool,
    },
    InterfaceTypeAnnotation {
        extends: NodeList,
        body: Option<NodePtr>,
    },
    TypeAlias {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
        right: NodePtr,
    },
    OpaqueType {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
        impltype: NodePtr,
        supertype: Option<NodePtr>,
    },
    InterfaceDeclaration {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
        extends: NodeList,
        body: NodePtr,
    },
    DeclareTypeAlias {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
        right: NodePtr,
    },
    DeclareOpaqueType {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
        impltype: Option<NodePtr>,
        supertype: Option<NodePtr>,
    },
    DeclareInterface {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
        extends: NodeList,
        body: NodePtr,
    },
    DeclareClass {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
        extends: NodeList,
        implements: NodeList,
        mixins: NodeList,
        body: NodePtr,
    },
    DeclareFunction {
        id: NodePtr,
        predicate: Option<NodePtr>,
    },
    DeclareVariable {
        id: NodePtr,
    },
    DeclareExportDeclaration {
        declaration: Option<NodePtr>,
        specifiers: NodeList,
        source: Option<NodePtr>,
        default: bool,
    },
    DeclareExportAllDeclaration {
        source: NodePtr,
    },
    DeclareModule {
        id: NodePtr,
        body: NodePtr,
        kind: NodeLabel,
    },
    DeclareModuleExports {
        type_annotation: NodePtr,
    },
    InterfaceExtends {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
    },
    ClassImplements {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
    },
    TypeAnnotation {
        type_annotation: NodePtr,
    },
    ObjectTypeAnnotation {
        properties: NodeList,
        indexers: NodeList,
        call_properties: NodeList,
        internal_slots: NodeList,
        inexact: bool,
        exact: bool,
    },
    ObjectTypeProperty {
        key: NodePtr,
        value: NodePtr,
        method: bool,
        optional: bool,
        is_static: bool,
        proto: bool,
        variance: Option<NodePtr>,
        kind: NodeLabel,
    },
    ObjectTypeSpreadProperty {
        argument: NodePtr,
    },
    ObjectTypeInternalSlot {
        id: NodePtr,
        value: NodePtr,
        optional: bool,
        is_static: bool,
        method: bool,
    },
    ObjectTypeCallProperty {
        value: NodePtr,
        is_static: bool,
    },
    ObjectTypeIndexer {
        id: Option<NodePtr>,
        key: NodePtr,
        value: NodePtr,
        is_static: bool,
        variance: Option<NodePtr>,
    },
    Variance {
        kind: NodeLabel,
    },
    TypeParameterDeclaration {
        params: NodeList,
    },
    TypeParameter {
        name: NodeLabel,
        bound: Option<NodePtr>,
        variance: Option<NodePtr>,
        default: Option<NodePtr>,
    },
    TypeParameterInstantiation {
        params: NodeList,
    },
    TypeCastExpression {
        expression: NodePtr,
        type_annotation: NodePtr,
    },
    InferredPredicate,
    DeclaredPredicate {
        value: NodePtr,
    },
    EnumDeclaration {
        id: NodePtr,
        body: NodePtr,
    },
    EnumStringBody {
        members: NodeList,
        explicit_type: bool,
        has_unknown_members: bool,
    },
    EnumNumberBody {
        members: NodeList,
        explicit_type: bool,
        has_unknown_members: bool,
    },
    EnumBooleanBody {
        members: NodeList,
        explicit_type: bool,
        has_unknown_members: bool,
    },
    EnumSymbolBody {
        members: NodeList,
        has_unknown_members: bool,
    },
    EnumDefaultedMember {
        id: NodePtr,
    },
    EnumStringMember {
        id: NodePtr,
        init: NodePtr,
    },
    EnumNumberMember {
        id: NodePtr,
        init: NodePtr,
    },
    EnumBooleanMember {
        id: NodePtr,
        init: NodePtr,
    },

    TSTypeAnnotation {
        type_annotation: NodePtr,
    },
    TSAnyKeyword,
    TSNumberKeyword,
    TSBooleanKeyword,
    TSStringKeyword,
    TSSymbolKeyword,
    TSVoidKeyword,
    TSThisType,
    TSLiteralType {
        literal: NodePtr,
    },
    TSIndexedAccessType {
        object_type: NodePtr,
        index_type: NodePtr,
    },
    TSArrayType {
        element_type: NodePtr,
    },
    TSTypeReference {
        type_name: NodePtr,
        type_parameters: Option<NodePtr>,
    },
    TSQualifiedName {
        left: NodePtr,
        right: Option<NodePtr>,
    },
    TSFunctionType {
        params: NodeList,
        return_type: NodePtr,
        type_parameters: Option<NodePtr>,
    },
    TSConstructorType {
        params: NodeList,
        return_type: NodePtr,
        type_parameters: Option<NodePtr>,
    },
    TSTypePredicate {
        parameter_name: NodePtr,
        type_annotation: NodePtr,
    },
    TSTupleType {
        element_types: NodeList,
    },
    TSTypeAssertion {
        type_annotation: NodePtr,
        expression: NodePtr,
    },
    TSAsExpression {
        expression: NodePtr,
        type_annotation: NodePtr,
    },
    TSParameterProperty {
        parameter: NodePtr,
        accessibility: Option<NodeLabel>,
        readonly: bool,
        is_static: bool,
        export: bool,
    },
    TSTypeAliasDeclaration {
        id: NodePtr,
        type_parameters: Option<NodePtr>,
        type_annotation: NodePtr,
    },
    TSInterfaceDeclaration {
        id: NodePtr,
        body: NodePtr,
        extends: NodeList,
        type_parameters: Option<NodePtr>,
    },
    TSInterfaceHeritage {
        expression: NodePtr,
        type_parameters: Option<NodePtr>,
    },
    TSInterfaceBody {
        body: NodeList,
    },
    TSEnumDeclaration {
        id: NodePtr,
        members: NodeList,
    },
    TSEnumMember {
        id: NodePtr,
        initializer: Option<NodePtr>,
    },
    TSModuleDeclaration {
        id: NodePtr,
        body: NodePtr,
    },
    TSModuleBlock {
        body: NodeList,
    },
    TSModuleMember {
        id: NodePtr,
        initializer: Option<NodePtr>,
    },
    TSTypeParameterDeclaration {
        params: NodeList,
    },
    TSTypeParameter {
        name: NodePtr,
        constraint: Option<NodePtr>,
        default: Option<NodePtr>,
    },
    TSTypeParameterInstantiation {
        params: NodeList,
    },
    TSUnionType {
        types: NodeList,
    },
    TSIntersectionType {
        types: NodeList,
    },
    TSTypeQuery {
        expr_name: NodePtr,
    },
    TSConditionalType {
        extends_type: NodePtr,
        check_type: NodePtr,
        true_type: NodePtr,
        false_t_ype: NodePtr,
    },
    TSTypeLiteral {
        members: NodeList,
    },
    TSPropertySignature {
        key: NodePtr,
        type_annotation: Option<NodePtr>,
        initializer: Option<NodePtr>,
        optional: bool,
        computed: bool,
        readonly: bool,
        is_static: bool,
        export: bool,
    },
    TSMethodSignature {
        key: NodePtr,
        params: NodeList,
        return_type: Option<NodePtr>,
        computed: bool,
    },
    TSIndexSignature {
        parameters: NodeList,
        type_annotation: Option<NodePtr>,
    },
    TSCallSignatureDeclaration {
        params: NodeList,
        return_type: Option<NodePtr>,
    }
}
}
