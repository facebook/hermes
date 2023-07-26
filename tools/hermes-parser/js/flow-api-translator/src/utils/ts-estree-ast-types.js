/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

/**
 * The following types have been adapted by hand from
 * https://unpkg.com/browse/@typescript-eslint/types@5.41.0/dist/generated/ast-spec.d.ts
 *
 * Changes:
 * - remove and inline `ValueOf` type
 * - `undefined` -> `void`
 * - remove all `declare` keywords
 * - comment out `bigint` type
 *     -> flow doesn't support it yet
 * - remove `range` and `loc` from `NodeOrTokenData`
 *     -> during conversion our locations will be all off, so we'll rely on prettier to print later
 * - make all properties readonly and all arrays $ReadOnlyArray
 *     -> unlike TS - flow enforces subtype constraints strictly!
 * - add `type` to interfaces that previously relied upon inheriting the `type`
 *     -> this is because flow sentinel refinement does not check inherited members
 * - create "Ambiguous" versions for some nodes that have unions (like PropertyDefinition, MemberDefinition)
 *     -> makes it easier to construct them from other nodes that have unions
 */

'use strict';

interface NodeOrTokenData {}
interface BaseNode extends NodeOrTokenData {}
interface BaseToken extends NodeOrTokenData {
  +value: string;
}

export type Accessibility = 'private' | 'protected' | 'public';
export interface ArrayExpression extends BaseNode {
  +type: 'ArrayExpression';
  +elements: $ReadOnlyArray<Expression | SpreadElement>;
}
export interface ArrayPattern extends BaseNode {
  +type: 'ArrayPattern';
  +elements: $ReadOnlyArray<DestructuringPattern | null>;
  +typeAnnotation?: TSTypeAnnotation;
  +optional?: boolean;
  +decorators?: $ReadOnlyArray<Decorator>;
}
export interface ArrowFunctionExpression extends BaseNode {
  +type: 'ArrowFunctionExpression';
  +generator: boolean;
  +id: null;
  +params: $ReadOnlyArray<Parameter>;
  +body: BlockStatement | Expression;
  +async: boolean;
  +expression: boolean;
  +returnType?: TSTypeAnnotation;
  +typeParameters?: TSTypeParameterDeclaration;
}
export interface AssignmentExpression extends BaseNode {
  +type: 'AssignmentExpression';
  +operator:
    | '='
    | '+='
    | '-='
    | '*='
    | '**='
    | '/='
    | '%='
    | '<<='
    | '>>='
    | '>>>='
    | '&='
    | '|='
    | '||='
    | '&&='
    | '??='
    | '^=';
  +left: Expression;
  +right: Expression;
}
export interface AssignmentPattern extends BaseNode {
  +type: 'AssignmentPattern';
  +left: BindingName;
  +right: Expression;
  +typeAnnotation?: TSTypeAnnotation;
  +optional?: boolean;
  +decorators?: $ReadOnlyArray<Decorator>;
}
export interface AwaitExpression extends BaseNode {
  +type: 'AwaitExpression';
  +argument: Expression;
}
export interface BigIntLiteral extends LiteralBase {
  +type: 'Literal';
  +value: /*bigint |*/ null;
  +bigint: string;
}
export interface BinaryExpression extends BaseNode {
  +type: 'BinaryExpression';
  +operator: string;
  +left: Expression | PrivateIdentifier;
  +right: Expression;
}
export type BindingName = BindingPattern | Identifier;
export type BindingPattern = ArrayPattern | ObjectPattern;
export interface BlockComment extends BaseToken {
  +type: 'Block';
}
export interface BlockStatement extends BaseNode {
  +type: 'BlockStatement';
  +body: $ReadOnlyArray<Statement>;
}
export interface BooleanLiteral extends LiteralBase {
  +type: 'Literal';
  +value: boolean;
  +raw: 'false' | 'true';
}
export interface BooleanToken extends BaseToken {
  +type: 'Boolean';
}
export interface BreakStatement extends BaseNode {
  +type: 'BreakStatement';
  +label: Identifier | null;
}
export interface CallExpression extends BaseNode {
  +type: 'CallExpression';
  +callee: LeftHandSideExpression;
  +arguments: $ReadOnlyArray<CallExpressionArgument>;
  +typeParameters?: TSTypeParameterInstantiation;
  +optional: boolean;
}
export type CallExpressionArgument = Expression | SpreadElement;
export interface CatchClause extends BaseNode {
  +type: 'CatchClause';
  +param: BindingName | null;
  +body: BlockStatement;
}
export type ChainElement =
  | CallExpression
  | MemberExpression
  | TSNonNullExpression;
export interface ChainExpression extends BaseNode {
  +type: 'ChainExpression';
  +expression: ChainElement;
}
interface ClassBase extends BaseNode {
  /**
   * Whether the class is an abstract class.
   * ```
   * abstract class Foo {...}
   * ```
   * This is always `undefined` for `ClassExpression`.
   */
  +abstract?: boolean;
  /**
   * The class body.
   */
  +body: ClassBody;
  /**
   * Whether the class has been `declare`d:
   * ```
   * declare class Foo {...}
   * ```
   * This is always `undefined` for `ClassExpression`.
   */
  +declare?: boolean;
  /**
   * The decorators declared for the class.
   * This is `undefined` if there are no decorators.
   * ```
   * @deco
   * class Foo {...}
   * ```
   * This is always `undefined` for `ClassExpression`.
   */
  +decorators?: $ReadOnlyArray<Decorator>;
  /**
   * The class's name.
   * - For a `ClassExpression` this may be `null` if the name is omitted.
   * - For a `ClassDeclaration` this may be `null` if and only if the parent is
   *   an `ExportDefaultDeclaration`.
   */
  +id: Identifier | null;
  /**
   * The implemented interfaces for the class.
   * This is `undefined` if there are no implemented interfaces.
   */
  +implements?: $ReadOnlyArray<TSClassImplements>;
  /**
   * The super class this class extends.
   */
  +superClass: LeftHandSideExpression | null;
  /**
   * The generic type parameters passed to the superClass.
   * This is `undefined` if there are no generic type parameters passed.
   */
  +superTypeParameters?: TSTypeParameterInstantiation;
  /**
   * The generic type parameters declared for the class.
   * This is `undefined` if there are no generic type parameters declared.
   */
  +typeParameters?: TSTypeParameterDeclaration;
}
export interface ClassBody extends BaseNode {
  +type: 'ClassBody';
  +body: $ReadOnlyArray<ClassElement>;
}
export type ClassDeclaration =
  | ClassDeclarationWithName
  | ClassDeclarationWithOptionalName;
interface ClassDeclarationBase extends ClassBase {
  +type: 'ClassDeclaration';
}
export interface ClassDeclarationWithName extends ClassDeclarationBase {
  +type: 'ClassDeclaration';
  +id: Identifier;
}
export interface ClassDeclarationWithOptionalName extends ClassDeclarationBase {
  +type: 'ClassDeclaration';
  +id: Identifier | null;
}
export type ClassElement =
  | MethodDefinition
  | PropertyDefinition
  | MethodDefinitionAmbiguous
  | PropertyDefinitionAmbiguous
  | StaticBlock
  | TSAbstractMethodDefinition
  | TSAbstractPropertyDefinition
  | TSIndexSignature;
export interface ClassExpression extends ClassBase {
  +type: 'ClassExpression';
  +abstract?: void;
  +declare?: void;
  +decorators?: void;
}
interface ClassMethodDefinitionNonComputedNameBase
  extends MethodDefinitionBase {
  +type: 'MethodDefinition';
  +key: ClassPropertyNameNonComputed;
  +computed: false;
}
interface ClassPropertyDefinitionNonComputedNameBase
  extends PropertyDefinitionBase {
  +type: 'PropertyDefinition';
  +key: ClassPropertyNameNonComputed;
  +computed: false;
}
export type ClassPropertyNameNonComputed =
  | PrivateIdentifier
  | PropertyNameNonComputed;
