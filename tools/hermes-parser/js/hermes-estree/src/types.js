/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

/**
 *
 * IMPORTANT NOTE
 *
 * This file intentionally uses interfaces and `+` for readonly.
 *
 * - `$ReadOnly` is an "evaluated" utility type in flow; meaning that flow does
 *    not actually calculate the resulting type until it is used. This creates
 *    a copy of the type at each usage site - ballooning memory and processing
 *    times.
 *    Usually this isn't a problem as a type might only be used one or two times
 *    - but in this giant circular-referencing graph that is the AST types, this
 *    causes check times for consumers to be awful.
 *
 *    Thus instead we manually annotate properties with `+` to avoid the `$ReadOnly` type.
 *
 * - `...Type` spreads do not preserve the readonly-ness of the properties. If
 *   we used object literal types then we would have to `$ReadOnly` all spreads
 *   (see point 1). On the other hand extending an interface does preserve
 *   readonlyness of properties.
 *
 *   Thus instead of object literals, we use interfaces.
 *
 *** Please ensure all properties are marked as readonly! ***
 */

export type Range = [number, number];

export interface BaseToken {
  +loc: SourceLocation;
  +range: Range;
}
export interface BaseNode extends BaseToken {
  // this is added by ESLint and is not part of the ESTree spec
  +parent: ESNode;
}

/*
 * Token and Comment are pseudo-nodes to represent pieces of source code
 *
 * NOTE:
 * They are not included in the `ESNode` union below on purpose because they
 * are not ever included as part of the standard AST tree.
 */

export interface MostTokens extends BaseToken {
  +type:
    | 'Boolean'
    | 'Identifier'
    | 'JSXIdentifier'
    | 'JSXText'
    | 'Keyword'
    | 'Null'
    | 'Numeric'
    | 'Punctuator'
    | 'RegularExpression'
    | 'String'
    | 'Template'
    // comment types
    | 'Block'
    | 'Line';
  +value: string;
}
export interface RegexToken extends BaseToken {
  +type: 'RegularExpression';
  +value: string;
  +regex: {
    +pattern: string,
    +flags: string,
  };
}
export interface LineComment extends BaseToken {
  +type: 'Line';
  +value: string;
}
export interface BlockComment extends BaseToken {
  +type: 'Block';
  +value: string;
}
export type Comment = LineComment | BlockComment;
export type Token = MostTokens | RegexToken | Comment;

export interface SourceLocation {
  +start: Position;
  +end: Position;
}

export interface Position {
  /** >= 1 */
  +line: number;
  /** >= 0 */
  +column: number;
}

export interface Program extends BaseNode {
  +type: 'Program';
  +sourceType: 'script' | 'module';
  +body: $ReadOnlyArray<Statement | ModuleDeclaration>;
  +tokens: $ReadOnlyArray<Token>;
  +comments: $ReadOnlyArray<Comment>;
  +loc: SourceLocation;
  // program is the only node without a parent - but typing it as such is _super_ annoying and difficult
  +parent: ESNode;
}

// Flow declares a "Node" type as part of its HTML typedefs.
// Because this file declares global types - we can't clash with it
export type ESNode =
  | Identifier
  | Literal
  | Program
  | AFunction
  | SwitchCase
  | CatchClause
  | VariableDeclarator
  | Statement
  | Expression
  | Property
  | Super
  | TemplateElement
  | SpreadElement
  | Pattern
  | ClassBody
  | AClass
  | MethodDefinition
  | ModuleDeclaration
  | ModuleSpecifier
  | ImportAttribute
  // flow nodes
  | TypeAnnotation
  | TypeAnnotationType
  | Variance
  | FunctionTypeParam
  | InferredPredicate
  | ObjectTypeProperty
  | ObjectTypeCallProperty
  | ObjectTypeIndexer
  | ObjectTypeSpreadProperty
  | InterfaceExtends
  | ClassProperty
  | ClassPrivateProperty
  | ClassImplements
  | Decorator
  | TypeParameterDeclaration
  | TypeParameter
  | TypeParameterInstantiation
  | EnumDeclaration
  | EnumNumberBody
  | EnumStringBody
  | EnumStringMember
  | EnumDefaultedMember
  | EnumNumberMember
  | EnumBooleanBody
  | EnumBooleanMember
  | EnumSymbolBody
  | DeclareClass
  | DeclareVariable
  | DeclareFunction
  | DeclaredPredicate
  | DeclareModule
  | ObjectTypeInternalSlot
  // JSX
  | JSXNode;

interface BaseFunction extends BaseNode {
  +params: $ReadOnlyArray<Pattern>;
  +async: boolean;

  +predicate: null | InferredPredicate;
  +returnType: null | TypeAnnotation;
  +typeParameters: null | TypeParameterDeclaration;
}

