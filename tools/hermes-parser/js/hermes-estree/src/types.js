/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

export interface ObjectWithLoc {
  +loc: SourceLocation;
}
export interface BaseToken extends ObjectWithLoc {
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
    | 'BigInt'
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

// note: this is only ever present on Program.interpreter, never in the body
export interface InterpreterDirective extends BaseNode {
  type: 'InterpreterDirective';
  value: string;
}

export type DocblockDirectives = $ReadOnly<{
  // some well-known tags
  flow?: $ReadOnlyArray<string> | void,
  format?: $ReadOnlyArray<string> | void,
  noflow?: $ReadOnlyArray<string> | void,
  noformat?: $ReadOnlyArray<string> | void,
  [string]: $ReadOnlyArray<string> | void,
}>;

export type DocblockMetadata = $ReadOnly<{
  directives: DocblockDirectives,
  comment: BlockComment,
}>;

export interface Program extends BaseNode {
  +type: 'Program';
  +sourceType: 'script' | 'module';
  +body: $ReadOnlyArray<Statement | ModuleDeclaration>;
  +tokens: $ReadOnlyArray<Token>;
  +comments: $ReadOnlyArray<Comment>;
  +loc: SourceLocation;
  +interpreter: null | InterpreterDirective;
  +docblock: null | DocblockMetadata;
  // program is the only node without a parent - but typing it as such is _super_ annoying and difficult
  +parent: ESNode;
}

// Flow declares a "Node" type as part of its HTML typedefs.
// Because this file declares global types - we can't clash with it
export type ESNode =
  | Identifier
  | PrivateIdentifier
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
  | BindingName
  | RestElement
  | AssignmentPattern
  | MemberExpression
  | ClassBody
  | AClass
  | MethodDefinition
  | PropertyDefinition
  | ModuleDeclaration
  | ModuleSpecifier
  | ImportAttribute
  // flow nodes
  | TypeAnnotation
  | TypeAnnotationType
  | Variance
  | FunctionTypeParam
  | ComponentTypeParameter
  | InferredPredicate
  | ObjectTypeProperty
  | ObjectTypeCallProperty
  | ObjectTypeIndexer
  | ObjectTypeSpreadProperty
  | ObjectTypeMappedTypeProperty
  | InterfaceExtends
  | ClassImplements
  | Decorator
  | TypeParameterDeclaration
  | TypeParameter
  | TypeParameterInstantiation
  | ComponentDeclaration
  | ComponentParameter
  | EnumDeclaration
  | EnumNumberBody
  | EnumStringBody
  | EnumStringMember
  | EnumDefaultedMember
  | EnumNumberMember
  | EnumBooleanBody
  | EnumBooleanMember
  | EnumSymbolBody
  | DeclaredNode
  | ObjectTypeInternalSlot
  // JSX
  | JSXNode;

export type BindingName = Identifier | BindingPattern;
export type BindingPattern = ArrayPattern | ObjectPattern;
export type RestElementPattern = AssignmentPattern | BindingName | RestElement;
export type FunctionParameter = AssignmentPattern | BindingName | RestElement;
export type DestructuringPattern =
  | BindingName
  | AssignmentPattern
  | MemberExpression
  | RestElement;

interface BaseFunction extends BaseNode {
  +params: $ReadOnlyArray<FunctionParameter>;
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
  | BlockStatement
  | BreakStatement
  | ClassDeclaration
  | ComponentDeclaration
  | ContinueStatement
  | DebuggerStatement
  | DeclareClass
  | DeclareComponent
  | DeclareVariable
  | DeclareEnum
  | DeclareFunction
  | DeclareInterface
  | DeclareModule
  | DeclareOpaqueType
  | DeclareTypeAlias
  | DoWhileStatement
  | EmptyStatement
  | EnumDeclaration
  | ExpressionStatement
  | ForInStatement
  | ForOfStatement
  | ForStatement
  | FunctionDeclaration
  | IfStatement
  | InterfaceDeclaration
  | LabeledStatement
  | OpaqueType
  | ReturnStatement
  | SwitchStatement
  | ThrowStatement
  | TryStatement
  | TypeAlias
  | VariableDeclaration
  | WhileStatement
  | WithStatement;

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
  +left: VariableDeclaration | BindingName | MemberExpression;
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

type ComponentParameterAndRestElement = ComponentParameter | RestElement;

export interface ComponentParameter extends BaseNode {
  +type: 'ComponentParameter';
  +name: Identifier | StringLiteral;
  +local: BindingName | AssignmentPattern;
  +shorthand: boolean;
}

export interface ComponentDeclaration extends BaseNode {
  +type: 'ComponentDeclaration';
  +body: BlockStatement;
  +id: Identifier;
  +params: $ReadOnlyArray<ComponentParameterAndRestElement>;
  +rendersType: null | TypeAnnotation;
  +typeParameters: null | TypeParameterDeclaration;
}

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
  +id: BindingName;
  +init?: Expression | null;

