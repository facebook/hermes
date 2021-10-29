/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// A reusable macro for defining code based on the AST.
/// Takes a `$callback` such as `gen_nodekind_enum`, which is invoked like this:
/// ```ignore
/// $callback! {
/// NodeKind {
///     Node1[parent] {
///       field1: type1[constraint_a, constraint_b],
///       field2: type2,
///     },
///     Node2,
/// }
/// }
/// ```
/// in order to avoid excessive copy/paste boilerplate.
/// Parents and constraints can be any member of `NodeVariant`,
/// which includes any constructible `NodeKind`s` as well as interfaces like
/// `Statement`, `Expression`, etc.
/// If multiple constraints are provided, at least one must be satisfied.
/// The `null` constraint is encoded via `Option`, it need not be listed explicitly.
/// See [`gen_nodekind_enum`] for an example of how to pattern match on the information
/// provided by this macro.
macro_rules! nodekind_defs {
    ($callback:ident) => {
        $callback! {
        NodeKind {
            Empty,
            Metadata,
            Program {
                body: NodeList<'a>[Directive, Statement],
            },
            FunctionExpression[Expression] {
                id: Option<&'a Node<'a>>[Identifier],
                params: NodeList<'a>[Pattern],
                body: &'a Node<'a>[BlockStatement],
                type_parameters: Option<&'a Node<'a>>[TypeParameterDeclaration],
                return_type: Option<&'a Node<'a>>[TypeAnnotation],
                predicate: Option<&'a Node<'a>>[InferredPredicate, DeclaredPredicate],
                generator: bool,
                is_async: bool,
            },
            ArrowFunctionExpression[Expression] {
                id: Option<&'a Node<'a>>[Identifier],
                params: NodeList<'a>[Pattern],
                body: &'a Node<'a>[BlockStatement],
                type_parameters: Option<&'a Node<'a>>[TypeParameterDeclaration],
                return_type: Option<&'a Node<'a>>[TypeAnnotation],
                predicate: Option<&'a Node<'a>>[InferredPredicate, DeclaredPredicate],
                expression: bool,
                is_async: bool,
            },
            FunctionDeclaration[Declaration] {
                id: Option<&'a Node<'a>>[Identifier],
                params: NodeList<'a>[Pattern],
                body: &'a Node<'a>[BlockStatement],
                type_parameters: Option<&'a Node<'a>>[TypeParameterDeclaration],
                return_type: Option<&'a Node<'a>>[TypeAnnotation],
                predicate: Option<&'a Node<'a>>[InferredPredicate, DeclaredPredicate],
                generator: bool,
                is_async: bool,
            },
            WhileStatement[Statement] {
                body: &'a Node<'a>[Statement],
                test: &'a Node<'a>[Expression],
            },
            DoWhileStatement[Statement] {
                body: &'a Node<'a>[Statement],
                test: &'a Node<'a>[Expression],
            },
            ForInStatement[Statement] {
                left: &'a Node<'a>[VariableDeclaration, Pattern],
                right: &'a Node<'a>[Expression],
                body: &'a Node<'a>[Statement],
            },
            ForOfStatement[ForInStatement] {
                left: &'a Node<'a>[VariableDeclaration, Pattern],
                right: &'a Node<'a>[Expression],
                body: &'a Node<'a>[Statement],
                is_await: bool,
            },
            ForStatement[Statement] {
                init: Option<&'a Node<'a>>[VariableDeclaration, Expression],
                test: Option<&'a Node<'a>>[Expression],
                update: Option<&'a Node<'a>>[Expression],
                body: &'a Node<'a>[Statement],
            },
            DebuggerStatement[Statement],
            EmptyStatement[Statement],
            BlockStatement[Statement] {
                body: NodeList<'a>[Statement],
            },
            BreakStatement[Statement] {
                label: Option<&'a Node<'a>>[Identifier],
            },
            ContinueStatement[Statement] {
                label: Option<&'a Node<'a>>[Identifier],
            },
            ThrowStatement[Statement] {
                argument: &'a Node<'a>[Expression],
            },
            ReturnStatement[Statement] {
                argument: Option<&'a Node<'a>>[Expression],
            },
            WithStatement[Statement] {
                object: &'a Node<'a>[Expression],
                body: &'a Node<'a>[Statement],
            },
            SwitchStatement[Statement] {
                discriminant: &'a Node<'a>[Expression],
                cases: NodeList<'a>[SwitchCase],
            },
            LabeledStatement[Statement] {
                label: &'a Node<'a>[Identifier],
                body: &'a Node<'a>[Statement],
            },
            ExpressionStatement[Statement] {
                expression: &'a Node<'a>[Expression],
                directive: Option<NodeString>,
            },
            TryStatement[Statement] {
                block: &'a Node<'a>[BlockStatement],
                handler: Option<&'a Node<'a>>[CatchClause],
                finalizer: Option<&'a Node<'a>>[BlockStatement],
            },
            IfStatement[Statement] {
                test: &'a Node<'a>[Expression],
                consequent: &'a Node<'a>[Statement],
                alternate: Option<&'a Node<'a>>[Statement],
            },
            NullLiteral[Literal],
            BooleanLiteral[Literal] {
                value: bool,
            },
            StringLiteral[Literal] {
                value: NodeString,
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
                expressions: NodeList<'a>[Expression],
            },
            ObjectExpression[Expression] {
                properties: NodeList<'a>[Property],
            },
            ArrayExpression[Expression] {
                elements: NodeList<'a>[Expression, SpreadElement],
                trailing_comma: bool,
            },
            SpreadElement {
                argument: &'a Node<'a>[Expression],
            },
            NewExpression[Expression] {
                callee: &'a Node<'a>[Expression],
                type_arguments: Option<&'a Node<'a>>[TypeParameterInstantiation],
                arguments: NodeList<'a>[Expression, SpreadElement],
            },
            YieldExpression[Expression] {
                argument: Option<&'a Node<'a>>[Expression],
                delegate: bool,
            },
            AwaitExpression[Expression] {
                argument: &'a Node<'a>[Expression],
            },
            ImportExpression[Expression] {
                source: &'a Node<'a>[Expression],
                attributes: Option<&'a Node<'a>>[Expression],
            },
            CallExpression[Expression] {
                callee: &'a Node<'a>[Expression, Super],
                type_arguments: Option<&'a Node<'a>>[TypeParameterInstantiation],
                arguments: NodeList<'a>[Expression],
            },
            OptionalCallExpression[Expression] {
                callee: &'a Node<'a>[Expression, Super],
                type_arguments: Option<&'a Node<'a>>[TypeParameterInstantiation],
                arguments: NodeList<'a>[Expression],
                optional: bool,
            },
            AssignmentExpression[Expression] {
                operator: AssignmentExpressionOperator,
                left: &'a Node<'a>[LVal],
                right: &'a Node<'a>[Expression],
            },
            UnaryExpression[Expression] {
                operator: UnaryExpressionOperator,
                argument: &'a Node<'a>[Expression],
                prefix: bool,
            },
            UpdateExpression[Expression] {
                operator: UpdateExpressionOperator,
                argument: &'a Node<'a>[Expression],
                prefix: bool,
            },
            MemberExpression[LVal] {
                object: &'a Node<'a>[Expression],
                property: &'a Node<'a>[Expression],
                computed: bool,
            },
            OptionalMemberExpression[Expression] {
                object: &'a Node<'a>[Expression],
                property: &'a Node<'a>[Expression],
                computed: bool,
                optional: bool,
            },
            LogicalExpression[Expression] {
                left: &'a Node<'a>[Expression],
                right: &'a Node<'a>[Expression],
                operator: LogicalExpressionOperator,
            },
            ConditionalExpression[Expression] {
                test: &'a Node<'a>[Expression],
                alternate: &'a Node<'a>[Expression],
                consequent: &'a Node<'a>[Expression],
            },
            BinaryExpression[Expression] {
                left: &'a Node<'a>[Expression],
                right: &'a Node<'a>[Expression],
                operator: BinaryExpressionOperator,
            },
            Directive[Statement] {
                value: &'a Node<'a>,
            },
            DirectiveLiteral[Literal] {
                value: NodeString,
            },
            Identifier[Pattern] {
                name: NodeLabel,
                type_annotation: Option<&'a Node<'a>>[TypeAnnotation],
                optional: bool,
            },
            PrivateName[LVal] {
                id: &'a Node<'a>[Identifier],
            },
            MetaProperty[LVal] {
                meta: &'a Node<'a>[Identifier],
                property: &'a Node<'a>[Identifier],
            },
            SwitchCase {
                test: Option<&'a Node<'a>>[Expression],
                consequent: NodeList<'a>[Statement],
            },
            CatchClause {
                param: Option<&'a Node<'a>>[Pattern],
                body: &'a Node<'a>[BlockStatement],
            },
            VariableDeclarator[Declaration] {
                init: Option<&'a Node<'a>>[Expression],
                id: &'a Node<'a>[Pattern],
            },
            VariableDeclaration[Declaration] {
                kind: VariableDeclarationKind,
                declarations: NodeList<'a>[VariableDeclarator],
            },
            TemplateLiteral[Expression] {
                quasis: NodeList<'a>[TemplateElement],
                expressions: NodeList<'a>[Expression],
            },
            TaggedTemplateExpression[Expression] {
                tag: &'a Node<'a>[Expression],
                quasi: &'a Node<'a>[TemplateLiteral],
            },
            TemplateElement {
                tail: bool,
                cooked: Option<NodeString>,
                raw: NodeLabel,
            },
            Property {
                key: &'a Node<'a>[Literal, Identifier, Expression],
                value: &'a Node<'a>[Expression],
                kind: PropertyKind,
                computed: bool,
                method: bool,
                shorthand: bool,
            },
            ClassDeclaration[Declaration] {
                id: Option<&'a Node<'a>>[Identifier],
                type_parameters: Option<&'a Node<'a>>[TypeParameterDeclaration],
                super_class: Option<&'a Node<'a>>[Expression],
                super_type_parameters: Option<&'a Node<'a>>[TypeParameterDeclaration],
                implements: NodeList<'a>[ClassImplements],
                decorators: NodeList<'a>,
                body: &'a Node<'a>[ClassBody],
            },
            ClassExpression[Expression] {
                id: Option<&'a Node<'a>>[Identifier],
                type_parameters: Option<&'a Node<'a>>[TypeParameterDeclaration],
                super_class: Option<&'a Node<'a>>[Expression],
                super_type_parameters: Option<&'a Node<'a>>[TypeParameterDeclaration],
                implements: NodeList<'a>[ClassImplements],
                decorators: NodeList<'a>,
                body: &'a Node<'a>[ClassBody],
            },
            ClassBody {
                body: NodeList<'a>[ClassProperty, ClassPrivateProperty, MethodDefinition],
            },
            ClassProperty {
                key: &'a Node<'a>[Expression],
                value: Option<&'a Node<'a>>[Expression],
                computed: bool,
                is_static: bool,
                declare: bool,
                optional: bool,
                variance: Option<&'a Node<'a>>[Variance],
                type_annotation: Option<&'a Node<'a>>[TypeAnnotation],
            },
            ClassPrivateProperty {
                key: &'a Node<'a>[PrivateName],
                value: Option<&'a Node<'a>>[Expression],
                is_static: bool,
                declare: bool,
                optional: bool,
                variance: Option<&'a Node<'a>>[Variance],
                type_annotation: Option<&'a Node<'a>>[TypeAnnotation],
            },
            MethodDefinition {
                key: &'a Node<'a>[Expression],
                value: &'a Node<'a>[FunctionExpression],
                kind: MethodDefinitionKind,
                computed: bool,
                is_static: bool,
            },
            ImportDeclaration[Declaration] {
                specifiers: NodeList<'a>[ImportSpecifier],
                source: &'a Node<'a>[StringLiteral],
                assertions: Option<NodeList<'a>>[ImportAttribute],
                import_kind: ImportKind,
            },
            ImportSpecifier {
                imported: &'a Node<'a>[Identifier],
                local: &'a Node<'a>[Identifier],
                import_kind: ImportKind,
            },
            ImportDefaultSpecifier {
                local: &'a Node<'a>[Identifier],
            },
            ImportNamespaceSpecifier {
                local: &'a Node<'a>[Identifier],
            },
            ImportAttribute {
                key: &'a Node<'a>[StringLiteral],
                value: &'a Node<'a>[Expression],
            },
            ExportNamedDeclaration[Declaration] {
                declaration: Option<&'a Node<'a>>[Declaration],
                specifiers: NodeList<'a>[ExportSpecifier],
                source: Option<&'a Node<'a>>[StringLiteral],
                export_kind: ExportKind,
            },
            ExportSpecifier {
                exported: &'a Node<'a>[Identifier],
                local: &'a Node<'a>[Identifier],
            },
            ExportNamespaceSpecifier {
                exported: &'a Node<'a>[Identifier],
            },
            ExportDefaultDeclaration[Declaration] {
                declaration: &'a Node<'a>[Declaration],
            },
            ExportAllDeclaration[Declaration] {
                source: &'a Node<'a>[StringLiteral],
                export_kind: ExportKind,
            },
            ObjectPattern[Pattern] {
                properties: NodeList<'a>[Property],
                type_annotation: Option<&'a Node<'a>>[TypeAnnotation],
            },
            ArrayPattern[Pattern] {
                elements: NodeList<'a>[Pattern],
                type_annotation: Option<&'a Node<'a>>[TypeAnnotation],
            },
            RestElement[Pattern] {
                argument: &'a Node<'a>[Pattern],
            },
            AssignmentPattern[Pattern] {
                left: &'a Node<'a>[Pattern],
                right: &'a Node<'a>[Expression],
            },

            JSXIdentifier {
                name: NodeLabel,
            },
            JSXMemberExpression {
                object: &'a Node<'a>,
                property: &'a Node<'a>,
            },
            JSXNamespacedName {
                namespace: &'a Node<'a>,
                name: &'a Node<'a>,
            },
            JSXEmptyExpression,
            JSXExpressionContainer {
                expression: &'a Node<'a>,
            },
            JSXSpreadChild {
                expression: &'a Node<'a>,
            },
            JSXOpeningElement {
                name: &'a Node<'a>,
                attributes: NodeList<'a>[JSXAttribute, JSXSpreadAttribute],
                self_closing: bool,
            },
            JSXClosingElement {
                name: &'a Node<'a>,
            },
            JSXAttribute {
                name: &'a Node<'a>,
                value: Option<&'a Node<'a>>,
            },
            JSXSpreadAttribute {
                argument: &'a Node<'a>,
            },
            JSXText {
                value: NodeString,
                raw: NodeLabel,
            },
            JSXElement[Expression] {
                opening_element: &'a Node<'a>[JSXOpeningElement],
                children: NodeList<'a>,
                closing_element: Option<&'a Node<'a>>[JSXClosingElement],
            },
            JSXFragment[Expression] {
                opening_fragment: &'a Node<'a>[JSXOpeningFragment],
                children: NodeList<'a>,
                closing_fragment: &'a Node<'a>[JSXClosingFragment],
            },
            JSXOpeningFragment,
            JSXClosingFragment,

            ExistsTypeAnnotation,
            EmptyTypeAnnotation,
            StringTypeAnnotation,
            NumberTypeAnnotation,
            StringLiteralTypeAnnotation {
                value: NodeString,
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
                params: NodeList<'a>,
                this: Option<&'a Node<'a>>,
                return_type: &'a Node<'a>,
                rest: Option<&'a Node<'a>>,
                type_parameters: Option<&'a Node<'a>>,
            },
            FunctionTypeParam {
                name: Option<&'a Node<'a>>,
                type_annotation: &'a Node<'a>,
                optional: bool,
            },
            NullableTypeAnnotation {
                type_annotation: &'a Node<'a>,
            },
            QualifiedTypeIdentifier {
                qualification: &'a Node<'a>,
                id: &'a Node<'a>,
            },
            TypeofTypeAnnotation {
                argument: &'a Node<'a>,
            },
            TupleTypeAnnotation {
                types: NodeList<'a>,
            },
            ArrayTypeAnnotation {
                element_type: &'a Node<'a>,
            },
            UnionTypeAnnotation {
                types: NodeList<'a>,
            },
            IntersectionTypeAnnotation {
                types: NodeList<'a>,
            },
            GenericTypeAnnotation {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
            },
            IndexedAccessType {
                object_type: &'a Node<'a>,
                index_type: &'a Node<'a>,
            },
            OptionalIndexedAccessType {
                object_type: &'a Node<'a>,
                index_type: &'a Node<'a>,
                optional: bool,
            },
            InterfaceTypeAnnotation {
                extends: NodeList<'a>,
                body: Option<&'a Node<'a>>,
            },
            TypeAlias {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
                right: &'a Node<'a>,
            },
            OpaqueType {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
                impltype: &'a Node<'a>,
                supertype: Option<&'a Node<'a>>,
            },
            InterfaceDeclaration {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
                extends: NodeList<'a>,
                body: &'a Node<'a>,
            },
            DeclareTypeAlias {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
                right: &'a Node<'a>,
            },
            DeclareOpaqueType {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
                impltype: Option<&'a Node<'a>>,
                supertype: Option<&'a Node<'a>>,
            },
            DeclareInterface {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
                extends: NodeList<'a>,
                body: &'a Node<'a>,
            },
            DeclareClass {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
                extends: NodeList<'a>,
                implements: NodeList<'a>,
                mixins: NodeList<'a>,
                body: &'a Node<'a>,
            },
            DeclareFunction {
                id: &'a Node<'a>,
                predicate: Option<&'a Node<'a>>,
            },
            DeclareVariable {
                id: &'a Node<'a>,
            },
            DeclareExportDeclaration {
                declaration: Option<&'a Node<'a>>,
                specifiers: NodeList<'a>,
                source: Option<&'a Node<'a>>,
                default: bool,
            },
            DeclareExportAllDeclaration {
                source: &'a Node<'a>,
            },
            DeclareModule {
                id: &'a Node<'a>,
                body: &'a Node<'a>,
                kind: NodeLabel,
            },
            DeclareModuleExports {
                type_annotation: &'a Node<'a>,
            },
            InterfaceExtends {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
            },
            ClassImplements {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
            },
            TypeAnnotation {
                type_annotation: &'a Node<'a>,
            },
            ObjectTypeAnnotation {
                properties: NodeList<'a>,
                indexers: NodeList<'a>,
                call_properties: NodeList<'a>,
                internal_slots: NodeList<'a>,
                inexact: bool,
                exact: bool,
            },
            ObjectTypeProperty {
                key: &'a Node<'a>,
                value: &'a Node<'a>,
                method: bool,
                optional: bool,
                is_static: bool,
                proto: bool,
                variance: Option<&'a Node<'a>>,
                kind: NodeLabel,
            },
            ObjectTypeSpreadProperty {
                argument: &'a Node<'a>,
            },
            ObjectTypeInternalSlot {
                id: &'a Node<'a>,
                value: &'a Node<'a>,
                optional: bool,
                is_static: bool,
                method: bool,
            },
            ObjectTypeCallProperty {
                value: &'a Node<'a>,
                is_static: bool,
            },
            ObjectTypeIndexer {
                id: Option<&'a Node<'a>>,
                key: &'a Node<'a>,
                value: &'a Node<'a>,
                is_static: bool,
                variance: Option<&'a Node<'a>>,
            },
            Variance {
                kind: NodeLabel,
            },
            TypeParameterDeclaration {
                params: NodeList<'a>,
            },
            TypeParameter {
                name: NodeLabel,
                bound: Option<&'a Node<'a>>,
                variance: Option<&'a Node<'a>>,
                default: Option<&'a Node<'a>>,
            },
            TypeParameterInstantiation {
                params: NodeList<'a>,
            },
            TypeCastExpression {
                expression: &'a Node<'a>,
                type_annotation: &'a Node<'a>,
            },
            InferredPredicate,
            DeclaredPredicate {
                value: &'a Node<'a>,
            },
            EnumDeclaration {
                id: &'a Node<'a>,
                body: &'a Node<'a>,
            },
            EnumStringBody {
                members: NodeList<'a>,
                explicit_type: bool,
                has_unknown_members: bool,
            },
            EnumNumberBody {
                members: NodeList<'a>,
                explicit_type: bool,
                has_unknown_members: bool,
            },
            EnumBooleanBody {
                members: NodeList<'a>,
                explicit_type: bool,
                has_unknown_members: bool,
            },
            EnumSymbolBody {
                members: NodeList<'a>,
                has_unknown_members: bool,
            },
            EnumDefaultedMember {
                id: &'a Node<'a>,
            },
            EnumStringMember {
                id: &'a Node<'a>,
                init: &'a Node<'a>,
            },
            EnumNumberMember {
                id: &'a Node<'a>,
                init: &'a Node<'a>,
            },
            EnumBooleanMember {
                id: &'a Node<'a>,
                init: &'a Node<'a>,
            },

            TSTypeAnnotation {
                type_annotation: &'a Node<'a>,
            },
            TSAnyKeyword,
            TSNumberKeyword,
            TSBooleanKeyword,
            TSStringKeyword,
            TSSymbolKeyword,
            TSVoidKeyword,
            TSThisType,
            TSLiteralType {
                literal: &'a Node<'a>,
            },
            TSIndexedAccessType {
                object_type: &'a Node<'a>,
                index_type: &'a Node<'a>,
            },
            TSArrayType {
                element_type: &'a Node<'a>,
            },
            TSTypeReference {
                type_name: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
            },
            TSQualifiedName {
                left: &'a Node<'a>,
                right: Option<&'a Node<'a>>,
            },
            TSFunctionType {
                params: NodeList<'a>,
                return_type: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
            },
            TSConstructorType {
                params: NodeList<'a>,
                return_type: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
            },
            TSTypePredicate {
                parameter_name: &'a Node<'a>,
                type_annotation: &'a Node<'a>,
            },
            TSTupleType {
                element_types: NodeList<'a>,
            },
            TSTypeAssertion {
                type_annotation: &'a Node<'a>,
                expression: &'a Node<'a>,
            },
            TSAsExpression {
                expression: &'a Node<'a>,
                type_annotation: &'a Node<'a>,
            },
            TSParameterProperty {
                parameter: &'a Node<'a>,
                accessibility: Option<NodeLabel>,
                readonly: bool,
                is_static: bool,
                export: bool,
            },
            TSTypeAliasDeclaration {
                id: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
                type_annotation: &'a Node<'a>,
            },
            TSInterfaceDeclaration {
                id: &'a Node<'a>,
                body: &'a Node<'a>,
                extends: NodeList<'a>,
                type_parameters: Option<&'a Node<'a>>,
            },
            TSInterfaceHeritage {
                expression: &'a Node<'a>,
                type_parameters: Option<&'a Node<'a>>,
            },
            TSInterfaceBody {
                body: NodeList<'a>,
            },
            TSEnumDeclaration {
                id: &'a Node<'a>,
                members: NodeList<'a>,
            },
            TSEnumMember {
                id: &'a Node<'a>,
                initializer: Option<&'a Node<'a>>,
            },
            TSModuleDeclaration {
                id: &'a Node<'a>,
                body: &'a Node<'a>,
            },
            TSModuleBlock {
                body: NodeList<'a>,
            },
            TSModuleMember {
                id: &'a Node<'a>,
                initializer: Option<&'a Node<'a>>,
            },
            TSTypeParameterDeclaration {
                params: NodeList<'a>,
            },
            TSTypeParameter {
                name: &'a Node<'a>,
                constraint: Option<&'a Node<'a>>,
                default: Option<&'a Node<'a>>,
            },
            TSTypeParameterInstantiation {
                params: NodeList<'a>,
            },
            TSUnionType {
                types: NodeList<'a>,
            },
            TSIntersectionType {
                types: NodeList<'a>,
            },
            TSTypeQuery {
                expr_name: &'a Node<'a>,
            },
            TSConditionalType {
                extends_type: &'a Node<'a>,
                check_type: &'a Node<'a>,
                true_type: &'a Node<'a>,
                false_t_ype: &'a Node<'a>,
            },
            TSTypeLiteral {
                members: NodeList<'a>,
            },
            TSPropertySignature {
                key: &'a Node<'a>,
                type_annotation: Option<&'a Node<'a>>,
                initializer: Option<&'a Node<'a>>,
                optional: bool,
                computed: bool,
                readonly: bool,
                is_static: bool,
                export: bool,
            },
            TSMethodSignature {
                key: &'a Node<'a>,
                params: NodeList<'a>,
                return_type: Option<&'a Node<'a>>,
                computed: bool,
            },
            TSIndexSignature {
                parameters: NodeList<'a>,
                type_annotation: Option<&'a Node<'a>>,
            },
            TSCallSignatureDeclaration {
                params: NodeList<'a>,
                return_type: Option<&'a Node<'a>>,
            }
        }
        }
    };
}