export type AFunction =
  | FunctionDeclaration
  | FunctionExpression
  | ArrowFunctionExpression;

export type Statement =
  | ExpressionStatement
  | BlockStatement
  | EmptyStatement
  | DebuggerStatement
  | WithStatement
  | ReturnStatement
  | LabeledStatement
  | BreakStatement
  | ContinueStatement
  | IfStatement
  | SwitchStatement
  | ThrowStatement
  | TryStatement
  | WhileStatement
  | DoWhileStatement
  | ForStatement
  | ForInStatement
  | ForOfStatement
  | TypeAlias
  | OpaqueType
  | InterfaceDeclaration
  | Declaration
  | DeclareTypeAlias
  | DeclareOpaqueType
  | DeclareInterface
  | DeclareModule;

// nodes that can be the direct parent of a statement
export type StatementParentSingle =
  | IfStatement
  | LabeledStatement
  | WithStatement
  | WhileStatement
  | DoWhileStatement
  | ForStatement
  | ForInStatement
  | ForOfStatement;
// nodes that can be the parent of a statement that store the statements in an array
export type StatementParentArray = SwitchCase | Program | BlockStatement;
export type StatementParent = StatementParentSingle | StatementParentArray;

export interface EmptyStatement extends BaseNode {
  +type: 'EmptyStatement';
}

export interface BlockStatement extends BaseNode {
  +type: 'BlockStatement';
  +body: $ReadOnlyArray<Statement>;
}

export interface ExpressionStatement extends BaseNode {
  +type: 'ExpressionStatement';
  +expression: Expression;
  +directive: string | null;
}

export interface IfStatement extends BaseNode {
  +type: 'IfStatement';
  +test: Expression;
  +consequent: Statement;
  +alternate?: Statement | null;
}

export interface LabeledStatement extends BaseNode {
  +type: 'LabeledStatement';
  +label: Identifier;
  +body: Statement;
}

export interface BreakStatement extends BaseNode {
  +type: 'BreakStatement';
  +label?: Identifier | null;
}

export interface ContinueStatement extends BaseNode {
  +type: 'ContinueStatement';
  +label?: Identifier | null;
}

export interface WithStatement extends BaseNode {
  +type: 'WithStatement';
  +object: Expression;
  +body: Statement;
}

export interface SwitchStatement extends BaseNode {
  +type: 'SwitchStatement';
  +discriminant: Expression;
  +cases: $ReadOnlyArray<SwitchCase>;
}

export interface ReturnStatement extends BaseNode {
  +type: 'ReturnStatement';
  +argument?: Expression | null;
}

export interface ThrowStatement extends BaseNode {
  +type: 'ThrowStatement';
  +argument: Expression;
}

export interface TryStatement extends BaseNode {
  +type: 'TryStatement';
  +block: BlockStatement;
  +handler?: CatchClause | null;
  +finalizer?: BlockStatement | null;
}

export interface WhileStatement extends BaseNode {
  +type: 'WhileStatement';
  +test: Expression;
  +body: Statement;
}

export interface DoWhileStatement extends BaseNode {
  +type: 'DoWhileStatement';
  +body: Statement;
  +test: Expression;
}

export interface ForStatement extends BaseNode {
  +type: 'ForStatement';
  +init?: VariableDeclaration | Expression | null;
  +test?: Expression | null;
  +update?: Expression | null;
  +body: Statement;
}

interface BaseForXStatement extends BaseNode {
  +left: VariableDeclaration | Pattern;
  +right: Expression;
  +body: Statement;
}

export interface ForInStatement extends BaseForXStatement {
  +type: 'ForInStatement';
}

export interface ForOfStatement extends BaseForXStatement {
  +type: 'ForOfStatement';
  +await: boolean;
}

export interface DebuggerStatement extends BaseNode {
  +type: 'DebuggerStatement';
}

export type Declaration =
  | FunctionDeclaration
  | VariableDeclaration
  | ClassDeclaration;

export interface FunctionDeclaration extends BaseFunction {
  +type: 'FunctionDeclaration';
  /** It is null when a function declaration is a part of the `export default function` statement */
  +id: Identifier | null;
  +body: BlockStatement;
  +generator: boolean;
}

export interface VariableDeclaration extends BaseNode {
  +type: 'VariableDeclaration';
  +declarations: $ReadOnlyArray<VariableDeclarator>;
  +kind: 'var' | 'let' | 'const';
}

export interface VariableDeclarator extends BaseNode {
  +type: 'VariableDeclarator';
  +id: Pattern;
  +init?: Expression | null;
}