  +parent: VariableDeclaration;
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
  | ConditionalExpression
  | CallExpression
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
  | JSXFragment
  | JSXElement;

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
  +properties: $ReadOnlyArray<ObjectProperty | SpreadElement>;
}

// This is the complete type of a "Property"
// This same node (unfortunately) covers both object literal properties
// and object desturcturing properties.
export type Property = ObjectProperty | DestructuringObjectProperty;

export type ObjectProperty =
  | ObjectPropertyWithNonShorthandStaticName
  | ObjectPropertyWithShorthandStaticName
  | ObjectPropertyWithComputedName;
interface ObjectPropertyBase extends BaseNode {
  +parent: ObjectExpression | ObjectPattern;
}
export interface ObjectPropertyWithNonShorthandStaticName
  extends ObjectPropertyBase {
  +type: 'Property';
  +computed: false;
  // non-computed, non-shorthand names are constrained significantly
  +key: Identifier | StringLiteral | NumericLiteral;
  +value: Expression;
  +kind: 'init' | 'get' | 'set';
  +method: boolean;
  +shorthand: false;
}
export interface ObjectPropertyWithShorthandStaticName
  extends ObjectPropertyBase {
  +type: 'Property';
  +computed: false;
  // shorthand keys *must* be identifiers
  +key: Identifier;
  // shorthand values *must* be identifiers (that look the same as the key)
  +value: Identifier;
  +kind: 'init';
  +method: false;
  +shorthand: true;
}
export interface ObjectPropertyWithComputedName extends ObjectPropertyBase {
  +type: 'Property';
  +computed: true;
  // computed names can be any expression
  +key: Expression;
  +value: Expression;
  +kind: 'init' | 'get' | 'set';
  +method: boolean;
  // cannot have a shorthand computed name
  +shorthand: false;
}

export type DestructuringObjectProperty =
  | DestructuringObjectPropertyWithNonShorthandStaticName
  | DestructuringObjectPropertyWithShorthandStaticName
  | DestructuringObjectPropertyWithComputedName;
interface DestructuringObjectPropertyBase extends BaseNode {
  // destructuring properties cannot be methods
  +kind: 'init';
  +method: false;