export type Comment = BlockComment | LineComment;
export interface ConditionalExpression extends BaseNode {
  +type: 'ConditionalExpression';
  +test: Expression;
  +consequent: Expression;
  +alternate: Expression;
}
export interface ContinueStatement extends BaseNode {
  +type: 'ContinueStatement';
  +label: Identifier | null;
}
export interface DebuggerStatement extends BaseNode {
  +type: 'DebuggerStatement';
}
export type DeclarationStatement =
  | ClassDeclaration
  | ClassExpression
  | ExportAllDeclaration
  | ExportDefaultDeclaration
  | ExportNamedDeclaration
  | FunctionDeclaration
  | TSDeclareFunction
  | TSEnumDeclaration
  | TSImportEqualsDeclaration
  | TSInterfaceDeclaration
  | TSModuleDeclaration
  | TSNamespaceExportDeclaration
  | TSTypeAliasDeclaration;
export interface Decorator extends BaseNode {
  +type: 'Decorator';
  +expression: LeftHandSideExpression;
}
export type DefaultExportDeclarations =
  | ClassDeclarationWithOptionalName
  | Expression
  | FunctionDeclarationWithName
  | FunctionDeclarationWithOptionalName
  | TSDeclareFunction
  | TSEnumDeclaration
  | TSInterfaceDeclaration
  | TSModuleDeclaration
  | TSTypeAliasDeclaration
  | VariableDeclaration;
export type DestructuringPattern =
  | ArrayPattern
  | AssignmentPattern
  | Identifier
  | MemberExpression
  | ObjectPattern
  | RestElement;
export interface DoWhileStatement extends BaseNode {
  +type: 'DoWhileStatement';
  +test: Expression;
  +body: Statement;
}
export interface EmptyStatement extends BaseNode {
  +type: 'EmptyStatement';
}
export type EntityName = Identifier | ThisExpression | TSQualifiedName;
export interface ExportAllDeclaration extends BaseNode {
  +type: 'ExportAllDeclaration';
  /**
   * The assertions declared for the export.
   * ```
   * export * from 'mod' assert { type: 'json' };
   * ```
   */
  +assertions: $ReadOnlyArray<ImportAttribute>;
  /**
   * The name for the exported items. `null` if no name is assigned.
   */
  +exported: Identifier | null;
  /**
   * The kind of the export.
   */
  +exportKind: ExportKind;
  /**
   * The source module being exported from.
   */
  +source: StringLiteral;
}
type ExportAndImportKind = 'type' | 'value';
export type ExportDeclaration =
  | DefaultExportDeclarations
  | NamedExportDeclarations;
export interface ExportDefaultDeclaration extends BaseNode {
  +type: 'ExportDefaultDeclaration';
  /**
   * The declaration being exported.
   */
  +declaration: DefaultExportDeclarations;
  /**
   * The kind of the export.
   */
  +exportKind: ExportKind;
}
type ExportKind = ExportAndImportKind;
export type ExportNamedDeclaration =
  | ExportNamedDeclarationWithoutSourceWithMultiple
  | ExportNamedDeclarationWithoutSourceWithSingle
  | ExportNamedDeclarationWithSource;
interface ExportNamedDeclarationBase extends BaseNode {
  +type: 'ExportNamedDeclaration';
  /**
   * The assertions declared for the export.
   * ```
   * export { foo } from 'mod' assert { type: 'json' };
   * ```
   * This will be an empty array if `source` is `null`
   */
  +assertions: $ReadOnlyArray<ImportAttribute>;
  /**
   * The exported declaration.
   * ```
   * export const x = 1;
   * ```
   * This will be `null` if `source` is not `null`, or if there are `specifiers`
   */
  +declaration: NamedExportDeclarations | null;
  /**
   * The kind of the export.
   */
  +exportKind: ExportKind;
  /**
   * The source module being exported from.
   */
  +source: StringLiteral | null;
  /**
   * The specifiers being exported.
   * ```
   * export { a, b };
   * ```
   * This will be an empty array if `declaration` is not `null`
   */
  +specifiers: $ReadOnlyArray<ExportSpecifier>;
}
export interface ExportNamedDeclarationAmbiguous
  extends ExportNamedDeclarationBase {
  +type: 'ExportNamedDeclaration';
}
export interface ExportNamedDeclarationWithoutSourceWithSingle
  extends ExportNamedDeclarationBase {
  +type: 'ExportNamedDeclaration';
  +assertions: $ReadOnlyArray<ImportAttribute>;
  +declaration: NamedExportDeclarations;
  +source: null;
  +specifiers: [];
}
export interface ExportNamedDeclarationWithoutSourceWithMultiple
  extends ExportNamedDeclarationBase {
  +type: 'ExportNamedDeclaration';
  +assertions: $ReadOnlyArray<ImportAttribute>;
  +declaration: null;
  +source: null;
  +specifiers: $ReadOnlyArray<ExportSpecifier>;
}
export interface ExportNamedDeclarationWithSource
  extends ExportNamedDeclarationBase {
  +type: 'ExportNamedDeclaration';
  +assertions: $ReadOnlyArray<ImportAttribute>;
  +declaration: null;
  +source: StringLiteral;
  +specifiers: $ReadOnlyArray<ExportSpecifier>;
}
export interface ExportSpecifier extends BaseNode {
  +type: 'ExportSpecifier';
  +local: Identifier;
  +exported: Identifier;
  +exportKind: ExportKind;
}
export type Expression =
  | ArrayExpression
  | ArrayPattern
  | ArrowFunctionExpression
  | AssignmentExpression
  | AwaitExpression
  | BinaryExpression
  | CallExpression
  | ChainExpression
  | ClassExpression
  | ConditionalExpression
  | FunctionExpression
  | Identifier
  | ImportExpression
  | JSXElement
  | JSXFragment
  | LiteralExpression
  | LogicalExpression
  | MemberExpression
  | MetaProperty
  | NewExpression
  | ObjectExpression
  | ObjectPattern
  | SequenceExpression
  | Super
  | TaggedTemplateExpression
  | TemplateLiteral
  | ThisExpression
  | TSAsExpression
  | TSInstantiationExpression
  | TSNonNullExpression
  | TSTypeAssertion
  | UnaryExpression
  | UpdateExpression
  | YieldExpression;
export interface ExpressionStatement extends BaseNode {
  +type: 'ExpressionStatement';
  +expression: Expression;
  +directive?: string;
}
export type ForInitialiser = Expression | VariableDeclaration;
export interface ForInStatement extends BaseNode {
  +type: 'ForInStatement';
  +left: ForInitialiser;
  +right: Expression;
  +body: Statement;
}
export interface ForOfStatement extends BaseNode {
  +type: 'ForOfStatement';
  +left: ForInitialiser;
  +right: Expression;
  +body: Statement;
  +await: boolean;
}
export interface ForStatement extends BaseNode {
  +type: 'ForStatement';
  +init: Expression | ForInitialiser | null;
  +test: Expression | null;
  +update: Expression | null;
  +body: Statement;
}
interface FunctionBase extends BaseNode {
  /**
   * Whether the function is async:
   * ```
   * async function foo(...) {...}
   * const x = async function (...) {...}
   * const x = async (...) => {...}
   * ```
   */
  +async: boolean;
  /**
   * The body of the function.
   * - For an `ArrowFunctionExpression` this may be an `Expression` or `BlockStatement`.
   * - For a `FunctionDeclaration` or `FunctionExpression` this is always a `BlockStatement.
   * - For a `TSDeclareFunction` this is always `undefined`.
   * - For a `TSEmptyBodyFunctionExpression` this is always `null`.
   */
  +body?: BlockStatement | Expression | null;
  /**
   * This is only `true` if and only if the node is a `TSDeclareFunction` and it has `declare`:
   * ```
   * declare function foo(...) {...}
   * ```
   */
  +declare?: boolean;
  /**
   * This is only ever `true` if and only the node is an `ArrowFunctionExpression` and the body
   * is an expression:
   * ```
   * (() => 1)
   * ```
   */
  +expression: boolean;
  /**
   * Whether the function is a generator function:
   * ```
   * function *foo(...) {...}
   * const x = function *(...) {...}
   * ```
   * This is always `false` for arrow functions as they cannot be generators.
   */
  +generator: boolean;
  /**
   * The function's name.
   * - For an `ArrowFunctionExpression` this is always `null`.
   * - For a `FunctionExpression` this may be `null` if the name is omitted.
   * - For a `FunctionDeclaration` or `TSDeclareFunction` this may be `null` if
   *   and only if the parent is an `ExportDefaultDeclaration`.
   */
  +id: Identifier | null;
  /**
   * The list of parameters declared for the function.
   */
  +params: $ReadOnlyArray<Parameter>;
  /**
   * The return type annotation for the function.
   * This is `undefined` if there is no return type declared.
   */
  +returnType?: TSTypeAnnotation;
  /**
   * The generic type parameter declaration for the function.
   * This is `undefined` if there are no generic type parameters declared.
   */
  +typeParameters?: TSTypeParameterDeclaration;
}
export type FunctionDeclaration =
  | FunctionDeclarationWithName
  | FunctionDeclarationWithOptionalName;
