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
            Literal,
            $($kind),*
        }

        impl NodeVariant {
            /// The `parent` of the variant in ESTree, used for validation.
            /// Return `None` if there is no parent.
            pub fn parent(&self) -> Option<NodeVariant> {
                match self {
                    Self::Expression => None,
                    Self::Statement => None,
                    Self::Literal => Some(Self::Expression),
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
        body: NodeList,
    },
    FunctionExpression {
        id: Option<NodePtr>,
        params: NodeList,
        body: NodePtr,
        type_parameters: Option<NodePtr>,
        return_type: Option<NodePtr>,
        predicate: Option<NodePtr>,
        generator: bool,
        is_async: bool,
    },
    ArrowFunctionExpression {
        id: Option<NodePtr>,
        params: NodeList,
        body: NodePtr,
        type_parameters: Option<NodePtr>,
        return_type: Option<NodePtr>,
        predicate: Option<NodePtr>,
        expression: bool,
        is_async: bool,
    },
    FunctionDeclaration {
        id: Option<NodePtr>,
        params: NodeList,
        body: NodePtr,
        type_parameters: Option<NodePtr>,
        return_type: Option<NodePtr>,
        predicate: Option<NodePtr>,
        generator: bool,
        is_async: bool,
    },
    WhileStatement {
        body: NodePtr,
        test: NodePtr,
    },
    DoWhileStatement {
        body: NodePtr,
        test: NodePtr,
    },
    ForInStatement {
        left: NodePtr,
        right: NodePtr,
        body: NodePtr,
    },
    ForOfStatement {
        left: NodePtr,
        right: NodePtr,
        body: NodePtr,
        is_await: bool,
    },
    ForStatement {
        init: Option<NodePtr>,
        test: Option<NodePtr>,
        update: Option<NodePtr>,
        body: NodePtr,
    },
    DebuggerStatement,
    EmptyStatement,
    BlockStatement {
        body: NodeList,
    },
    BreakStatement {
        label: Option<NodePtr>,
    },
    ContinueStatement {
        label: Option<NodePtr>,
    },
    ThrowStatement {
        argument: NodePtr,
    },
    ReturnStatement[Statement] {
        argument: Option<NodePtr>[Expression],
    },
    WithStatement {
        object: NodePtr,
        body: NodePtr,
    },
    SwitchStatement {
        discriminant: NodePtr,
        cases: NodeList,
    },
    LabeledStatement {
        label: NodePtr,
        body: NodePtr,
    },
    ExpressionStatement {
        expression: NodePtr,
        directive: Option<StringLiteral>,
    },
    TryStatement {
        block: NodePtr,
        handler: Option<NodePtr>,
        finalizer: Option<NodePtr>,
    },
    IfStatement {
        test: NodePtr,
        consequent: NodePtr,
        alternate: Option<NodePtr>,
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
    SequenceExpression {
        expressions: NodeList,
    },
    ObjectExpression {
        properties: NodeList,
    },
    ArrayExpression {
        elements: NodeList,
        trailing_comma: bool,
    },
    SpreadElement {
        argument: NodePtr,
    },
    NewExpression {
        callee: NodePtr,
        type_arguments: Option<NodePtr>,
        arguments: NodeList,
    },
    YieldExpression {
        argument: Option<NodePtr>,
        delegate: bool,
    },
    AwaitExpression {
        argument: NodePtr,
    },
    ImportExpression {
        source: NodePtr,
        attributes: Option<NodePtr>,
    },
    CallExpression {
        callee: NodePtr,
        type_arguments: Option<NodePtr>,
        arguments: NodeList,
    },
    OptionalCallExpression {
        callee: NodePtr,
        type_arguments: Option<NodePtr>,
        arguments: NodeList,
        optional: bool,
    },
    AssignmentExpression {
        operator: NodeLabel,
        left: NodePtr,
        right: NodePtr,
    },
    UnaryExpression {
        operator: NodeLabel,
        argument: NodePtr,
        prefix: bool,
    },
    UpdateExpression {
        operator: NodeLabel,
        argument: NodePtr,
        prefix: bool,
    },
    MemberExpression {
        object: NodePtr,
        property: NodePtr,
        computed: bool,
    },
    OptionalMemberExpression {
        object: NodePtr,
        property: NodePtr,
        computed: bool,
        optional: bool,
    },
    LogicalExpression {
        left: NodePtr,
        right: NodePtr,
        operator: NodeLabel,
    },
    ConditionalExpression {
        test: NodePtr,
        alternate: NodePtr,
        consequent: NodePtr,
    },
    BinaryExpression {
        left: NodePtr,
        right: NodePtr,
        operator: NodeLabel,
    },
    Directive {
        value: NodePtr,
    },
    DirectiveLiteral {
        value: StringLiteral,
    },
    Identifier {
        name: NodeLabel,
        type_annotation: Option<NodePtr>,
        optional: bool,
    },
    PrivateName {
        id: NodePtr,
    },
    MetaProperty {
        meta: NodePtr,
        property: NodePtr,
    },
    SwitchCase {
        test: Option<NodePtr>,
        consequent: NodeList,
    },
    CatchClause {
        param: Option<NodePtr>,
        body: NodePtr,
    },
    VariableDeclarator {
        init: Option<NodePtr>,
        id: NodePtr,
    },
    VariableDeclaration {
        kind: NodeLabel,
        declarations: NodeList,
    },
    TemplateLiteral {
        quasis: NodeList,
        expressions: NodeList,
    },
    TaggedTemplateExpression {
        tag: NodePtr,
        quasi: NodePtr,
    },
    TemplateElement {
        tail: bool,
        cooked: Option<StringLiteral>,
        raw: StringLiteral,
    },
    Property {
        key: NodePtr,
        value: NodePtr,
        kind: NodeLabel,
        computed: bool,
        method: bool,
        shorthand: bool,
    },
    ClassDeclaration {
        id: Option<NodePtr>,
        type_parameters: Option<NodePtr>,
        super_class: Option<NodePtr>,
        super_type_parameters: Option<NodePtr>,
        implements: NodeList,
        decorators: NodeList,
        body: NodePtr,
    },
    ClassExpression {
        id: Option<NodePtr>,
        type_parameters: Option<NodePtr>,
        super_class: Option<NodePtr>,
        super_type_parameters: Option<NodePtr>,
        implements: NodeList,
        decorators: NodeList,
        body: NodePtr,
    },
    ClassBody {
        body: NodeList,
    },
    ClassProperty {
        key: NodePtr,
        value: Option<NodePtr>,
        computed: bool,
        is_static: bool,
        declare: bool,
        optional: bool,
        variance: Option<NodePtr>,
        type_annotation: Option<NodePtr>,
    },
    ClassPrivateProperty {
        key: NodePtr,
        value: Option<NodePtr>,
        is_static: bool,
        declare: bool,
        optional: bool,
        variance: Option<NodePtr>,
        type_annotation: Option<NodePtr>,
    },
    MethodDefinition {
        key: NodePtr,
        value: NodePtr,
        kind: NodeLabel,
        computed: bool,
        is_static: bool,
    },
    ImportDeclaration {
        specifiers: NodeList,
        source: NodePtr,
        attributes: Option<NodeList>,
        import_kind: NodeLabel,
    },
    ImportSpecifier {
        imported: NodePtr,
        local: NodePtr,
        import_kind: NodeLabel,
    },
    ImportDefaultSpecifier {
        local: NodePtr,
    },
    ImportNamespaceSpecifier {
        local: NodePtr,
    },
    ImportAttribute {
        key: NodePtr,
        value: NodePtr,
    },
    ExportNamedDeclaration {
        declaration: Option<NodePtr>,
        specifiers: NodeList,
        source: Option<NodePtr>,
        export_kind: NodeLabel,
    },
    ExportSpecifier {
        exported: NodePtr,
        local: NodePtr,
    },
    ExportNamespaceSpecifier {
        exported: NodePtr,
    },
    ExportDefaultDeclaration {
        declaration: NodePtr,
    },
    ExportAllDeclaration {
        source: NodePtr,
        export_kind: NodeLabel,
    },
    ObjectPattern {
        properties: NodeList,
        type_annotation: Option<NodePtr>,
    },
    ArrayPattern {
        elements: NodeList,
        type_annotation: Option<NodePtr>,
    },
    RestElement {
        argument: NodePtr,
    },
    AssignmentPattern {
        left: NodePtr,
        right: NodePtr,
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