  +parent: ObjectExpression | ObjectPattern;
}
export interface DestructuringObjectPropertyWithNonShorthandStaticName
  extends DestructuringObjectPropertyBase {
  +type: 'Property';
  +computed: false;
  // non-computed, non-shorthand names are constrained significantly
  +key: Identifier | StringLiteral | NumericLiteral;
  // destructuring properties cannot have any value
  +value: DestructuringPattern;
  +shorthand: false;
}
export interface DestructuringObjectPropertyWithShorthandStaticName
  extends DestructuringObjectPropertyBase {
  +type: 'Property';
  +computed: false;
  // shorthand keys *must* be identifiers
  +key: Identifier;
  // shorthand values *must* be identifiers (that look the same as the key)
  +value: Identifier;
  +shorthand: true;
}
export interface DestructuringObjectPropertyWithComputedName
  extends DestructuringObjectPropertyBase {
  +type: 'Property';
  +computed: true;
  // computed names can be any expression
  +key: Expression;
  // destructuring properties cannot have any value
  +value: DestructuringPattern;
  // cannot have a shorthand computed name
  +shorthand: false;
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

export interface BinaryExpressionWithoutIn extends BaseNode {
  +type: 'BinaryExpression';
  +operator: BinaryOperatorWithoutIn;
  +left: Expression;
  +right: Expression;
}

// Private brand checks (#foo in bar) are a special case
// other binary expressions do not allow PrivateIdentifier in the left
export interface BinaryExpressionIn extends BaseNode {
  +type: 'BinaryExpression';
  +operator: 'in';
  +left: Expression | PrivateIdentifier;
  +right: Expression;
}

export type BinaryExpression = BinaryExpressionWithoutIn | BinaryExpressionIn;

export interface AssignmentExpression extends BaseNode {
  +type: 'AssignmentExpression';
  +operator: AssignmentOperator;
  +left: BindingName | MemberExpression;
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
  +optional: boolean;
}

export interface NewExpression extends BaseCallExpression {
  +type: 'NewExpression';
}

export type MemberExpression =
  | MemberExpressionWithComputedName
  | MemberExpressionWithNonComputedName;
export interface MemberExpressionWithComputedName extends BaseNode {
  +type: 'MemberExpression';
  +object: Expression | Super;
  +property: Expression;
  +computed: true;
  +optional: boolean;
}
export interface MemberExpressionWithNonComputedName extends BaseNode {
  +type: 'MemberExpression';
  +object: Expression | Super;
  +property: Identifier | PrivateIdentifier;
  +computed: false;
  +optional: boolean;
}

export type ChainElement = CallExpression | MemberExpression;

export interface ChainExpression extends BaseNode {
  +type: 'ChainExpression';
  +expression: ChainElement;
}

export interface SwitchCase extends BaseNode {
  +type: 'SwitchCase';
  +test?: Expression | null;
  +consequent: $ReadOnlyArray<Statement>;
}

export interface CatchClause extends BaseNode {
  +type: 'CatchClause';
  +param: BindingName | null;
  +body: BlockStatement;
}

export interface Identifier extends BaseNode {
  +type: 'Identifier';
  +name: string;

  +typeAnnotation: TypeAnnotation | null;
  // only applies to function arguments
  +optional: boolean;
}

export interface PrivateIdentifier extends BaseNode {
  +type: 'PrivateIdentifier';
  +name: string;
}

export type Literal =
  | BigIntLiteral
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
  +literalType: 'bigint';
}

export interface BooleanLiteral extends BaseNode {
  +type: 'Literal';
  +value: boolean;
  +raw: 'true' | 'false';
  +literalType: 'boolean';
}

export interface NullLiteral extends BaseNode {
  +type: 'Literal';
  +value: null;
  +raw: 'null';
  +literalType: 'null';
}

export interface NumericLiteral extends BaseNode {
  +type: 'Literal';
  +value: number;
  +raw: string;
  +literalType: 'numeric';
}

export interface RegExpLiteral extends BaseNode {
  +type: 'Literal';
  +value: RegExp | null;
  +regex: interface {
    +pattern: string,
    +flags: string,
  };
  +raw: string;
  +literalType: 'regexp';
}

export interface StringLiteral extends BaseNode {
  +type: 'Literal';
  +value: string;
  +raw: string;
  +literalType: 'string';
}

export type UnaryOperator =
  | '-'
  | '+'
  | '!'
  | '~'
  | 'typeof'
  | 'void'
  | 'delete';

export type BinaryOperatorWithoutIn =
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
  | 'instanceof';

export type BinaryOperator = BinaryOperatorWithoutIn | 'in';

export type LogicalOperator = '||' | '&&' | '??';

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
  | '&='
  // not yet supported, but future proofing
  | '||='
  | '&&='
  | '??=';

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
  +properties: $ReadOnlyArray<DestructuringObjectProperty | RestElement>;
  // if used as a VariableDeclarator.id
  +typeAnnotation: TypeAnnotation | null;
}

export interface ArrayPattern extends BaseNode {
  +type: 'ArrayPattern';
  // an element will be null if the pattern contains a hole: `[a,,b]`
  +elements: $ReadOnlyArray<?DestructuringPattern>;
  +typeAnnotation: TypeAnnotation | null;
}

export interface RestElement extends BaseNode {
  +type: 'RestElement';
  +argument: RestElementPattern;
  // the Pattern owns the typeAnnotation
}

export interface AssignmentPattern extends BaseNode {
  +type: 'AssignmentPattern';
  +left: BindingName;
  +right: Expression;
}

export type AClass = ClassDeclaration | ClassExpression;
interface BaseClass extends BaseNode {
  +superClass?: Expression | null;
  +body: ClassBody;