interface FunctionDeclarationBase extends FunctionBase {
  +type: 'FunctionDeclaration';
  +body: BlockStatement;
  +declare?: false;
  +expression: false;
}
export interface FunctionDeclarationWithName extends FunctionDeclarationBase {
  +type: 'FunctionDeclaration';
  +id: Identifier;
}
export interface FunctionDeclarationWithOptionalName
  extends FunctionDeclarationBase {
  +type: 'FunctionDeclaration';
  +id: Identifier | null;
}
export interface FunctionExpression extends FunctionBase {
  +type: 'FunctionExpression';
  +body: BlockStatement;
  +expression: false;
}
export type FunctionLike =
  | ArrowFunctionExpression
  | FunctionDeclaration
  | FunctionExpression
  | TSDeclareFunction
  | TSEmptyBodyFunctionExpression;
export interface Identifier extends BaseNode {
  +type: 'Identifier';
  +name: string;
  +typeAnnotation?: TSTypeAnnotation;
  +optional?: boolean;
  +decorators?: $ReadOnlyArray<Decorator>;
}
export interface IdentifierToken extends BaseToken {
  +type: 'Identifier';
}
export interface IfStatement extends BaseNode {
  +type: 'IfStatement';
  +test: Expression;
  +consequent: Statement;
  +alternate: Statement | null;
}
export interface ImportAttribute extends BaseNode {
  +type: 'ImportAttribute';
  +key: Identifier | Literal;
  +value: Literal;
}
export type ImportClause =
  | ImportDefaultSpecifier
  | ImportNamespaceSpecifier
  | ImportSpecifier;
export interface ImportDeclaration extends BaseNode {
  +type: 'ImportDeclaration';
  /**
   * The assertions declared for the export.
   * ```
   * import * from 'mod' assert { type: 'json' };
   * ```
   */
  +assertions: $ReadOnlyArray<ImportAttribute>;
  /**
   * The kind of the import.
   */
  +importKind: ImportKind;
  /**
   * The source module being imported from.
   */
  +source: StringLiteral;
  /**
   * The specifiers being imported.
   * If this is an empty array then either there are no specifiers:
   * ```
   * import {} from 'mod';
   * ```
   * Or it is a side-effect import:
   * ```
   * import 'mod';
   * ```
   */
  +specifiers: $ReadOnlyArray<ImportClause>;
}
export interface ImportDefaultSpecifier extends BaseNode {
  +type: 'ImportDefaultSpecifier';
  +local: Identifier;
}
export interface ImportExpression extends BaseNode {
  +type: 'ImportExpression';
  +source: Expression;
  +attributes: Expression | null;
}
type ImportKind = ExportAndImportKind;
export interface ImportNamespaceSpecifier extends BaseNode {
  +type: 'ImportNamespaceSpecifier';
  +local: Identifier;
}
export interface ImportSpecifier extends BaseNode {
  +type: 'ImportSpecifier';
  +local: Identifier;
  +imported: Identifier;
  +importKind: ?ImportKind;
}
export type IterationStatement =
  | DoWhileStatement
  | ForInStatement
  | ForOfStatement
  | ForStatement
  | WhileStatement;
export interface JSXAttribute extends BaseNode {
  +type: 'JSXAttribute';
  +name: JSXIdentifier | JSXNamespacedName;
  +value: JSXExpression | Literal | null;
}
export type JSXChild = JSXElement | JSXExpression | JSXFragment | JSXText;
export interface JSXClosingElement extends BaseNode {
  +type: 'JSXClosingElement';
  +name: JSXTagNameExpression;
}
export interface JSXClosingFragment extends BaseNode {
  +type: 'JSXClosingFragment';
}
export interface JSXElement extends BaseNode {
  +type: 'JSXElement';
  +openingElement: JSXOpeningElement;
  +closingElement: JSXClosingElement | null;
  +children: $ReadOnlyArray<JSXChild>;
}
export interface JSXEmptyExpression extends BaseNode {
  +type: 'JSXEmptyExpression';
}
export type JSXExpression =
  | JSXEmptyExpression
  | JSXExpressionContainer
  | JSXSpreadChild;
export interface JSXExpressionContainer extends BaseNode {
  +type: 'JSXExpressionContainer';
  +expression: Expression | JSXEmptyExpression;
}
export interface JSXFragment extends BaseNode {
  +type: 'JSXFragment';
  +openingFragment: JSXOpeningFragment;
  +closingFragment: JSXClosingFragment;
  +children: $ReadOnlyArray<JSXChild>;
}
export interface JSXIdentifier extends BaseNode {
  +type: 'JSXIdentifier';
  +name: string;
}
export interface JSXIdentifierToken extends BaseToken {
  +type: 'JSXIdentifier';
}
export interface JSXMemberExpression extends BaseNode {
  +type: 'JSXMemberExpression';
  +object: JSXTagNameExpression;
  +property: JSXIdentifier;
}
export interface JSXNamespacedName extends BaseNode {
  +type: 'JSXNamespacedName';
  +namespace: JSXIdentifier;
  +name: JSXIdentifier;
}
export interface JSXOpeningElement extends BaseNode {
  +type: 'JSXOpeningElement';
  +typeParameters?: TSTypeParameterInstantiation;
  +selfClosing: boolean;
  +name: JSXTagNameExpression;
  +attributes: $ReadOnlyArray<JSXAttribute | JSXSpreadAttribute>;
}
export interface JSXOpeningFragment extends BaseNode {
  +type: 'JSXOpeningFragment';
}
export interface JSXSpreadAttribute extends BaseNode {
  +type: 'JSXSpreadAttribute';
  +argument: Expression;
}
export interface JSXSpreadChild extends BaseNode {
  +type: 'JSXSpreadChild';
  +expression: Expression | JSXEmptyExpression;
}
export type JSXTagNameExpression =
  | JSXIdentifier
  | JSXMemberExpression
  | JSXNamespacedName;
export interface JSXText extends BaseNode {
  +type: 'JSXText';
  +value: string;
  +raw: string;
}
export interface JSXTextToken extends BaseToken {
  +type: 'JSXText';
}
export interface KeywordToken extends BaseToken {
  +type: 'Keyword';
}
export interface LabeledStatement extends BaseNode {
  +type: 'LabeledStatement';
  +label: Identifier;
  +body: Statement;
}
export type LeftHandSideExpression =
  | ArrayExpression
  | ArrayPattern
  | ArrowFunctionExpression
  | CallExpression
  | ClassExpression
  | FunctionExpression
  | Identifier
  | JSXElement
  | JSXFragment
  | LiteralExpression
  | MemberExpression
  | MetaProperty
  | ObjectExpression
  | ObjectPattern
  | SequenceExpression
  | Super
  | TaggedTemplateExpression
  | ThisExpression
  | TSAsExpression
  | TSNonNullExpression
  | TSTypeAssertion;