export type Expression =
  | ThisExpression
  | ArrayExpression
  | ObjectExpression
  | FunctionExpression
  | ArrowFunctionExpression
  | YieldExpression
  | Literal
  | UnaryExpression
  | UpdateExpression
  | BinaryExpression
  | AssignmentExpression
  | LogicalExpression
  | MemberExpression
  | OptionalMemberExpression
  | ConditionalExpression
  | CallExpression
  | OptionalCallExpression
  | NewExpression
  | SequenceExpression
  | TemplateLiteral
  | TaggedTemplateExpression
  | ClassExpression
  | MetaProperty
  | Identifier
  | AwaitExpression
  | ImportExpression
  | ChainExpression
  | TypeCastExpression
  | PrivateName;

export interface ThisExpression extends BaseNode {
  +type: 'ThisExpression';
}

export interface ArrayExpression extends BaseNode {
  +type: 'ArrayExpression';
  +elements: $ReadOnlyArray<Expression | SpreadElement>;
  // this is not part of the ESTree spec, but hermes emits it
  +trailingComma: boolean;
}

export interface ObjectExpression extends BaseNode {
  +type: 'ObjectExpression';
  +properties: $ReadOnlyArray<Property | SpreadElement>;
}

interface PropertyBase extends BaseNode {
  +key: Expression;
  +shorthand: boolean;
  +computed: boolean;
}

export interface Property extends PropertyBase {
  +type: 'Property';
  +value: Expression | Pattern; // Could be an AssignmentProperty
  +kind: 'init' | 'get' | 'set';
  +method: boolean;
}

// this is a special type of property
// we can't use `interface ... extends` here because it's a sub-type
export interface AssignmentProperty extends PropertyBase {
  +type: 'Property';
  +value: Pattern;
  +kind: 'init';
  +method: false;
}

export interface FunctionExpression extends BaseFunction {
  +id?: Identifier | null;
  +type: 'FunctionExpression';
  +body: BlockStatement;
  +generator: boolean;
}

export interface SequenceExpression extends BaseNode {
  +type: 'SequenceExpression';
  +expressions: $ReadOnlyArray<Expression>;
}

export interface UnaryExpression extends BaseNode {
  +type: 'UnaryExpression';
  +operator: UnaryOperator;
  +prefix: true;
  +argument: Expression;
}

export interface BinaryExpression extends BaseNode {
  +type: 'BinaryExpression';
  +operator: BinaryOperator;
  +left: Expression;
  +right: Expression;
}

export interface AssignmentExpression extends BaseNode {
  +type: 'AssignmentExpression';
  +operator: AssignmentOperator;
  +left: Pattern | MemberExpression;
  +right: Expression;
}

export interface UpdateExpression extends BaseNode {
  +type: 'UpdateExpression';
  +operator: UpdateOperator;
  +argument: Expression;
  +prefix: boolean;
}

export interface LogicalExpression extends BaseNode {
  +type: 'LogicalExpression';
  +operator: LogicalOperator;
  +left: Expression;
  +right: Expression;
}

export interface ConditionalExpression extends BaseNode {
  +type: 'ConditionalExpression';
  +test: Expression;
  +alternate: Expression;
  +consequent: Expression;
}

interface BaseCallExpression extends BaseNode {
  +callee: Expression | Super;
  +arguments: $ReadOnlyArray<Expression | SpreadElement>;
  +typeArguments: null | TypeParameterInstantiation;
}
export interface CallExpression extends BaseCallExpression {
  +type: 'CallExpression';
}

export interface NewExpression extends BaseCallExpression {
  +type: 'NewExpression';
}

interface BaseMemberExpression extends BaseNode {
  +object: Expression | Super;
  +property: Expression;
  +computed: boolean;
}
export interface MemberExpression extends BaseMemberExpression {
  +type: 'MemberExpression';
}

export type ChainElement = CallExpression | MemberExpression;

export interface ChainExpression extends BaseNode {
  +type: 'ChainExpression';
  +expression: ChainElement;
}

export type Pattern =
  | Identifier
  | ObjectPattern
  | ArrayPattern
  | RestElement
  | AssignmentPattern
  | MemberExpression;

export interface SwitchCase extends BaseNode {
  +type: 'SwitchCase';
  +test?: Expression | null;
  +consequent: $ReadOnlyArray<Statement>;
}

export interface CatchClause extends BaseNode {
  +type: 'CatchClause';
  +param: Pattern | null;
  +body: BlockStatement;
}

export interface Identifier extends BaseNode {
  +type: 'Identifier';
  +name: string;

  +typeAnnotation: TypeAnnotation | null;
  // only applies to function arguments
  +optional: boolean;
}

export type Literal =
  | BigIntLiteral
  | BigIntLiteralLegacy
  | BooleanLiteral
  | NullLiteral
  | NumericLiteral
  | RegExpLiteral
  | StringLiteral;

export interface BigIntLiteral extends BaseNode {
  +type: 'Literal';
  +value: null /* | bigint */;
  +bigint: string;
  +raw: string;
}