  +typeParameters: null | TypeParameterDeclaration;
  +superTypeParameters: null | TypeParameterInstantiation;
  +implements: $ReadOnlyArray<ClassImplements>;
  +decorators: $ReadOnlyArray<Decorator>;
}

export type PropertyName =
  | ClassPropertyNameComputed
  | ClassPropertyNameNonComputed;
export type ClassPropertyNameComputed = Expression;
export type ClassPropertyNameNonComputed =
  | PrivateIdentifier
  | Identifier
  | StringLiteral;

export type ClassMember = PropertyDefinition | MethodDefinition;
export type ClassMemberWithNonComputedName =
  | PropertyDefinitionWithNonComputedName
  | MethodDefinitionConstructor
  | MethodDefinitionWithNonComputedName;
export interface ClassBody extends BaseNode {
  +type: 'ClassBody';
  +body: $ReadOnlyArray<ClassMember>;

  +parent: AClass;
}

export type MethodDefinition =
  | MethodDefinitionConstructor
  | MethodDefinitionWithComputedName
  | MethodDefinitionWithNonComputedName;
interface MethodDefinitionBase extends BaseNode {
  +value: FunctionExpression;

  +parent: ClassBody;
}
export interface MethodDefinitionConstructor extends MethodDefinitionBase {
  +type: 'MethodDefinition';
  +key: Identifier | StringLiteral;
  +kind: 'constructor';
  +computed: false;
  +static: false;
}
export interface MethodDefinitionWithComputedName extends MethodDefinitionBase {
  +type: 'MethodDefinition';
  +key: ClassPropertyNameComputed;
  +kind: 'method' | 'get' | 'set';
  +computed: true;
  +static: boolean;
}
export interface MethodDefinitionWithNonComputedName
  extends MethodDefinitionBase {
  +type: 'MethodDefinition';
  +key: ClassPropertyNameNonComputed;
  +kind: 'method' | 'get' | 'set';
  +computed: false;
  +static: boolean;
}

// `PropertyDefinition` is the new standard for all class properties
export type PropertyDefinition =
  | PropertyDefinitionWithComputedName
  | PropertyDefinitionWithNonComputedName;
interface PropertyDefinitionBase extends BaseNode {
  +value: null | Expression;
  +typeAnnotation: null | TypeAnnotation;
  +static: boolean;
  +variance: null | Variance;
  +declare: boolean;
  // hermes always emit this as false
  +optional: false;

  +parent: ClassBody;
}
export interface PropertyDefinitionWithComputedName
  extends PropertyDefinitionBase {
  +type: 'PropertyDefinition';
  +key: ClassPropertyNameComputed;
  +computed: true;
}
export interface PropertyDefinitionWithNonComputedName
  extends PropertyDefinitionBase {
  +type: 'PropertyDefinition';
  +key: ClassPropertyNameNonComputed;
  +computed: false;
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
  | ExportSpecifier;

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

  +parent: ImportDeclaration | ImportExpression;
}

export interface ImportSpecifier extends BaseNode {
  +type: 'ImportSpecifier';
  +imported: Identifier;
  +local: Identifier;
  +importKind: null | 'type' | 'typeof';

  +parent: ImportDeclaration;
}

export interface ImportExpression extends BaseNode {
  +type: 'ImportExpression';
  +source: Expression;
  +attributes: $ReadOnlyArray<ImportAttribute> | null;
}

export interface ImportDefaultSpecifier extends BaseNode {
  +type: 'ImportDefaultSpecifier';
  +local: Identifier;

  +parent: ImportDeclaration;
}

export interface ImportNamespaceSpecifier extends BaseNode {
  +type: 'ImportNamespaceSpecifier';
  +local: Identifier;

  +parent: ImportDeclaration;
}

export type DefaultDeclaration =
  | FunctionDeclaration
  | ClassDeclaration
  | ComponentDeclaration;
export type NamedDeclaration =
  | DefaultDeclaration
  | VariableDeclaration
  | TypeAlias
  | OpaqueType
  | InterfaceDeclaration
  | EnumDeclaration;