export interface LineComment extends BaseToken {
  +type: 'Line';
}
export type Literal =
  | BigIntLiteral
  | BooleanLiteral
  | NullLiteral
  | NumberLiteral
  | RegExpLiteral
  | StringLiteral;
interface LiteralBase extends BaseNode {
  +type: 'Literal';
  +raw: string;
  +value: RegExp | /*bigint |*/ boolean | number | string | null;
}
export type LiteralExpression = Literal | TemplateLiteral;
export interface LogicalExpression extends BaseNode {
  +type: 'LogicalExpression';
  +operator: '??' | '&&' | '||';
  +left: Expression;
  +right: Expression;
}
export type MemberExpression =
  | MemberExpressionComputedName
  | MemberExpressionNonComputedName;
interface MemberExpressionBase extends BaseNode {
  +object: LeftHandSideExpression;
  +property: Expression | Identifier | PrivateIdentifier;
  +computed: boolean;
  +optional: boolean;
}
export interface MemberExpressionComputedName extends MemberExpressionBase {
  +type: 'MemberExpression';
  +property: Expression;
  +computed: true;
}
export interface MemberExpressionNonComputedName extends MemberExpressionBase {
  +type: 'MemberExpression';
  +property: Identifier | PrivateIdentifier;
  +computed: false;
}
export interface MetaProperty extends BaseNode {
  +type: 'MetaProperty';
  +meta: Identifier;
  +property: Identifier;
}
export type MethodDefinition =
  | MethodDefinitionComputedName
  | MethodDefinitionNonComputedName;
/** this should not be directly used - instead use MethodDefinitionComputedNameBase or MethodDefinitionNonComputedNameBase */
interface MethodDefinitionBase extends BaseNode {
  +accessibility?: Accessibility;
  +computed: boolean;
  +decorators?: $ReadOnlyArray<Decorator>;
  +key: PropertyName;
  +kind: 'constructor' | 'get' | 'method' | 'set';
  +optional?: boolean;
  +override?: boolean;
  +static: boolean;
  +typeParameters?: TSTypeParameterDeclaration;
  +value: FunctionExpression | TSEmptyBodyFunctionExpression;
}
export interface MethodDefinitionAmbiguous extends MethodDefinitionBase {
  type: 'MethodDefinition';
}
export interface MethodDefinitionComputedName
  extends MethodDefinitionComputedNameBase {
  +type: 'MethodDefinition';
}
interface MethodDefinitionComputedNameBase extends MethodDefinitionBase {
  +key: PropertyNameComputed;
  +computed: true;
}
export interface MethodDefinitionNonComputedName
  extends ClassMethodDefinitionNonComputedNameBase {
  +type: 'MethodDefinition';
}
interface MethodDefinitionNonComputedNameBase extends MethodDefinitionBase {
  +key: PropertyNameNonComputed;
  +computed: false;
}
export type Modifier =
  | TSAbstractKeyword
  | TSAsyncKeyword
  | TSPrivateKeyword
  | TSProtectedKeyword
  | TSPublicKeyword
  | TSReadonlyKeyword
  | TSStaticKeyword;
export type NamedExportDeclarations =
  | ClassDeclarationWithName
  | ClassDeclarationWithOptionalName
  | FunctionDeclarationWithName
  | FunctionDeclarationWithOptionalName
  | TSDeclareFunction
  | TSEnumDeclaration
  | TSInterfaceDeclaration
  | TSModuleDeclaration
  | TSTypeAliasDeclaration
  | VariableDeclaration;
export interface NewExpression extends BaseNode {
  +type: 'NewExpression';
  +callee: LeftHandSideExpression;
  +arguments: $ReadOnlyArray<CallExpressionArgument>;
  +typeParameters?: TSTypeParameterInstantiation;
}
export type Node =
  | ArrayExpression
  | ArrayPattern
  | ArrowFunctionExpression
  | AssignmentExpression
  | AssignmentPattern
  | AwaitExpression
  | BinaryExpression
  | BlockStatement
  | BreakStatement
  | CallExpression
  | CatchClause
  | ChainExpression
  | ClassBody
  | ClassDeclaration
  | ClassExpression
  | ConditionalExpression
  | ContinueStatement
  | DebuggerStatement
  | Decorator
  | DoWhileStatement
  | EmptyStatement
  | ExportAllDeclaration
  | ExportDefaultDeclaration
  | ExportNamedDeclaration
  | ExportSpecifier
  | ExpressionStatement
  | ForInStatement
  | ForOfStatement
  | ForStatement
  | FunctionDeclaration
  | FunctionExpression
  | Identifier
  | IfStatement
  | ImportAttribute
  | ImportDeclaration
  | ImportDefaultSpecifier
  | ImportExpression
  | ImportNamespaceSpecifier
  | ImportSpecifier
  | JSXAttribute
  | JSXClosingElement
  | JSXClosingFragment
  | JSXElement
  | JSXEmptyExpression
  | JSXExpressionContainer
  | JSXFragment
  | JSXIdentifier
  | JSXMemberExpression
  | JSXNamespacedName
  | JSXOpeningElement
  | JSXOpeningFragment
  | JSXSpreadAttribute
  | JSXSpreadChild
  | JSXText
  | LabeledStatement
  | Literal
  | LogicalExpression
  | MemberExpression
  | MetaProperty
  | MethodDefinition
  | NewExpression
  | ObjectExpression
  | ObjectPattern
  | PrivateIdentifier
  | Program
  | Property
  | PropertyDefinition
  | RestElement
  | ReturnStatement
  | SequenceExpression
  | SpreadElement
  | StaticBlock
  | Super
  | SwitchCase
  | SwitchStatement
  | TaggedTemplateExpression
  | TemplateElement
  | TemplateLiteral
  | ThisExpression
  | ThrowStatement
  | TryStatement
  | TSAbstractKeyword
  | TSAbstractMethodDefinition
  | TSAbstractPropertyDefinition
  | TSAnyKeyword
  | TSArrayType
  | TSAsExpression
  | TSAsyncKeyword
  | TSBigIntKeyword
  | TSBooleanKeyword
  | TSCallSignatureDeclaration
  | TSClassImplements
  | TSConditionalType
  | TSConstructorType
  | TSConstructSignatureDeclaration
  | TSDeclareFunction
  | TSDeclareKeyword
  | TSEmptyBodyFunctionExpression
  | TSEnumDeclaration
  | TSEnumMember
  | TSExportAssignment
  | TSExportKeyword
  | TSExternalModuleReference
  | TSFunctionType
  | TSImportEqualsDeclaration
  | TSImportType
  | TSIndexedAccessType
  | TSIndexSignature
  | TSInferType
  | TSInstantiationExpression
  | TSInterfaceBody
  | TSInterfaceDeclaration
  | TSInterfaceHeritage
  | TSIntersectionType
  | TSIntrinsicKeyword
  | TSLiteralType
  | TSMappedType
  | TSMethodSignature
  | TSModuleBlock
  | TSModuleDeclaration
  | TSNamedTupleMember
  | TSNamespaceExportDeclaration
  | TSNeverKeyword
  | TSNonNullExpression
  | TSNullKeyword
  | TSNumberKeyword
  | TSObjectKeyword
  | TSOptionalType
  | TSParameterProperty
  | TSPrivateKeyword
  | TSPropertySignature
  | TSProtectedKeyword
  | TSPublicKeyword
  | TSQualifiedName
  | TSReadonlyKeyword
  | TSRestType
  | TSStaticKeyword
  | TSStringKeyword
  | TSSymbolKeyword
  | TSTemplateLiteralType
  | TSThisType
  | TSTupleType
  | TSTypeAliasDeclaration
  | TSTypeAnnotation
  | TSTypeAssertion
  | TSTypeLiteral
  | TSTypeOperator
  | TSTypeParameter
  | TSTypeParameterDeclaration
  | TSTypeParameterInstantiation
  | TSTypePredicate
  | TSTypeQuery
  | TSTypeReference
  | TSUndefinedKeyword
  | TSUnionType
  | TSUnknownKeyword
  | TSVoidKeyword
  | UnaryExpression
  | UpdateExpression
  | VariableDeclaration
  | VariableDeclarator
  | WhileStatement
  | WithStatement
  | YieldExpression
  // new "ambiguous" nodes
  | ExportNamedDeclarationAmbiguous;