export interface BooleanLiteral extends BaseNode {
  +type: 'Literal';
  +value: boolean;
  +raw: 'true' | 'false';
}

export interface NullLiteral extends BaseNode {
  +type: 'Literal';
  +value: null;
  +raw: 'null';
}

export interface NumericLiteral extends BaseNode {
  +type: 'Literal';
  +value: number;
  +raw: string;
}

export interface RegExpLiteral extends BaseNode {
  +type: 'Literal';
  +value: RegExp | null;
  +regex: interface {
    +pattern: string,
    +flags: string,
  };
  +raw: string;
}

export interface StringLiteral extends BaseNode {
  +type: 'Literal';
  +value: string;
  +raw: string;
}

export type UnaryOperator =
  | '-'
  | '+'
  | '!'
  | '~'
  | 'typeof'
  | 'void'
  | 'delete';

export type BinaryOperator =
  | '=='
  | '!='
  | '==='
  | '!=='
  | '<'
  | '<='
  | '>'
  | '>='
  | '<<'
  | '>>'
  | '>>>'
  | '+'
  | '-'
  | '*'
  | '/'
  | '%'
  | '**'
  | '|'
  | '^'
  | '&'
  | 'in'
  | 'instanceof';

export type LogicalOperator = '||' | '&&';

export type AssignmentOperator =
  | '='
  | '+='
  | '-='
  | '*='
  | '/='
  | '%='
  | '**='
  | '<<='
  | '>>='
  | '>>>='
  | '|='
  | '^='
  | '&=';

export type UpdateOperator = '++' | '--';

export interface Super extends BaseNode {
  +type: 'Super';
}

export interface SpreadElement extends BaseNode {
  +type: 'SpreadElement';
  +argument: Expression;
}

export interface ArrowFunctionExpression extends BaseFunction {
  +type: 'ArrowFunctionExpression';
  +expression: boolean;
  +body: BlockStatement | Expression;
  // hermes emits this - but it's always null
  +id: null;
  // note - arrow functions cannot be generators
}

export interface YieldExpression extends BaseNode {
  +type: 'YieldExpression';
  +argument?: Expression | null;
  +delegate: boolean;
}

export interface TemplateLiteral extends BaseNode {
  +type: 'TemplateLiteral';
  +quasis: $ReadOnlyArray<TemplateElement>;
  +expressions: $ReadOnlyArray<Expression>;
}

export interface TaggedTemplateExpression extends BaseNode {
  +type: 'TaggedTemplateExpression';
  +tag: Expression;
  +quasi: TemplateLiteral;
}

export interface TemplateElement extends BaseNode {
  +type: 'TemplateElement';
  +tail: boolean;
  +value: interface {
    +cooked: string,
    +raw: string,
  };
}

export interface ObjectPattern extends BaseNode {
  +type: 'ObjectPattern';
  +properties: $ReadOnlyArray<AssignmentProperty | RestElement>;
  // if used as a VariableDeclarator.id
  +typeAnnotation: TypeAnnotation | null;
}

export interface ArrayPattern extends BaseNode {
  +type: 'ArrayPattern';
  +elements: $ReadOnlyArray<Pattern>;

  +typeAnnotation: TypeAnnotation | null;
}

export interface RestElement extends BaseNode {
  +type: 'RestElement';
  +argument: Pattern;
  // the Pattern owns the typeAnnotation
}

export interface AssignmentPattern extends BaseNode {
  +type: 'AssignmentPattern';
  +left: Pattern;
  +right: Expression;
}

export type AClass = ClassDeclaration | ClassExpression;
interface BaseClass extends BaseNode {
  +superClass?: Expression | null;
  +body: ClassBody;

  +typeParameters: null | TypeParameterDeclaration;
  +superTypeParameters: null | TypeParameterDeclaration;
  +implements: $ReadOnlyArray<ClassImplements>;
  +decorators: $ReadOnlyArray<Decorator>;
}

export interface ClassBody extends BaseNode {
  +type: 'ClassBody';
  +body: $ReadOnlyArray<
    ClassProperty | ClassPrivateProperty | MethodDefinition,
  >;
}

export interface MethodDefinition extends BaseNode {
  +type: 'MethodDefinition';
  +key: Expression;
  +value: FunctionExpression;
  +kind: 'constructor' | 'method' | 'get' | 'set';
  +computed: boolean;
  +static: boolean;
}

export interface ClassDeclaration extends BaseClass {
  +type: 'ClassDeclaration';
  /** It is null when a class declaration is a part of the `export default class` statement */
  +id: Identifier | null;
}

export interface ClassExpression extends BaseClass {
  +type: 'ClassExpression';
  +id?: Identifier | null;
}

export interface MetaProperty extends BaseNode {
  +type: 'MetaProperty';
  +meta: Identifier;
  +property: Identifier;
}