interface ExportNamedDeclarationBase extends BaseNode {
  +type: 'ExportNamedDeclaration';
  +declaration?: NamedDeclaration | null;
  +specifiers: $ReadOnlyArray<ExportSpecifier>;
  +source?: StringLiteral | null;
  +exportKind: 'value' | 'type';
}
export interface ExportNamedDeclarationWithSpecifiers
  extends ExportNamedDeclarationBase {
  +type: 'ExportNamedDeclaration';
  +declaration: null;
  +source: null;
  +specifiers: $ReadOnlyArray<ExportSpecifier>;
}
export interface ExportNamedDeclarationWithDeclaration
  extends ExportNamedDeclarationBase {
  +type: 'ExportNamedDeclaration';
  +declaration: NamedDeclaration;
  +source: null;
  +specifiers: [];
}
export type ExportNamedDeclaration =
  | ExportNamedDeclarationWithSpecifiers
  | ExportNamedDeclarationWithDeclaration;

export interface ExportSpecifier extends BaseNode {
  +type: 'ExportSpecifier';
  +exported: Identifier;
  +local: Identifier;
}

export interface ExportDefaultDeclaration extends BaseNode {
  +type: 'ExportDefaultDeclaration';
  +declaration: DefaultDeclaration | Expression;
}

export interface ExportAllDeclaration extends BaseNode {
  +type: 'ExportAllDeclaration';
  +source: StringLiteral;
  +exportKind: 'value' | 'type';
  +exported?: Identifier | null;
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
  | BigIntTypeAnnotation
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
  | BigIntLiteralTypeAnnotation
  | BooleanLiteralTypeAnnotation
  | ArrayTypeAnnotation
  | NullableTypeAnnotation
  | ExistsTypeAnnotation
  | GenericTypeAnnotation
  | QualifiedTypeIdentifier
  | QualifiedTypeofIdentifier
  | TypeofTypeAnnotation
  | KeyofTypeAnnotation
  | TupleTypeAnnotation
  | TupleTypeSpreadElement
  | TupleTypeLabeledElement
  | InferTypeAnnotation
  | InterfaceTypeAnnotation
  | UnionTypeAnnotation
  | IntersectionTypeAnnotation
  | ConditionalTypeAnnotation
  | TypePredicate
  | FunctionTypeAnnotation
  | ComponentTypeAnnotation
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
export interface BigIntTypeAnnotation extends BaseNode {
  +type: 'BigIntTypeAnnotation';
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
  +raw: string;
}
export interface NumberLiteralTypeAnnotation extends BaseNode {
  +type: 'NumberLiteralTypeAnnotation';
  +value: number;
  +raw: string;
}
export interface BigIntLiteralTypeAnnotation extends BaseNode {
  +type: 'BigIntLiteralTypeAnnotation';
  +bigint: string;
  +value: null /* | bigint */;
  +raw: string;
}
export interface BooleanLiteralTypeAnnotation extends BaseNode {
  +type: 'BooleanLiteralTypeAnnotation';
  +value: boolean;
  +raw: 'true' | 'false';
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
  +id: Identifier | QualifiedTypeIdentifier;
  +typeParameters: null | TypeParameterInstantiation;
}
export interface QualifiedTypeIdentifier extends BaseNode {
  +type: 'QualifiedTypeIdentifier';
  +id: Identifier;
  +qualification: QualifiedTypeIdentifier | Identifier;
}
export interface QualifiedTypeofIdentifier extends BaseNode {
  +type: 'QualifiedTypeofIdentifier';
  +id: Identifier;
  +qualification: QualifiedTypeofIdentifier | Identifier;
}
export interface TypeofTypeAnnotation extends BaseNode {
  +type: 'TypeofTypeAnnotation';
  +argument: QualifiedTypeofIdentifier | Identifier;
}
export interface KeyofTypeAnnotation extends BaseNode {
  +type: 'KeyofTypeAnnotation';
  +argument: TypeAnnotationType;
}
export interface TupleTypeAnnotation extends BaseNode {
  +type: 'TupleTypeAnnotation';
  +types: $ReadOnlyArray<TypeAnnotationType>;
}
export interface TupleTypeSpreadElement extends BaseNode {
  +type: 'TupleTypeSpreadElement';
  +label?: Identifier | null;
  +typeAnnotation: TypeAnnotationType;
}
export interface TupleTypeLabeledElement extends BaseNode {
  +type: 'TupleTypeLabeledElement';
  +label: Identifier;
  +elementType: TypeAnnotationType;
  +optional: boolean;
  +variance: Variance | null;
}

export interface InferTypeAnnotation extends BaseNode {
  +type: 'InferTypeAnnotation';
  +typeParameter: TypeParameter;
}

// type T = { [[foo]]: number };
export interface ObjectTypeInternalSlot extends BaseNode {
  +type: 'ObjectTypeInternalSlot';
  +id: Identifier;
  +optional: boolean;
  +static: boolean;
  +method: boolean;
  +value: TypeAnnotation;

