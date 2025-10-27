# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import json
from typing import no_type_check, Tuple

from .typing_defs import JSON
from .utils import TestCaseResult, TestResultCode

# Collect the keys that should be omitted from either Hermes or esprima ASTs.
# Key is the type of a node, and value is the set of keys of child nodes that
# need to be omitted.
# fmt: off
OMITTED_KEYS = {
    # Keys in Hermes
    "ArrayExpression": {"trailingComma"},
    "BlockStatement": {"implicit"},
    # Some literals support "raw" and others don't.
    # esprima doesn't distinguish.
    "EnumStringBody": {"hasUnknownMembers"},
    "EnumNumberBody": {"hasUnknownMembers"},
    "EnumBooleanBody": {"hasUnknownMembers"},
    "EnumSymbolBody": {"hasUnknownMembers"},

    # Common keys in both Hermes and esprima
    "Literal": {"raw"},
    "ImportDeclaration": {
        # Hermes
        "assertions",
        # esprima
        "importKind",
        "attributes",
    },
    "ImportSpecifier": {"importKind"},
    "ExportNamedDeclaration": {"exportKind"},
    "ExportAllDeclaration": {"exportKind"},
    # TODO: Flow conditionally outputs "inexact" depending on whether the parent
    # node is an Interface or not.
    "ObjectTypeAnnotation": {"inexact"},
    "ForOfStatement": {"await"},

    # Keys in esprima
    "Program": {"tokens", "sourceType", "comments"},
    "BigIntLiteral": {"value"},
    "BigIntLiteralTypeAnnotation": {"value"},
    # ES6+ specific enhancement to the ESTree original definitions that Hermes
    # does not support yet.
    # TODO: remember to update or remove them once we update the parser.
    "FunctionDeclaration": {"expression", "predicate"},
    "FunctionExpression": {"expression", "predicate"},
    "ArrowFunctionExpression": {"generator"},
    "ForInStatement": {"each"},
    "Identifier": {"optional"},
    "MethodDefinition": {"decorators"},
    "ExportDefaultDeclaration": {"exportKind"},
}
# fmt: on

# These are the keys in the JSON ASTs that should be omitted when trimming.
OMITTED_KEYS_COMMON = {"loc", "range", "errors"}  # These are all from esprima


def trim_ast(node: JSON) -> JSON:
    """Further trim the AST after normalization."""
    if isinstance(node, dict):
        tree = node
        res = {}
        for key, val in tree.items():
            if key in OMITTED_KEYS_COMMON:
                continue
            if (
                "type" in tree
                and tree["type"] in OMITTED_KEYS
                and key in OMITTED_KEYS[tree["type"]]
            ):
                continue
            else:
                res[key] = trim_ast(val)
        return res
    elif isinstance(node, list):
        return [trim_ast(elt) for elt in node]
    else:
        return node