export type ModuleDeclaration =
  | ImportDeclaration
  | ExportNamedDeclaration
  | ExportDefaultDeclaration
  | ExportAllDeclaration
  | DeclareExportDeclaration
  | DeclareExportAllDeclaration
  | DeclareModuleExports;

export type ModuleSpecifier =
  | ImportSpecifier
  | ImportDefaultSpecifier
  | ImportNamespaceSpecifier
  | ExportSpecifier
  | ExportNamespaceSpecifier;

export interface ImportDeclaration extends BaseNode {
  +type: 'ImportDeclaration';
  +specifiers: $ReadOnlyArray<
    ImportSpecifier | ImportDefaultSpecifier | ImportNamespaceSpecifier,
  >;
  +source: StringLiteral;
  +assertions: $ReadOnlyArray<ImportAttribute>;

  +importKind: 'value' | 'type' | 'typeof';
}
export interface ImportAttribute extends BaseNode {
  +type: 'ImportAttribute';
  +key: Identifier;
  +value: StringLiteral;
}

export interface ImportSpecifier extends BaseNode {
  +type: 'ImportSpecifier';
  +imported: Identifier;
  +local: Identifier;
  +importKind: null | 'type' | 'typeof';
}

export interface ImportExpression extends BaseNode {
  +type: 'ImportExpression';
  +source: Expression;
  +attributes: $ReadOnlyArray<ImportAttribute> | null;
}

export interface ImportDefaultSpecifier extends BaseNode {
  +type: 'ImportDefaultSpecifier';
  +local: Identifier;
}

export interface ImportNamespaceSpecifier extends BaseNode {
  +type: 'ImportNamespaceSpecifier';
  +local: Identifier;
}

export interface ExportNamedDeclaration extends BaseNode {
  +type: 'ExportNamedDeclaration';
  +declaration?: Declaration | null;
  +specifiers: $ReadOnlyArray<ExportSpecifier | ExportNamespaceSpecifier>;
  +source?: Literal | null;
  +exportKind: 'value' | 'type';
}

export interface ExportSpecifier extends BaseNode {
  +type: 'ExportSpecifier';
  +exported: Identifier;
  +local: Identifier;
}

export interface ExportDefaultDeclaration extends BaseNode {
  +type: 'ExportDefaultDeclaration';
  +declaration: Declaration | Expression;
}

export interface ExportAllDeclaration extends BaseNode {
  +type: 'ExportAllDeclaration';
  +source: Literal;
  +exportKind: 'value' | 'type';
  // uncomment this when hermes stops using ExportNamespaceSpecifier
  // +exported: Identifier;
}

export interface AwaitExpression extends BaseNode {
  +type: 'AwaitExpression';
  +argument: Expression;
}

/***********************
 * Flow specific nodes *
 ***********************/

export type TypeAnnotationType =
  | NumberTypeAnnotation
  | StringTypeAnnotation
  | BooleanTypeAnnotation
  | NullLiteralTypeAnnotation
  | AnyTypeAnnotation
  | EmptyTypeAnnotation
  | SymbolTypeAnnotation
  | ThisTypeAnnotation
  | MixedTypeAnnotation
  | VoidTypeAnnotation
  | StringLiteralTypeAnnotation
  | NumberLiteralTypeAnnotation
  | BooleanLiteralTypeAnnotation
  | ArrayTypeAnnotation
  | NullableTypeAnnotation
  | ExistsTypeAnnotation
  | GenericTypeAnnotation
  | QualifiedTypeIdentifier
  | TypeofTypeAnnotation
  | TupleTypeAnnotation
  | InterfaceTypeAnnotation
  | UnionTypeAnnotation
  | IntersectionTypeAnnotation
  | FunctionTypeAnnotation
  | ObjectTypeAnnotation
  | IndexedAccessType
  | OptionalIndexedAccessType;

export interface Variance extends BaseNode {
  +type: 'Variance';
  +kind: 'plus' | 'minus';
}

interface BaseTypeAlias extends BaseNode {
  +id: Identifier;
  +typeParameters: null | TypeParameterDeclaration;
  +right: TypeAnnotationType;
}

export interface TypeAnnotation extends BaseNode {
  +type: 'TypeAnnotation';
  +typeAnnotation: TypeAnnotationType;
}

export interface TypeAlias extends BaseTypeAlias {
  +type: 'TypeAlias';
}

interface BaseOpaqueType extends BaseNode {
  +id: Identifier;
  +supertype: TypeAnnotationType | null;
  +typeParameters: TypeParameterDeclaration | null;
}
export interface OpaqueType extends BaseOpaqueType {
  +type: 'OpaqueType';
  +impltype: TypeAnnotationType;
}