  +parent: ObjectTypeAnnotation;
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

export interface ConditionalTypeAnnotation extends BaseNode {
  +type: 'ConditionalTypeAnnotation';
  +checkType: TypeAnnotationType;
  +extendsType: TypeAnnotationType;
  +trueType: TypeAnnotationType;
  +falseType: TypeAnnotationType;
}

export interface TypePredicate extends BaseNode {
  +type: 'TypePredicate';
  +parameterName: Identifier;
  +typeAnnotation: TypeAnnotationType | null;
  +asserts: boolean;
}

export interface FunctionTypeAnnotation extends BaseNode {
  +type: 'FunctionTypeAnnotation';
  +params: $ReadOnlyArray<FunctionTypeParam>;
  +returnType: TypeAnnotationType;
  +rest: null | FunctionTypeParam;
  +typeParameters: null | TypeParameterDeclaration;
  +this: FunctionTypeParam | null;
}
export interface FunctionTypeParam extends BaseNode {
  +type: 'FunctionTypeParam';
  +name: Identifier | null;
  +typeAnnotation: TypeAnnotationType;
  +optional: boolean;

  +parent: FunctionTypeAnnotation;
}

export interface ComponentTypeAnnotation extends BaseNode {
  +type: 'ComponentTypeAnnotation';
  +params: $ReadOnlyArray<ComponentTypeParameter>;
  +rest: null | ComponentTypeParameter;
  +typeParameters: null | TypeParameterDeclaration;
  +rendersType: null | TypeAnnotationType;
}
export interface ComponentTypeParameter extends BaseNode {
  +type: 'ComponentTypeParameter';
  +name: Identifier | StringLiteral | null;
  +typeAnnotation: TypeAnnotationType;
  +optional: boolean;

  +parent: ComponentTypeAnnotation | DeclareComponent;
}

export interface InferredPredicate extends BaseNode {
  +type: 'InferredPredicate';

  +parent: AFunction | DeclareFunction;
}

export interface ObjectTypeAnnotation extends BaseNode {
  +type: 'ObjectTypeAnnotation';
  +inexact: false;
  +exact: boolean;
  +properties: $ReadOnlyArray<
    | ObjectTypeProperty
    | ObjectTypeSpreadProperty
    | ObjectTypeMappedTypeProperty,
  >;
  +indexers: $ReadOnlyArray<ObjectTypeIndexer>;
  +callProperties: $ReadOnlyArray<ObjectTypeCallProperty>;
  +internalSlots: $ReadOnlyArray<ObjectTypeInternalSlot>;
}
interface ObjectTypePropertyBase extends BaseNode {
  +type: 'ObjectTypeProperty';
  +key: Identifier | StringLiteral;
  +value: TypeAnnotationType;
  +method: boolean;
  +optional: boolean;
  +static: boolean; // only applies to the "declare class" case
  +proto: boolean; // only applies to the "declare class" case
  +variance: Variance | null;
  +kind: 'init' | 'get' | 'set';

  +parent: ObjectTypeAnnotation;
}
export interface ObjectTypeMethodSignature extends ObjectTypePropertyBase {
  +type: 'ObjectTypeProperty';
  +value: FunctionTypeAnnotation;
  +method: true;
  +optional: false;
  +variance: null;
  +kind: 'init';

  +parent: ObjectTypeAnnotation;
}
export interface ObjectTypePropertySignature extends ObjectTypePropertyBase {
  +type: 'ObjectTypeProperty';
  +value: TypeAnnotationType;
  +method: false;
  +optional: boolean;
  +variance: Variance | null;
  +kind: 'init';

  +parent: ObjectTypeAnnotation;
}
export interface ObjectTypeAccessorSignature extends ObjectTypePropertyBase {
  +type: 'ObjectTypeProperty';
  +value: FunctionTypeAnnotation;
  +method: false;
  +optional: false;
  +variance: null;
  +kind: 'get' | 'set';