export interface NullLiteral extends LiteralBase {
  +type: 'Literal';
  +value: null;
  +raw: 'null';
}
export interface NullToken extends BaseToken {
  +type: 'Null';
}
export interface NumberLiteral extends LiteralBase {
  +type: 'Literal';
  +value: number;
}
export interface NumericToken extends BaseToken {
  +type: 'Numeric';
}
export interface ObjectExpression extends BaseNode {
  +type: 'ObjectExpression';
  +properties: $ReadOnlyArray<ObjectLiteralElement>;
}
export type ObjectLiteralElement = MethodDefinition | Property | SpreadElement;
export type ObjectLiteralElementLike = ObjectLiteralElement;
export interface ObjectPattern extends BaseNode {
  +type: 'ObjectPattern';
  +properties: $ReadOnlyArray<Property | RestElement>;
  +typeAnnotation?: TSTypeAnnotation;
  +optional?: boolean;
  +decorators?: $ReadOnlyArray<Decorator>;
}
export type Parameter =
  | ArrayPattern
  | AssignmentPattern
  | Identifier
  | ObjectPattern
  | RestElement
  | TSParameterProperty;
export interface Position {
  /**
   * Line number (1-indexed)
   */
  +line: number;
  /**
   * Column number on the line (0-indexed)
   */
  +column: number;
}
export type PrimaryExpression =
  | ArrayExpression
  | ArrayPattern
  | ClassExpression
  | FunctionExpression
  | Identifier
  | JSXElement
  | JSXFragment
  | JSXOpeningElement
  | LiteralExpression
  | MetaProperty
  | ObjectExpression
  | ObjectPattern
  | Super
  | TemplateLiteral
  | ThisExpression
  | TSNullKeyword;
export interface PrivateIdentifier extends BaseNode {
  +type: 'PrivateIdentifier';
  +name: string;
}
export interface Program extends BaseNode {
  +type: 'Program';
  +body: $ReadOnlyArray<ProgramStatement>;
  +sourceType: 'module' | 'script';
  +comments?: $ReadOnlyArray<Comment>;
  +tokens?: $ReadOnlyArray<Token>;
}
export type ProgramStatement =
  | ExportAllDeclaration
  | ExportDefaultDeclaration
  | ExportNamedDeclaration
  | ImportDeclaration
  | Statement
  | TSImportEqualsDeclaration
  | TSNamespaceExportDeclaration;
export type Property = PropertyComputedName | PropertyNonComputedName;
interface PropertyBase extends BaseNode {
  +type: 'Property';
  +key: PropertyName;
  +value:
    | AssignmentPattern
    | BindingName
    | Expression
    | TSEmptyBodyFunctionExpression;
  +computed: boolean;
  +method: boolean;
  +shorthand: boolean;
  +optional?: boolean;
  +kind: 'get' | 'init' | 'set';
}
export interface PropertyComputedName extends PropertyBase {
  +type: 'Property';
  +key: PropertyNameComputed;
  +computed: true;
}
export type PropertyDefinition =
  | PropertyDefinitionComputedName
  | PropertyDefinitionNonComputedName;
interface PropertyDefinitionBase extends BaseNode {
  +accessibility?: Accessibility;
  +computed: boolean;
  +declare: boolean;
  +decorators?: $ReadOnlyArray<Decorator>;
  +definite?: boolean;
  +key: PropertyName;
  +optional?: boolean;
  +override?: boolean;
  +readonly?: boolean;
  +static: boolean;
  +typeAnnotation?: TSTypeAnnotation;
  +value: Expression | null;
}
export interface PropertyDefinitionAmbiguous extends PropertyDefinitionBase {
  type: 'PropertyDefinition';
}
export interface PropertyDefinitionComputedName
  extends PropertyDefinitionComputedNameBase {
  +type: 'PropertyDefinition';
}
interface PropertyDefinitionComputedNameBase extends PropertyDefinitionBase {
  +key: PropertyNameComputed;
  +computed: true;
}
export interface PropertyDefinitionNonComputedName
  extends ClassPropertyDefinitionNonComputedNameBase {
  +type: 'PropertyDefinition';
}
interface PropertyDefinitionNonComputedNameBase extends PropertyDefinitionBase {
  +key: PropertyNameNonComputed;
  +computed: false;
}
export type PropertyName =
  | ClassPropertyNameNonComputed
  | PropertyNameComputed
  | PropertyNameNonComputed;
export type PropertyNameComputed = Expression;
export type PropertyNameNonComputed =
  | Identifier
  | NumberLiteral
  | StringLiteral;
export interface PropertyNonComputedName extends PropertyBase {
  +type: 'Property';
  +key: PropertyNameNonComputed;
  +computed: false;
}
export interface PunctuatorToken extends BaseToken {
  +type: 'Punctuator';
  +value:
    | '{'
    | '}'
    | '('
    | ')'
    | '['
    | ']'
    | '.'
    | '...'
    | ';'
    | ','
    | '?.'
    | '<'
    | '</'
    | '>'
    | '<='
    | '>='
    | '=='
    | '!='
    | '==='
    | '!=='
    | '=>'
    | '+'
    | '-'
    | '*'
    | '**'
    | '/'
    | '%'
    | '++'
    | '--'
    | '<<'
    | '>>'
    | '>>>'
    | '&'
    | '|'
    | '^'
    | '!'
    | '~'
    | '&&'
    | '||'
    | '?'
    | ':'
    | '@'
    | '??'
    | '`'
    | '#';
}
/**
 * An array of two numbers.
 * Both numbers are a 0-based index which is the position in the array of source code characters.
 * The first is the start position of the node, the second is the end position of the node.
 */
export type Range = [number, number];
export interface RegExpLiteral extends LiteralBase {
  +type: 'Literal';
  +value: RegExp | null;
  +regex: {
    +pattern: string,
    +flags: string,
  };
}
export interface RegularExpressionToken extends BaseToken {
  +type: 'RegularExpression';
  +regex: {
    +pattern: string,
    +flags: string,
  };
}
export interface RestElement extends BaseNode {
  +type: 'RestElement';
  +argument: DestructuringPattern;
  +typeAnnotation?: TSTypeAnnotation;
  +optional?: boolean;
  +value?: AssignmentPattern;
  +decorators?: $ReadOnlyArray<Decorator>;
}
export interface ReturnStatement extends BaseNode {
  +type: 'ReturnStatement';
  +argument: Expression | null;
}
export interface SequenceExpression extends BaseNode {
  +type: 'SequenceExpression';
  +expressions: $ReadOnlyArray<Expression>;
}
export interface SourceLocation {
  /**
   * The position of the first character of the parsed source region
   */
  +start: Position;
  /**
   * The position of the first character after the parsed source region
   */
  +end: Position;
}
export interface SpreadElement extends BaseNode {
  +type: 'SpreadElement';
  +argument: Expression;
}
export type Statement =
  | BlockStatement
  | BreakStatement
  | ClassDeclarationWithName
  | ContinueStatement
  | DebuggerStatement
  | DoWhileStatement
  | ExportAllDeclaration
  | ExportDefaultDeclaration
  | ExportNamedDeclaration
  | ExpressionStatement
  | ForInStatement
  | ForOfStatement
  | ForStatement
  | FunctionDeclarationWithName
  | IfStatement
  | ImportDeclaration
  | LabeledStatement
  | ReturnStatement
  | SwitchStatement
  | ThrowStatement
  | TryStatement
  | TSDeclareFunction
  | TSEnumDeclaration
  | TSExportAssignment
  | TSImportEqualsDeclaration
  | TSInterfaceDeclaration
  | TSModuleDeclaration
  | TSNamespaceExportDeclaration
  | TSTypeAliasDeclaration
  | VariableDeclaration
  | WhileStatement
  | WithStatement;