export interface NumberTypeAnnotation extends BaseNode {
  +type: 'NumberTypeAnnotation';
}
export interface StringTypeAnnotation extends BaseNode {
  +type: 'StringTypeAnnotation';
}
export interface BooleanTypeAnnotation extends BaseNode {
  +type: 'BooleanTypeAnnotation';
}
export interface NullLiteralTypeAnnotation extends BaseNode {
  +type: 'NullLiteralTypeAnnotation';
}
export interface AnyTypeAnnotation extends BaseNode {
  +type: 'AnyTypeAnnotation';
}
export interface EmptyTypeAnnotation extends BaseNode {
  +type: 'EmptyTypeAnnotation';
}
export interface SymbolTypeAnnotation extends BaseNode {
  +type: 'SymbolTypeAnnotation';
}
export interface ThisTypeAnnotation extends BaseNode {
  +type: 'ThisTypeAnnotation';
}
export interface MixedTypeAnnotation extends BaseNode {
  +type: 'MixedTypeAnnotation';
}
export interface VoidTypeAnnotation extends BaseNode {
  +type: 'VoidTypeAnnotation';
}
export interface StringLiteralTypeAnnotation extends BaseNode {
  +type: 'StringLiteralTypeAnnotation';
  +value: string;
}
export interface NumberLiteralTypeAnnotation extends BaseNode {
  +type: 'NumberLiteralTypeAnnotation';
  +value: number;
  +raw: string;
}
export interface BooleanLiteralTypeAnnotation extends BaseNode {
  +type: 'BooleanLiteralTypeAnnotation';
  +value: boolean;
  +raw: string;
}
export interface ArrayTypeAnnotation extends BaseNode {
  +type: 'ArrayTypeAnnotation';
  +elementType: TypeAnnotationType;
}
export interface NullableTypeAnnotation extends BaseNode {
  +type: 'NullableTypeAnnotation';
  +typeAnnotation: TypeAnnotationType;
}
export interface ExistsTypeAnnotation extends BaseNode {
  +type: 'ExistsTypeAnnotation';
}
export interface GenericTypeAnnotation extends BaseNode {
  +type: 'GenericTypeAnnotation';
  +id: Identifier;
  +typeParameters: null | TypeParameterInstantiation;
}
export interface QualifiedTypeIdentifier extends BaseNode {
  +type: 'QualifiedTypeIdentifier';
  +id: Identifier;
  +qualification: QualifiedTypeIdentifier | Identifier;
}
export interface TypeofTypeAnnotation extends BaseNode {
  +type: 'TypeofTypeAnnotation';
  +argument: GenericTypeAnnotation | QualifiedTypeIdentifier;
}
export interface TupleTypeAnnotation extends BaseNode {
  +type: 'TupleTypeAnnotation';
  +types: TypeAnnotationType;
}

// type T = { [[foo]]: number };
export interface ObjectTypeInternalSlot extends BaseNode {
  +type: 'ObjectTypeInternalSlot';
  +id: Identifier;
  +optional: boolean;
  +static: boolean;
  +method: boolean;
  +value: TypeAnnotation;
}

export interface InterfaceTypeAnnotation extends BaseInterfaceNode {
  +type: 'InterfaceTypeAnnotation';
}

export interface UnionTypeAnnotation extends BaseNode {
  +type: 'UnionTypeAnnotation';
  +types: $ReadOnlyArray<TypeAnnotationType>;
}
export interface IntersectionTypeAnnotation extends BaseNode {
  +type: 'IntersectionTypeAnnotation';
  +types: $ReadOnlyArray<TypeAnnotationType>;
}

export interface FunctionTypeAnnotation extends BaseNode {
  +type: 'FunctionTypeAnnotation';
  +params: $ReadOnlyArray<FunctionTypeParam>;
  +returnType: TypeAnnotationType;
  +rest: null | FunctionTypeParam;
  +typeParameters: null | TypeParameterDeclaration;
  +this: TypeAnnotationType | null;
}
export interface FunctionTypeParam extends BaseNode {
  +type: 'FunctionTypeParam';
  +name: Identifier | null;
  +typeAnnotation: TypeAnnotationType;
  +optional: boolean;
}
export interface InferredPredicate extends BaseNode {
  +type: 'InferredPredicate';
}