# We could make typing this function work, but it needs a few wrapper functions
# and assertions. For now, let's simply disable typing it.
@no_type_check
def normalize_hermes_ast(ast: JSON) -> JSON:
    """
    Recursively normalize the Hermes AST for diff checking. Normalizations
    include deleting nodes that do not exist in esprima, updating node values
    to match esprima and skipping some nodes. Normalized AST is in a form
    between Hermes AST and esprima AST (we will do normalizations on esprima
    AST as well).
    """

    HERMES_LITERAL_NODE_TYPES = {
        "NullLiteral",
        "BooleanLiteral",
        "StringLiteral",
        "NumericLiteral",
        "BigIntLiteral",
        "RegExpLiteral",
        "JSXStringLiteral",
    }

    if isinstance(ast, list):
        return [normalize_hermes_ast(elt) for elt in ast]
    if not isinstance(ast, dict):
        return ast

    if "type" not in ast:
        return ast
    if ast["type"] == "Empty":
        return None
    # skip top level 'File' node
    if ast["type"] == "File":
        ast = ast["program"]
        # We don't really need this recursive call, this is just to make
        # type checker happy.
        return normalize_hermes_ast(ast)
    if ast["type"] == "ExpressionStatement":
        # esprima asts don't have the 'directive' field if it's null
        if (
            isinstance(ast["expression"], dict)
            and ast["expression"]["type"] == "StringLiteral"
            and ast["expression"]["value"] == "use strict"
        ):
            ast["directive"] = "use strict"
        else:
            del ast["directive"]
    if ast["type"] == "Identifier" and ast["name"] == "this":
        del ast["optional"]
    if ast["type"] == "DeclareNamespace":
        del ast["id"]["optional"]
    if ast["type"] == "PrivateName":
        ast = ast["id"]
        ast["type"] = "PrivateIdentifier"
    if ast["type"] == "ClassPrivateProperty":
        ast["key"]["type"] = "PrivateIdentifier"
        ast["computed"] = False
        ast["static"] = False
    if ast["type"] == "ClassProperty" or ast["type"] == "ClassPrivateProperty":
        if not ast["optional"]:
            del ast["optional"]
        ast["type"] = "PropertyDefinition"
    if ast["type"] == "TupleTypeAnnotation":
        ast["elementTypes"] = ast["types"]
        del ast["types"]
    if ast["type"] == "TypeParameter":
        if not ast["usesExtendsBound"]:
            del ast["usesExtendsBound"]
    if ast["type"] == "TupleTypeLabeledElement":
        if not ast["optional"]:
            del ast["optional"]
    # convert the literal node types to ESTree standard form
    if ast["type"] in HERMES_LITERAL_NODE_TYPES:
        if ast["type"] == "NullLiteral":
            ast["value"] = None
        if ast["type"] == "RegExpLiteral":
            ast["regex"] = {"pattern": ast["pattern"], "flags": ast["flags"]}
            del ast["pattern"]
            del ast["flags"]
        if ast["type"] == "BigIntLiteral":
            ast["bigint"] = ast["bigint"]
            ast["value"] = None
        ast["type"] = "Literal"

    # Normalize children node
    for key, val in ast.items():
        ast[key] = normalize_hermes_ast(val)
    return ast


@no_type_check
def normalize_esprima_ast(ast: JSON) -> JSON:
    """
    Recursively normalize the full esprima AST for diff checking.
    Normalizations include deleting some nodes that do not exist in Hermes AST
    and updating node values to match Hermes AST. Normalized AST is in a form
    between Hermes AST and esprima AST.
    """

    if isinstance(ast, list):
        return [normalize_esprima_ast(elt) for elt in ast]
    if not isinstance(ast, dict):
        return ast

    if "type" in ast and ast["type"] == "ExpressionStatement":
        # If the expression is a string, esprima will set the string to
        # the 'directive' field. Even if it's not 'use strict'
        if "directive" in ast and ast["directive"] != "use strict":
            del ast["directive"]
    if "type" in ast:
        # If it is a regexp literal, the 'value' field is unnecessary and
        # Hermes does not have it.
        if ast["type"] == "Literal" and "regex" in ast:
            del ast["value"]

        # Flow outputs RestProperty/SpreadProperty sometimes.
        if ast["type"] == "RestProperty":
            ast["type"] = "RestElement"
        if ast["type"] == "SpreadProperty":
            ast["type"] = "SpreadElement"
        if (
            ast["type"] == "ClassProperty"
            or ast["type"] == "ClassPrivateProperty"
            or ast["type"] == "PropertyDefinition"
        ):
            if "declare" not in ast:
                ast["declare"] = False

        # Hermes doesn't have the 'id' field in ArrowFunctionExpression.
        if ast["type"] == "ArrowFunctionExpression":
            del ast["id"]
    # If it is a template literal, the 'value' field contains
    # the 'cooked' and 'raw' strings, which should be moved.
    if "type" in ast and ast["type"] == "TemplateLiteral" and "quasis" in ast:
        for quasi in ast["quasis"]:
            quasi["cooked"] = quasi["value"]["cooked"]
            quasi["raw"] = quasi["value"]["raw"]
            del quasi["value"]

    for key, val in ast.items():
        ast[key] = normalize_esprima_ast(val)
    return ast