  +parent: ObjectTypeAnnotation;
}
export type ObjectTypeProperty =
  | ObjectTypeMethodSignature
  | ObjectTypePropertySignature
  | ObjectTypeAccessorSignature;

export interface ObjectTypeCallProperty extends BaseNode {
  +type: 'ObjectTypeCallProperty';
  +value: FunctionTypeAnnotation;
  +static: boolean; // can only be static when defined on a declare class

  +parent: ObjectTypeAnnotation;
}
export interface ObjectTypeIndexer extends BaseNode {
  +type: 'ObjectTypeIndexer';
  +id: null | Identifier;
  +key: TypeAnnotationType;
  +value: TypeAnnotationType;
  +static: boolean; // can only be static when defined on a declare class
  +variance: null | Variance;

  +parent: ObjectTypeAnnotation;
}
export interface ObjectTypeMappedTypeProperty extends BaseNode {
  +type: 'ObjectTypeMappedTypeProperty';
  +keyTparam: TypeParameter;
  +propType: TypeAnnotationType;
  +sourceType: TypeAnnotationType;
  +variance: null | Variance;
  +optional: null | 'PlusOptional' | 'MinusOptional' | 'Optional';

  +parent: ObjectTypeAnnotation;
}

export interface ObjectTypeSpreadProperty extends BaseNode {
  +type: 'ObjectTypeSpreadProperty';
  +argument: TypeAnnotationType;

  +parent: ObjectTypeAnnotation;
}

export interface IndexedAccessType extends BaseNode {
  +type: 'IndexedAccessType';
  +objectType: TypeAnnotationType;
  +indexType: TypeAnnotationType;
}
export interface OptionalIndexedAccessType extends BaseNode {
  +type: 'OptionalIndexedAccessType';
  +objectType: TypeAnnotationType;
  +indexType: TypeAnnotationType;
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
  +typeParameters: null | TypeParameterInstantiation;

  +parent: InterfaceDeclaration | DeclareInterface;
}

export interface ClassImplements extends BaseNode {
  +type: 'ClassImplements';
  +id: Identifier;
  +typeParameters: null | TypeParameterInstantiation;

  +parent: AClass | DeclareClass;
}

export interface Decorator extends BaseNode {
  +type: 'Decorator';
  +expression: Expression;

  +parent: AClass;
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
  +usesExtendsBound: boolean;
  +parent: TypeParameterDeclaration;
}
export interface TypeParameterInstantiation extends BaseNode {
  +type: 'TypeParameterInstantiation';
  +params: $ReadOnlyArray<TypeAnnotationType>;

  +parent: GenericTypeAnnotation | CallExpression | NewExpression;
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

  +parent: EnumDeclaration;
}

export interface EnumNumberMember extends BaseNode {
  +type: 'EnumNumberMember';
  +id: Identifier;
  +init: NumericLiteral;

  +parent: EnumNumberBody;
}

export interface EnumStringBody extends BaseInferrableEnumBody {
  +type: 'EnumStringBody';
  +members: $ReadOnlyArray<EnumStringMember | EnumDefaultedMember>;

  +parent: EnumDeclaration;
}

export interface EnumStringMember extends BaseNode {
  +type: 'EnumStringMember';
  +id: Identifier;
  +init: StringLiteral;

  +parent: EnumStringBody;
}

export interface EnumBooleanBody extends BaseInferrableEnumBody {
  +type: 'EnumBooleanBody';
  // enum boolean members cannot be defaulted
  +members: $ReadOnlyArray<EnumBooleanMember>;

  +parent: EnumDeclaration;
}

export interface EnumBooleanMember extends BaseNode {
  +type: 'EnumBooleanMember';
  +id: Identifier;
  +init: BooleanLiteral;

  +parent: EnumBooleanBody;
}

export interface EnumSymbolBody extends BaseEnumBody {
  +type: 'EnumSymbolBody';
  // enum symbol members can only be defaulted
  +members: $ReadOnlyArray<EnumDefaultedMember>;

  +parent: EnumDeclaration;
}

export interface EnumDefaultedMember extends BaseNode {
  +type: 'EnumDefaultedMember';
  +id: Identifier;