export interface ObjectTypeAnnotation extends BaseNode {
  +type: 'ObjectTypeAnnotation';
  +inexact: false;
  +exact: boolean;
  +properties: $ReadOnlyArray<ObjectTypeProperty | ObjectTypeSpreadProperty>;
  +indexers: $ReadOnlyArray<ObjectTypeIndexer>;
  +callProperties: $ReadOnlyArray<ObjectTypeCallProperty>;
  +internalSlots: $ReadOnlyArray<ObjectTypeInternalSlot>;
}
export interface ObjectTypeProperty extends BaseNode {
  +type: 'ObjectTypeProperty';
  +key: Identifier;
  +value: TypeAnnotationType;
  +method: boolean;
  +optional: boolean;
  +static: false; // can't be static
  +proto: false; // ???
  +variance: Variance | null;
  +kind: 'init';
}
export interface ObjectTypeCallProperty extends BaseNode {
  +type: 'ObjectTypeCallProperty';
  +value: FunctionTypeAnnotation;
  +static: false; // can't be static
}
export interface ObjectTypeIndexer extends BaseNode {
  +type: 'ObjectTypeIndexer';
  +id: null | Identifier;
  +key: TypeAnnotationType;
  +value: TypeAnnotationType;
  +static: false; // can't be static
  +variance: null | Variance;
}
export interface ObjectTypeSpreadProperty extends BaseNode {
  +type: 'ObjectTypeSpreadProperty';
  +argument: TypeAnnotationType;
}

export interface IndexedAccessType extends BaseNode {
  +type: 'IndexedAccessType';
  +objectType: TypeAnnotation;
  +indexType: TypeAnnotation;
}
export interface OptionalIndexedAccessType extends BaseNode {
  +type: 'OptionalIndexedAccessType';
  +objectType: TypeAnnotation;
  +indexType: TypeAnnotation;
  +optional: boolean;
}

export interface TypeCastExpression extends BaseNode {
  +type: 'TypeCastExpression';
  +expression: Expression;
  +typeAnnotation: TypeAnnotation;
}

interface BaseInterfaceNode extends BaseNode {
  +body: ObjectTypeAnnotation;
  +extends: $ReadOnlyArray<InterfaceExtends>;
}
interface BaseInterfaceDeclaration extends BaseInterfaceNode {
  +id: Identifier;
  +typeParameters: null | TypeParameterDeclaration;
}

export interface InterfaceDeclaration extends BaseInterfaceDeclaration {
  +type: 'InterfaceDeclaration';
}

export interface InterfaceExtends extends BaseNode {
  +type: 'InterfaceExtends';
  +id: Identifier;
  +typeParameters: null | TypeParameterDeclaration;
}

interface ClassPropertyBase extends BaseNode {
  +key: Identifier;
  +value: null | Literal;
  +typeAnnotation: null | TypeAnnotationType;
  +static: boolean;
  +variance: null | Variance;
  +declare: boolean;
  // hermes always emit this as false
  +optional: false;
}

export interface ClassImplements extends BaseNode {
  +type: 'ClassImplements';
  +id: Identifier;
  +typeParameters: null | TypeParameterDeclaration;
}

export interface Decorator extends BaseNode {
  +type: 'Decorator';
  +expression: Expression;
}

export interface TypeParameterDeclaration extends BaseNode {
  +type: 'TypeParameterDeclaration';
  +params: $ReadOnlyArray<TypeParameter>;
}
export interface TypeParameter extends BaseNode {
  +type: 'TypeParameter';
  +name: string;
  +bound: null | TypeAnnotation;
  +variance: null | Variance;
  +default: null | TypeAnnotationType;
}
export interface TypeParameterInstantiation extends BaseNode {
  +type: 'TypeParameterInstantiation';
  +params: $ReadOnlyArray<TypeAnnotationType>;
}

export interface EnumDeclaration extends BaseNode {
  +type: 'EnumDeclaration';
  +id: Identifier;
  +body: EnumNumberBody | EnumStringBody | EnumBooleanBody | EnumSymbolBody;
}

interface BaseEnumBody extends BaseNode {
  +hasUnknownMembers: boolean;
}
interface BaseInferrableEnumBody extends BaseEnumBody {
  +explicitType: boolean;
}

export interface EnumNumberBody extends BaseInferrableEnumBody {
  +type: 'EnumNumberBody';
  // enum number members cannot be defaulted
  +members: $ReadOnlyArray<EnumNumberMember>;
  +explicitType: boolean;
}

export interface EnumNumberMember extends BaseNode {
  +type: 'EnumNumberMember';
  +id: Identifier;
  +init: NumericLiteral;
}

export interface EnumStringBody extends BaseInferrableEnumBody {
  +type: 'EnumStringBody';
  +members: $ReadOnlyArray<EnumStringMember | EnumDefaultedMember>;
}

export interface EnumStringMember extends BaseNode {
  +type: 'EnumStringMember';
  +id: Identifier;
  +init: StringLiteral;
}

export interface EnumBooleanBody extends BaseInferrableEnumBody {
  +type: 'EnumBooleanBody';
  // enum boolean members cannot be defaulted
  +members: $ReadOnlyArray<EnumBooleanMember>;
}

export interface EnumBooleanMember extends BaseNode {
  +type: 'EnumBooleanMember';
  +id: Identifier;
  +init: BooleanLiteral;
}

export interface EnumSymbolBody extends BaseEnumBody {
  +type: 'EnumSymbolBody';
  // enum symbol members can only be defaulted
  +members: $ReadOnlyArray<EnumDefaultedMember>;
}