export interface StaticBlock extends BaseNode {
  +type: 'StaticBlock';
  +body: $ReadOnlyArray<Statement>;
}
export interface StringLiteral extends LiteralBase {
  +type: 'Literal';
  +value: string;
}
export interface StringToken extends BaseToken {
  +type: 'String';
}
export interface Super extends BaseNode {
  +type: 'Super';
}
export interface SwitchCase extends BaseNode {
  +type: 'SwitchCase';
  +test: Expression | null;
  +consequent: $ReadOnlyArray<Statement>;
}
export interface SwitchStatement extends BaseNode {
  +type: 'SwitchStatement';
  +discriminant: Expression;
  +cases: $ReadOnlyArray<SwitchCase>;
}
export interface TaggedTemplateExpression extends BaseNode {
  +type: 'TaggedTemplateExpression';
  +typeParameters?: TSTypeParameterInstantiation;
  +tag: LeftHandSideExpression;
  +quasi: TemplateLiteral;
}
export interface TemplateElement extends BaseNode {
  +type: 'TemplateElement';
  +value: {
    raw: string,
    cooked: string,
  };
  +tail: boolean;
}
export interface TemplateLiteral extends BaseNode {
  +type: 'TemplateLiteral';
  +quasis: $ReadOnlyArray<TemplateElement>;
  +expressions: $ReadOnlyArray<Expression>;
}
export interface TemplateToken extends BaseToken {
  +type: 'Template';
}
export interface ThisExpression extends BaseNode {
  +type: 'ThisExpression';
}
export interface ThrowStatement extends BaseNode {
  +type: 'ThrowStatement';
  +argument: Statement | TSAsExpression | null;
}
export type Token =
  | BooleanToken
  | Comment
  | IdentifierToken
  | JSXIdentifierToken
  | JSXTextToken
  | KeywordToken
  | NullToken
  | NumericToken
  | PunctuatorToken
  | RegularExpressionToken
  | StringToken
  | TemplateToken;
export interface TryStatement extends BaseNode {
  +type: 'TryStatement';
  +block: BlockStatement;
  +handler: CatchClause | null;
  +finalizer: BlockStatement | null;
}
export interface TSAbstractKeyword extends BaseNode {
  +type: 'TSAbstractKeyword';
}
export type TSAbstractMethodDefinition =
  | TSAbstractMethodDefinitionComputedName
  | TSAbstractMethodDefinitionNonComputedName;
export interface TSAbstractMethodDefinitionComputedName
  extends MethodDefinitionComputedNameBase {
  +type: 'TSAbstractMethodDefinition';
}
export interface TSAbstractMethodDefinitionNonComputedName
  extends MethodDefinitionNonComputedNameBase {
  +type: 'TSAbstractMethodDefinition';
}
export type TSAbstractPropertyDefinition =
  | TSAbstractPropertyDefinitionComputedName
  | TSAbstractPropertyDefinitionNonComputedName;
export interface TSAbstractPropertyDefinitionComputedName
  extends PropertyDefinitionComputedNameBase {
  +type: 'TSAbstractPropertyDefinition';
  +value: null;
}
export interface TSAbstractPropertyDefinitionNonComputedName
  extends PropertyDefinitionNonComputedNameBase {
  +type: 'TSAbstractPropertyDefinition';
  +value: null;
}
export interface TSAnyKeyword extends BaseNode {
  +type: 'TSAnyKeyword';
}
export interface TSArrayType extends BaseNode {
  +type: 'TSArrayType';
  +elementType: TypeNode;
}
export interface TSAsExpression extends BaseNode {
  +type: 'TSAsExpression';
  +expression: Expression;
  +typeAnnotation: TypeNode;
}
export interface TSAsyncKeyword extends BaseNode {
  +type: 'TSAsyncKeyword';
}
export interface TSBigIntKeyword extends BaseNode {
  +type: 'TSBigIntKeyword';
}
export interface TSBooleanKeyword extends BaseNode {
  +type: 'TSBooleanKeyword';
}
export interface TSCallSignatureDeclaration extends TSFunctionSignatureBase {
  +type: 'TSCallSignatureDeclaration';
}
export interface TSClassImplements extends TSHeritageBase {
  +type: 'TSClassImplements';
}
export interface TSConditionalType extends BaseNode {
  +type: 'TSConditionalType';
  +checkType: TypeNode;
  +extendsType: TypeNode;
  +trueType: TypeNode;
  +falseType: TypeNode;
}
export interface TSConstructorType extends TSFunctionSignatureBase {
  +type: 'TSConstructorType';
  +abstract: boolean;
}
export interface TSConstructSignatureDeclaration
  extends TSFunctionSignatureBase {
  +type: 'TSConstructSignatureDeclaration';
}
export interface TSDeclareFunction extends FunctionBase {
  +type: 'TSDeclareFunction';
  +body?: BlockStatement;
  +declare?: boolean;
  +expression: false;
}
export interface TSDeclareKeyword extends BaseNode {
  +type: 'TSDeclareKeyword';
}
export interface TSEmptyBodyFunctionExpression extends FunctionBase {
  +type: 'TSEmptyBodyFunctionExpression';
  +body: null;
  +id: null;
}
export interface TSEnumDeclaration extends BaseNode {
  +type: 'TSEnumDeclaration';
  /**
   * Whether this is a `const` enum.
   * ```
   * const enum Foo {...}
   * ```
   */
  +const?: boolean;
  /**
   * Whether this is a `declare`d enum.
   * ```
   * declare enum Foo {...}
   * ```
   */
  +declare?: boolean;
  /**
   * The enum name.
   */
  +id: Identifier;
  /**
   * The enum members.
   */
  +members: $ReadOnlyArray<TSEnumMember>;
  +modifiers?: $ReadOnlyArray<Modifier>;
}
export type TSEnumMember =
  | TSEnumMemberComputedName
  | TSEnumMemberNonComputedName;
interface TSEnumMemberBase extends BaseNode {
  +type: 'TSEnumMember';
  +id: PropertyNameComputed | PropertyNameNonComputed;
  +initializer?: Expression;
  +computed?: boolean;
}
/**
 * this should only really happen in semantically invalid code (errors 1164 and 2452)
 *
 * VALID:
 * enum Foo { ['a'] }
 *
 * INVALID:
 * const x = 'a';
 * enum Foo { [x] }
 * enum Bar { ['a' + 'b'] }
 */