  +parent: EnumStringBody | EnumSymbolBody;
}

/*****************
 * Declare nodes *
 *****************/

export type DeclaredNode =
  | DeclareClass
  | DeclareComponent
  | DeclareVariable
  | DeclareEnum
  | DeclareFunction
  | DeclareModule
  | DeclareInterface
  | DeclareTypeAlias
  | DeclareOpaqueType
  | DeclareExportAllDeclaration
  | DeclareExportDeclaration
  | DeclareModuleExports
  | DeclaredPredicate;

export interface DeclareClass extends BaseNode {
  +type: 'DeclareClass';
  +id: Identifier;
  +typeParameters: null | TypeParameterDeclaration;
  +extends: $ReadOnlyArray<InterfaceExtends>;
  +implements: $ReadOnlyArray<ClassImplements>;
  +body: ObjectTypeAnnotation;
  +mixins: $ReadOnlyArray<InterfaceExtends>;
}

export interface DeclareComponent extends BaseNode {
  +type: 'DeclareComponent';
  +id: Identifier;
  +params: Array<ComponentTypeParameter>;
  +rest: null | ComponentTypeParameter;
  +typeParameters: null | TypeParameterDeclaration;
  +rendersType: null | TypeAnnotation;
}

export interface DeclareVariable extends BaseNode {
  +type: 'DeclareVariable';
  +id: Identifier;
  +kind: 'var' | 'let' | 'const';
}

export interface DeclareEnum extends BaseNode {
  +type: 'DeclareEnum';
  +id: Identifier;
  +body: EnumNumberBody | EnumStringBody | EnumBooleanBody | EnumSymbolBody;
}

export interface DeclareFunction extends BaseNode {
  +type: 'DeclareFunction';
  // the function signature is stored as a type annotation on the ID
  +id: interface extends Identifier {
    +typeAnnotation: interface extends TypeAnnotation {
      +typeAnnotation: FunctionTypeAnnotation,
    },
  };
  +predicate: InferredPredicate | DeclaredPredicate | null;
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

interface DeclareExportDeclarationBase extends BaseNode {
  +type: 'DeclareExportDeclaration';
  +specifiers: $ReadOnlyArray<ExportSpecifier>;
  +source: StringLiteral | null;
  +default: boolean;
}
export interface DeclareExportDefaultDeclaration
  extends DeclareExportDeclarationBase {
  +type: 'DeclareExportDeclaration';
  +declaration: DeclareClass | DeclareFunction | TypeAnnotationType;
  +default: true;
  // default cannot have a source
  +source: null;
  // default cannot have specifiers
  +specifiers: [];
}
export interface DeclareExportDeclarationNamedWithDeclaration
  extends DeclareExportDeclarationBase {
  +type: 'DeclareExportDeclaration';
  +declaration:
    | DeclareClass
    | DeclareFunction
    | DeclareInterface
    | DeclareOpaqueType
    | DeclareVariable
    | DeclareEnum;
  +default: false;
  +source: null;
  // default cannot have specifiers and a declaration
  +specifiers: [];
}
export interface DeclareExportDeclarationNamedWithSpecifiers
  extends DeclareExportDeclarationBase {
  +type: 'DeclareExportDeclaration';
  // with a source you can't have a declaration
  +declaration: null;
  +default: false;
  +source: StringLiteral;
  +specifiers: $ReadOnlyArray<ExportSpecifier>;
}
export type DeclareExportDeclaration =
  | DeclareExportDefaultDeclaration
  | DeclareExportDeclarationNamedWithDeclaration
  | DeclareExportDeclarationNamedWithSpecifiers;

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

  +parent: JSXOpeningElement;
}

export interface JSXClosingElement extends BaseNode {
  +type: 'JSXClosingElement';
  +name: JSXTagNameExpression;

  +parent: JSXElement;
}

export interface JSXClosingFragment extends BaseNode {
  +type: 'JSXClosingFragment';

  +parent: JSXFragment;
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
  +typeArguments?: TypeParameterInstantiation | null;

  +parent: JSXElement;
}

export interface JSXOpeningFragment extends BaseNode {
  +type: 'JSXOpeningFragment';

  +parent: JSXFragment;
}

export interface JSXSpreadAttribute extends BaseNode {
  +type: 'JSXSpreadAttribute';
  +argument: Expression;

  +parent: JSXOpeningElement;
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

export {};