def compare_ast(hermes_n: JSON, esprima_n: JSON) -> Tuple[bool, str]:
    """
    Compare the two (normalized) AST.

    Returns:
    (True, "") if they are matched.
    (False, message) if there is a mismatch. Here message is the details.
    """
    if type(hermes_n) != type(esprima_n):  # noqa: E721
        msg = f"Expected {type(esprima_n)}, found {type(hermes_n)}"
        if isinstance(esprima_n, dict):
            msg += f"\n  in {esprima_n['type']}"
        elif isinstance(hermes_n, dict):
            msg += f"\n  in Hermes {hermes_n['type']}"
        return (False, msg)
    if isinstance(hermes_n, dict):
        # Just to make type checking happy
        assert isinstance(esprima_n, dict)
        # Check if Hermes is missing keys.
        for key, val in esprima_n.items():
            if key not in hermes_n:
                msg = f"{hermes_n['type']} missing property: {key}"
                return (False, msg)
            (eq, msg) = compare_ast(hermes_n[key], val)
            if not eq:
                return (False, msg)
        # Check if Hermes has extra keys.
        for key, val in hermes_n.items():
            if key not in esprima_n and val is not None:
                # Flow omits some keys which are null in Hermes output.
                # Ignore these keys in Hermes if they don't show up in
                # the expected AST if they have no value anyway.
                msg = f"{hermes_n['type']} extra property: {key}"
                return (False, msg)
        return (True, "")
    elif isinstance(hermes_n, list):
        # Just to make type checking happy
        assert isinstance(esprima_n, list)
        if len(hermes_n) != len(esprima_n):
            msg = f"List expected {len(esprima_n)} elements, found {len(hermes_n)}"
            return (False, msg)
        for i in range(len(hermes_n)):
            (eq, msg) = compare_ast(hermes_n[i], esprima_n[i])
            if not eq:
                return (False, msg)
        return (True, "")
    else:
        if hermes_n != esprima_n:
            msg = f"Expected {esprima_n}, found {hermes_n}"
            return (False, msg)
        return (True, "")


def diff(output: str, test_name: str, expected_ast: JSON) -> TestCaseResult:
    """
    Diff check the AST provided in output string against expected AST.

    Returns:
    True, if the two ASTs match (with normalization).
    False, if there is a mismatch, or other errors such as JSON decoder error.
    """
    try:
        hermes_ast = json.loads(output)
    except json.JSONDecodeError as err:
        msg = "FAIL: fail to get AST"
        details = f"JSON decode error: {err}\n"
        details += f"Raw:\n{output}"
        return TestCaseResult(test_name, TestResultCode.TEST_FAILED, msg, details)

    # We always normalize and trim the full AST for simplicity.
    # Most ASTs in esprima/flow tests are very small, performance is not a
    # concern. Even it is, most expensive operations happen in normalization
    # (e.g., del), and those have to be done when comparing ASTs.
    hermes_ast = trim_ast(normalize_hermes_ast(hermes_ast))
    expected_ast = trim_ast(normalize_esprima_ast(expected_ast))
    (test_passed, details) = compare_ast(hermes_ast, expected_ast)
    if not test_passed:
        msg = "FAIL: AST not expected"
        details = f"Reason: {details}\n"
        details += f"Hermes AST: \n{json.dumps(hermes_ast, indent=2)}\n"
        details += f"ESPRIMA AST: \n{json.dumps(expected_ast, indent=2)}"
        return TestCaseResult(test_name, TestResultCode.TEST_FAILED, msg, details)
    return TestCaseResult(test_name, TestResultCode.TEST_PASSED)