export interface TSEnumMemberComputedName extends TSEnumMemberBase {
  +type: 'TSEnumMember';
  +id: PropertyNameComputed;
  +computed: true;
}
export interface TSEnumMemberNonComputedName extends TSEnumMemberBase {
  +type: 'TSEnumMember';
  +id: PropertyNameNonComputed;
  +computed?: false;
}
export interface TSExportAssignment extends BaseNode {
  +type: 'TSExportAssignment';
  +expression: Expression;
}
export interface TSExportKeyword extends BaseNode {
  +type: 'TSExportKeyword';
}
export interface TSExternalModuleReference extends BaseNode {
  +type: 'TSExternalModuleReference';
  +expression: Expression;
}
interface TSFunctionSignatureBase extends BaseNode {
  +params: $ReadOnlyArray<Parameter>;
  +returnType?: TSTypeAnnotation;
  +typeParameters?: TSTypeParameterDeclaration;
}
export interface TSFunctionType extends TSFunctionSignatureBase {
  +type: 'TSFunctionType';
}
interface TSHeritageBase extends BaseNode {
  +expression: Expression;
  +typeParameters?: TSTypeParameterInstantiation;
}
export interface TSImportEqualsDeclaration extends BaseNode {
  +type: 'TSImportEqualsDeclaration';
  /**
   * The locally imported name
   */
  +id: Identifier;
  /**
   * The value being aliased.
   * ```
   * import F1 = A;
   * import F2 = A.B.C;
   * import F3 = require('mod');
   * ```
   */
  +moduleReference: EntityName | TSExternalModuleReference;
  +importKind: ImportKind;
  /**
   * Whether this is immediately exported
   * ```
   * export import F = A;
   * ```
   */
  +isExport: boolean;
}
export interface TSImportType extends BaseNode {
  +type: 'TSImportType';
  +isTypeOf: boolean;
  +parameter: TypeNode;
  +qualifier: EntityName | null;
  +typeParameters: TSTypeParameterInstantiation | null;
}
export interface TSIndexedAccessType extends BaseNode {
  +type: 'TSIndexedAccessType';
  +objectType: TypeNode;
  +indexType: TypeNode;
}
export interface TSIndexSignature extends BaseNode {
  +type: 'TSIndexSignature';
  +accessibility?: Accessibility;
  +export?: boolean;
  +parameters: $ReadOnlyArray<Parameter>;
  +readonly?: boolean;
  +static?: boolean;
  +typeAnnotation?: TSTypeAnnotation;
}
export interface TSInferType extends BaseNode {
  +type: 'TSInferType';
  +typeParameter: TSTypeParameter;
}
export interface TSInstantiationExpression extends BaseNode {
  +type: 'TSInstantiationExpression';
  +expression: Expression;
  +typeParameters: TSTypeParameterInstantiation;
}
export interface TSInterfaceBody extends BaseNode {
  +type: 'TSInterfaceBody';
  +body: $ReadOnlyArray<TypeElement>;
}
export interface TSInterfaceDeclaration extends BaseNode {
  +type: 'TSInterfaceDeclaration';
  +abstract?: boolean;
  /**
   * The body of the interface
   */
  +body: TSInterfaceBody;
  /**
   * Whether the interface was `declare`d, `undefined` otherwise
   */
  +declare?: boolean;
  /**
   * The types this interface `extends`
   */
  +extends?: $ReadOnlyArray<TSInterfaceHeritage>;
  /**
   * The name of this interface
   */
  +id: Identifier;
  +implements?: $ReadOnlyArray<TSInterfaceHeritage>;
  /**
   * The generic type parameters declared for the interface.
   * This is `undefined` if there are no generic type parameters declared.
   */
  +typeParameters?: TSTypeParameterDeclaration;
}
export interface TSInterfaceHeritage extends TSHeritageBase {
  +type: 'TSInterfaceHeritage';
}
export interface TSIntersectionType extends BaseNode {
  +type: 'TSIntersectionType';
  +types: $ReadOnlyArray<TypeNode>;
}
export interface TSIntrinsicKeyword extends BaseNode {
  +type: 'TSIntrinsicKeyword';
}
export interface TSLiteralType extends BaseNode {
  +type: 'TSLiteralType';
  +literal: LiteralExpression | UnaryExpression | UpdateExpression;
}
export interface TSMappedType extends BaseNode {
  +type: 'TSMappedType';
  +typeParameter: TSTypeParameter;
  +readonly?: boolean | '-' | '+';
  +optional?: boolean | '-' | '+';
  +typeAnnotation?: TypeNode;
  +nameType: TypeNode | null;
}
export type TSMethodSignature =
  | TSMethodSignatureComputedName
  | TSMethodSignatureNonComputedName;
interface TSMethodSignatureBase extends BaseNode {
  +type: 'TSMethodSignature';
  +accessibility?: Accessibility;
  +computed: boolean;
  +export?: boolean;
  +key: PropertyName;
  +kind: 'get' | 'method' | 'set';
  +optional?: boolean;
  +params: $ReadOnlyArray<Parameter>;
  +readonly?: boolean;
  +returnType?: TSTypeAnnotation;
  +static?: boolean;
  +typeParameters?: TSTypeParameterDeclaration;
}
export interface TSMethodSignatureComputedName extends TSMethodSignatureBase {
  +type: 'TSMethodSignature';
  +key: PropertyNameComputed;
  +computed: true;
}
export interface TSMethodSignatureNonComputedName
  extends TSMethodSignatureBase {
  +type: 'TSMethodSignature';
  +key: PropertyNameNonComputed;
  +computed: false;
}
export interface TSModuleBlock extends BaseNode {
  +type: 'TSModuleBlock';
  +body: $ReadOnlyArray<ProgramStatement>;
}
export interface TSModuleDeclaration extends BaseNode {
  +type: 'TSModuleDeclaration';
  /**
   * The name of the module
   * ```
   * namespace A {}
   * namespace A.B.C {}
   * module 'a' {}
   * ```
   */
  +id: Identifier | Literal;
  /**
   * The body of the module.
   * This can only be `undefined` for the code `declare module 'mod';`
   * This will be a `TSModuleDeclaration` if the name is "nested" (`Foo.Bar`).
   */
  +body?: TSModuleBlock | TSModuleDeclaration;
  /**
   * Whether this is a global declaration
   * ```
   * declare global {}
   * ```
   */
  +global?: boolean;
  /**
   * Whether the module is `declare`d
   * ```
   * declare namespace F {}
   * ```
   */
  +declare?: boolean;
  +modifiers?: $ReadOnlyArray<Modifier>;
}
export interface TSNamedTupleMember extends BaseNode {
  +type: 'TSNamedTupleMember';
  +elementType: TypeNode;
  +label: Identifier;
  +optional: boolean;
}
export interface TSNamespaceExportDeclaration extends BaseNode {
  +type: 'TSNamespaceExportDeclaration';
  /**
   * The name the global variable being exported to
   */
  +id: Identifier;
}
export interface TSNeverKeyword extends BaseNode {
  +type: 'TSNeverKeyword';
}
export interface TSNonNullExpression extends BaseNode {
  +type: 'TSNonNullExpression';
  +expression: Expression;
}
export interface TSNullKeyword extends BaseNode {
  +type: 'TSNullKeyword';
}
export interface TSNumberKeyword extends BaseNode {
  +type: 'TSNumberKeyword';
}
export interface TSObjectKeyword extends BaseNode {
  +type: 'TSObjectKeyword';
}
export interface TSOptionalType extends BaseNode {
  +type: 'TSOptionalType';
  +typeAnnotation: TypeNode;
}
export interface TSParameterProperty extends BaseNode {
  +type: 'TSParameterProperty';
  +accessibility?: Accessibility;
  +readonly?: boolean;
  +static?: boolean;
  +export?: boolean;
  +override?: boolean;
  +parameter: AssignmentPattern | BindingName | RestElement;
  +decorators?: $ReadOnlyArray<Decorator>;
}
export interface TSPrivateKeyword extends BaseNode {
  +type: 'TSPrivateKeyword';
}
export type TSPropertySignature =
  | TSPropertySignatureComputedName
  | TSPropertySignatureNonComputedName;