export interface EnumDefaultedMember extends BaseNode {
  +type: 'EnumDefaultedMember';
  +id: Identifier;
}

/*****************
 * Declare nodes *
 *****************/

export interface DeclareClass extends BaseNode {
  +type: 'DeclareClass';
  +id: Identifier;
  +typeParameters: null | TypeParameterDeclaration;
  +extends: $ReadOnlyArray<InterfaceExtends>;
  +implements: $ReadOnlyArray<ClassImplements>;
  +body: ObjectTypeAnnotation;
  +mixins: $ReadOnlyArray<InterfaceExtends>;
}

export interface DeclareVariable extends BaseNode {
  +type: 'DeclareVariable';
  +id: Identifier;
}

export interface DeclareFunction extends BaseNode {
  +type: 'DeclareFunction';
  +id: Identifier;
  +predicate: InferredPredicate | null;
}

export interface DeclareModule extends BaseNode {
  +type: 'DeclareModule';
  +id: StringLiteral | Identifier;
  +body: BlockStatement;
  +kind: 'CommonJS' | 'ES';
}

export interface DeclareInterface extends BaseInterfaceDeclaration {
  +type: 'DeclareInterface';
}

export interface DeclareTypeAlias extends BaseTypeAlias {
  +type: 'DeclareTypeAlias';
}

export interface DeclareOpaqueType extends BaseOpaqueType {
  +type: 'DeclareOpaqueType';
  +impltype: null;
}

export interface DeclareExportAllDeclaration extends BaseNode {
  +type: 'DeclareExportAllDeclaration';
  +source: StringLiteral;
}

export interface DeclareExportDeclaration extends BaseNode {
  +type: 'DeclareExportDeclaration';
  +specifiers: $ReadOnlyArray<ExportSpecifier>;
  +declaration:
    | TypeAnnotationType
    | DeclareClass
    | DeclareFunction
    | DeclareOpaqueType
    | DeclareInterface
    | null;
  +source: StringLiteral | null;
  +default: boolean;
}

export interface DeclareModuleExports extends BaseNode {
  +type: 'DeclareModuleExports';
  +typeAnnotation: TypeAnnotation;
}

export interface DeclaredPredicate extends BaseNode {
  +type: 'DeclaredPredicate';
  +value: Expression;
}

/**********************
 * JSX specific nodes *
 **********************/

export type JSXChild =
  | JSXElement
  | JSXExpression
  | JSXFragment
  | JSXText
  | JSXSpreadChild;
export type JSXExpression = JSXEmptyExpression | JSXExpressionContainer;
export type JSXTagNameExpression =
  | JSXIdentifier
  | JSXMemberExpression
  | JSXNamespacedName;

export type JSXNode =
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
  | JSXText
  | JSXSpreadChild;

export interface JSXAttribute extends BaseNode {
  +type: 'JSXAttribute';
  +name: JSXIdentifier;
  +value: Literal | JSXExpression | null;
}

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

export interface JSXExpressionContainer extends BaseNode {
  +type: 'JSXExpressionContainer';
  +expression: Expression;
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

export interface JSXText extends BaseNode {
  +type: 'JSXText';
  +value: string;
  +raw: string;
}

export interface JSXSpreadChild extends BaseNode {
  +type: 'JSXSpreadChild';
  +expression: Expression;
}

/******************************************************
 * Deprecated spec nodes awaiting migration by Hermes *
 ******************************************************/

// `ChainExpression` is the new standard for optional chaining
export interface OptionalCallExpression extends BaseCallExpression {
  +type: 'OptionalCallExpression';
  +optional: boolean;
}
export interface OptionalMemberExpression extends BaseMemberExpression {
  +type: 'OptionalMemberExpression';
  +optional: boolean;
}

// `ExportAllDeclaration` is the new standard for `export * as y from 'z'`
export interface ExportNamespaceSpecifier extends BaseNode {
  +type: 'ExportNamespaceSpecifier';
  +exported: Identifier;
}

// `Literal` is the new standard for bigints (see the BigIntLiteral interface)
export interface BigIntLiteralLegacy extends BaseNode {
  +type: 'BigIntLiteral';
  +value: null /* | bigint */;
  +bigint: string;
}

// `PropertyDefinition` is the new standard for all class properties
export interface ClassProperty extends ClassPropertyBase {
  +type: 'ClassProperty';
  +computed: false; // flow itself doesn't support computed ClassProperties, even though they might parse fine.
}
export interface ClassPrivateProperty extends ClassPropertyBase {
  +type: 'ClassPrivateProperty';
}

// `PrivateIdentifier` is the new standard for #private identifiers
export interface PrivateName extends BaseNode {
  +type: 'PrivateName';
  +id: Identifier;
}

export {};