interface TSPropertySignatureBase extends BaseNode {
  +type: 'TSPropertySignature';
  +accessibility?: Accessibility;
  +computed: boolean;
  +export?: boolean;
  +initializer?: Expression;
  +key: PropertyName;
  +optional?: boolean;
  +readonly?: boolean;
  +static?: boolean;
  +typeAnnotation?: TSTypeAnnotation;
}
export interface TSPropertySignatureComputedName
  extends TSPropertySignatureBase {
  +type: 'TSPropertySignature';
  +key: PropertyNameComputed;
  +computed: true;
}
export interface TSPropertySignatureNonComputedName
  extends TSPropertySignatureBase {
  +type: 'TSPropertySignature';
  +key: PropertyNameNonComputed;
  +computed: false;
}
export interface TSProtectedKeyword extends BaseNode {
  +type: 'TSProtectedKeyword';
}
export interface TSPublicKeyword extends BaseNode {
  +type: 'TSPublicKeyword';
}
export interface TSQualifiedName extends BaseNode {
  +type: 'TSQualifiedName';
  +left: EntityName;
  +right: Identifier;
}
export interface TSReadonlyKeyword extends BaseNode {
  +type: 'TSReadonlyKeyword';
}
export interface TSRestType extends BaseNode {
  +type: 'TSRestType';
  +typeAnnotation: TypeNode;
}
export interface TSStaticKeyword extends BaseNode {
  +type: 'TSStaticKeyword';
}
export interface TSStringKeyword extends BaseNode {
  +type: 'TSStringKeyword';
}
export interface TSSymbolKeyword extends BaseNode {
  +type: 'TSSymbolKeyword';
}
export interface TSTemplateLiteralType extends BaseNode {
  +type: 'TSTemplateLiteralType';
  +quasis: $ReadOnlyArray<TemplateElement>;
  +types: $ReadOnlyArray<TypeNode>;
}
export interface TSThisType extends BaseNode {
  +type: 'TSThisType';
}
export interface TSTupleType extends BaseNode {
  +type: 'TSTupleType';
  +elementTypes: $ReadOnlyArray<TypeNode>;
}
export interface TSTypeAliasDeclaration extends BaseNode {
  +type: 'TSTypeAliasDeclaration';
  /**
   * Whether the type was `declare`d.
   * ```
   * declare type T = 1;
   * ```
   */
  +declare?: boolean;
  /**
   * The name of the type.
   */
  +id: Identifier;
  /**
   * The "value" (type) of the declaration
   */
  +typeAnnotation: TypeNode;
  /**
   * The generic type parameters declared for the type.
   * This is `undefined` if there are no generic type parameters declared.
   */
  +typeParameters?: TSTypeParameterDeclaration;
}
export interface TSTypeAnnotation extends BaseNode {
  +type: 'TSTypeAnnotation';
  +typeAnnotation: TypeNode;
}
export interface TSTypeAssertion extends BaseNode {
  +type: 'TSTypeAssertion';
  +typeAnnotation: TypeNode;
  +expression: Expression;
}
export interface TSTypeLiteral extends BaseNode {
  +type: 'TSTypeLiteral';
  +members: $ReadOnlyArray<TypeElement>;
}
export interface TSTypeOperator extends BaseNode {
  +type: 'TSTypeOperator';
  +operator: 'keyof' | 'readonly' | 'unique';
  +typeAnnotation?: TypeNode;
}
export interface TSTypeParameter extends BaseNode {
  +type: 'TSTypeParameter';
  +name: Identifier;
  +constraint?: TypeNode;
  +default?: TypeNode;
  +in: boolean;
  +out: boolean;
}
export interface TSTypeParameterDeclaration extends BaseNode {
  +type: 'TSTypeParameterDeclaration';
  +params: $ReadOnlyArray<TSTypeParameter>;
}
export interface TSTypeParameterInstantiation extends BaseNode {
  +type: 'TSTypeParameterInstantiation';
  +params: $ReadOnlyArray<TypeNode>;
}
export interface TSTypePredicate extends BaseNode {
  +type: 'TSTypePredicate';
  +asserts: boolean;
  +parameterName: Identifier | TSThisType;
  +typeAnnotation: TSTypeAnnotation | null;
}
export interface TSTypeQuery extends BaseNode {
  +type: 'TSTypeQuery';
  +exprName: EntityName;
  +typeParameters?: TSTypeParameterInstantiation;
}
export interface TSTypeReference extends BaseNode {
  +type: 'TSTypeReference';
  +typeName: EntityName;
  +typeParameters?: TSTypeParameterInstantiation;
}
export type TSUnaryExpression =
  | AwaitExpression
  | LeftHandSideExpression
  | UnaryExpression
  | UpdateExpression;
export interface TSUndefinedKeyword extends BaseNode {
  +type: 'TSUndefinedKeyword';
}
export interface TSUnionType extends BaseNode {
  +type: 'TSUnionType';
  +types: $ReadOnlyArray<TypeNode>;
}
export interface TSUnknownKeyword extends BaseNode {
  +type: 'TSUnknownKeyword';
}
export interface TSVoidKeyword extends BaseNode {
  +type: 'TSVoidKeyword';
}
export type TypeElement =
  | TSCallSignatureDeclaration
  | TSConstructSignatureDeclaration
  | TSIndexSignature
  | TSMethodSignature
  | TSPropertySignature;
export type TypeNode =
  | TSAbstractKeyword
  | TSAnyKeyword
  | TSArrayType
  | TSAsyncKeyword
  | TSBigIntKeyword
  | TSBooleanKeyword
  | TSConditionalType
  | TSConstructorType
  | TSDeclareKeyword
  | TSExportKeyword
  | TSFunctionType
  | TSImportType
  | TSIndexedAccessType
  | TSInferType
  | TSIntersectionType
  | TSIntrinsicKeyword
  | TSLiteralType
  | TSMappedType
  | TSNamedTupleMember
  | TSNeverKeyword
  | TSNullKeyword
  | TSNumberKeyword
  | TSObjectKeyword
  | TSOptionalType
  | TSQualifiedName
  | TSPrivateKeyword
  | TSProtectedKeyword
  | TSPublicKeyword
  | TSReadonlyKeyword
  | TSRestType
  | TSStaticKeyword
  | TSStringKeyword
  | TSSymbolKeyword
  | TSTemplateLiteralType
  | TSThisType
  | TSTupleType
  | TSTypeLiteral
  | TSTypeOperator
  | TSTypePredicate
  | TSTypeQuery
  | TSTypeReference
  | TSUndefinedKeyword
  | TSUnionType
  | TSUnknownKeyword
  | TSVoidKeyword;
export interface UnaryExpression extends UnaryExpressionBase {
  +type: 'UnaryExpression';
  +operator: '-' | '!' | '+' | '~' | 'delete' | 'typeof' | 'void';
}
interface UnaryExpressionBase extends BaseNode {
  +operator: string;
  +prefix: boolean;
  +argument: LeftHandSideExpression | Literal | UnaryExpression;
}
export interface UpdateExpression extends UnaryExpressionBase {
  +type: 'UpdateExpression';
  +operator: '--' | '++';
}
export interface VariableDeclaration extends BaseNode {
  +type: 'VariableDeclaration';
  /**
   * The variables declared by this declaration.
   * Note that there may be 0 declarations (i.e. `const;`).
   * ```
   * let x;
   * let y, z;
   * ```
   */
  +declarations: $ReadOnlyArray<VariableDeclarator>;
  /**
   * Whether the declaration is `declare`d
   * ```
   * declare const x = 1;
   * ```
   */
  +declare?: boolean;
  /**
   * The keyword used to declare the variable(s)
   * ```
   * const x = 1;
   * let y = 2;
   * var z = 3;
   * ```
   */
  +kind: 'const' | 'let' | 'var';
}
export interface VariableDeclarator extends BaseNode {
  +type: 'VariableDeclarator';
  +id: BindingName;
  +init: Expression | null;
  +definite?: boolean;
}
export interface WhileStatement extends BaseNode {
  +type: 'WhileStatement';
  +test: Expression;
  +body: Statement;
}
export interface WithStatement extends BaseNode {
  +type: 'WithStatement';
  +object: Expression;
  +body: Statement;
}
export interface YieldExpression extends BaseNode {
  +type: 'YieldExpression';
  +delegate: boolean;
  +argument?: Expression;
}
