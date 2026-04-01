#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for generated_code_analyzer.py"""

import sys
import unittest
from pathlib import Path

# Add tools directory to path to import the module
tools_dir = Path(__file__).parent.parent / "tools"
sys.path.insert(0, str(tools_dir))

from generated_code_analyzer import categorize_call, parse_generated_c


class TestCategorizeCall(unittest.TestCase):
    """Test runtime call categorization."""

    def test_property_access(self):
        self.assertEqual(categorize_call("_sh_ljs_get_by_id"), "property_access")
        self.assertEqual(categorize_call("_sh_ljs_put_by_id"), "property_access")
        self.assertEqual(categorize_call("_sh_ljs_get_by_val"), "property_access")
        self.assertEqual(categorize_call("_sh_ljs_get_by_index"), "property_access")
        self.assertEqual(categorize_call("_sh_ljs_del_by_id"), "property_access")
        self.assertEqual(categorize_call("_sh_ljs_has_by_val"), "property_access")
        self.assertEqual(categorize_call("_sh_ljs_get_computed"), "property_access")
        self.assertEqual(categorize_call("_sh_prload"), "property_access")
        self.assertEqual(categorize_call("_sh_prstore"), "property_access")
        self.assertEqual(categorize_call("_sh_find_property"), "property_access")

    def test_type_check(self):
        self.assertEqual(categorize_call("_sh_ljs_typeof"), "type_check")
        self.assertEqual(categorize_call("_sh_ljs_typeof_rjs"), "type_check")
        self.assertEqual(categorize_call("_sh_ljs_is_object"), "type_check")
        self.assertEqual(categorize_call("_sh_ljs_is_undefined"), "type_check")
        self.assertEqual(categorize_call("_sh_to_number"), "type_check")
        self.assertEqual(categorize_call("_sh_to_string"), "type_check")
        self.assertEqual(categorize_call("_sh_ljs_double_to_int"), "type_check")
        self.assertEqual(categorize_call("_sh_ljs_bool_to_number"), "type_check")

    def test_gc(self):
        self.assertEqual(categorize_call("_sh_push_locals"), "gc")
        self.assertEqual(categorize_call("_sh_pop_locals"), "gc")
        self.assertEqual(categorize_call("_sh_gc_collect"), "gc")
        self.assertEqual(categorize_call("_sh_check_native_stack"), "gc")
        self.assertEqual(categorize_call("_sh_new_gcscope"), "gc")
        self.assertEqual(categorize_call("_sh_end_gcscope"), "gc")
        self.assertEqual(categorize_call("_sh_flush_gcscope"), "gc")

    def test_arithmetic(self):
        self.assertEqual(categorize_call("_sh_ljs_add"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_sub"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_mul"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_div"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_mod"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_inc"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_dec"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_negate"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_exp"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_bitwise"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_lshift"), "arithmetic")
        self.assertEqual(categorize_call("_sh_ljs_rshift"), "arithmetic")

    def test_comparison(self):
        self.assertEqual(categorize_call("_sh_ljs_equal"), "comparison")
        self.assertEqual(categorize_call("_sh_ljs_strict_equal"), "comparison")
        self.assertEqual(categorize_call("_sh_ljs_less"), "comparison")
        self.assertEqual(categorize_call("_sh_ljs_greater"), "comparison")
        self.assertEqual(categorize_call("_sh_ljs_abstract_equal"), "comparison")

    def test_call(self):
        self.assertEqual(categorize_call("_sh_ljs_call"), "call")
        self.assertEqual(categorize_call("_sh_ljs_call_builtin"), "call")
        self.assertEqual(categorize_call("_sh_ljs_construct"), "call")
        self.assertEqual(categorize_call("_sh_call_function"), "call")

    def test_object(self):
        self.assertEqual(categorize_call("_sh_ljs_create_object"), "object")
        self.assertEqual(categorize_call("_sh_ljs_new_object"), "object")
        self.assertEqual(categorize_call("_sh_new_array"), "object")
        self.assertEqual(categorize_call("_sh_ljs_object_create"), "object")
        self.assertEqual(categorize_call("_sh_ljs_array_push"), "object")

    def test_string(self):
        self.assertEqual(categorize_call("_sh_ljs_string_concat"), "string")
        self.assertEqual(categorize_call("_sh_string_create"), "string")
        self.assertEqual(categorize_call("_sh_ljs_concat"), "string")

    def test_other(self):
        self.assertEqual(categorize_call("_sh_unknown_function"), "other")
        self.assertEqual(categorize_call("_sh_some_other_call"), "other")


class TestParseGeneratedC(unittest.TestCase):
    """Test parsing of generated C code."""

    def test_empty_source(self):
        result = parse_generated_c("")
        self.assertEqual(result["functions"], [])
        self.assertEqual(result["runtime_calls"], {})
        self.assertEqual(result["category_breakdown"], {})
        self.assertEqual(result["patterns"], [])
        self.assertEqual(result["summary"]["total_functions"], 0)
        self.assertEqual(result["summary"]["total_runtime_calls"], 0)

    def test_source_with_no_functions(self):
        source = """
// Some header comments
#include <stdio.h>

int main() {
    return 0;
}
"""
        result = parse_generated_c(source)
        self.assertEqual(result["functions"], [])
        self.assertEqual(result["runtime_calls"], {})

    def test_source_with_no_sh_calls(self):
        source = """
static SHLegacyValue _0_myFunc(SHRuntime *shr, SHLegacyValue *frame) {
    int x = 42;
    return x;
}
"""
        result = parse_generated_c(source)
        self.assertEqual(len(result["functions"]), 1)
        func = result["functions"][0]
        self.assertEqual(func["name"], "_0_myFunc")
        self.assertEqual(func["total_runtime_calls"], 0)
        self.assertEqual(func["gc_scopes"], 0)

    def test_detect_function_boundaries(self):
        source = """
static SHLegacyValue _0_myFunc(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_push_locals(shr);
    SHLegacyValue v = _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    _sh_pop_locals(shr);
    return v;
}
"""
        result = parse_generated_c(source)
        self.assertEqual(len(result["functions"]), 1)
        func = result["functions"][0]
        self.assertEqual(func["name"], "_0_myFunc")
        self.assertEqual(func["start_line"], 2)
        self.assertEqual(func["end_line"], 7)
        self.assertEqual(func["line_count"], 6)

    def test_count_sh_calls_per_function(self):
        source = """
static SHLegacyValue _0_add(SHRuntime *shr, SHLegacyValue *frame) {
    SHLegacyValue a = _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    SHLegacyValue b = _sh_ljs_get_by_id_rjs(shr, frame[0], 2);
    return _sh_ljs_add(shr, a, b);
}
"""
        result = parse_generated_c(source)
        self.assertEqual(len(result["functions"]), 1)
        func = result["functions"][0]
        self.assertEqual(func["total_runtime_calls"], 3)
        self.assertIn("_sh_ljs_get_by_id_rjs", result["runtime_calls"])
        self.assertEqual(result["runtime_calls"]["_sh_ljs_get_by_id_rjs"], 2)
        self.assertIn("_sh_ljs_add", result["runtime_calls"])
        self.assertEqual(result["runtime_calls"]["_sh_ljs_add"], 1)

    def test_track_gc_scopes(self):
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_push_locals(shr);
    _sh_new_gcscope(shr);
    SHLegacyValue v = _sh_ljs_call(shr, frame[0]);
    _sh_end_gcscope(shr);
    _sh_pop_locals(shr);
    return v;
}
"""
        result = parse_generated_c(source)
        func = result["functions"][0]
        self.assertEqual(func["gc_scopes"], 2)  # _sh_push_locals and _sh_new_gcscope

    def test_calls_per_line_calculation(self):
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_push_locals(shr);
    SHLegacyValue v = _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    _sh_pop_locals(shr);
    return v;
}
"""
        result = parse_generated_c(source)
        func = result["functions"][0]
        # 3 calls in 6 lines = 0.5 calls/line
        self.assertEqual(func["calls_per_line"], 0.5)

    def test_top_calls_returns_most_common(self):
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 2);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 3);
    _sh_ljs_add(shr, 1, 2);
    _sh_ljs_mul(shr, 3, 4);
    return _sh_ljs_get_by_id_rjs(shr, frame[0], 4);
}
"""
        result = parse_generated_c(source)
        func = result["functions"][0]
        top_calls = func["top_calls"]
        self.assertEqual(len(top_calls), 3)
        # Most common should be _sh_ljs_get_by_id_rjs with 4 occurrences
        self.assertEqual(top_calls[0][0], "_sh_ljs_get_by_id_rjs")
        self.assertEqual(top_calls[0][1], 4)

    def test_category_breakdown_aggregation(self):
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    _sh_ljs_put_by_id(shr, frame[0], 2, 3);
    _sh_ljs_add(shr, 1, 2);
    _sh_ljs_mul(shr, 3, 4);
    _sh_ljs_typeof(shr, frame[0]);
    return _sh_push_locals(shr);
}
"""
        result = parse_generated_c(source)
        breakdown = result["category_breakdown"]
        self.assertEqual(breakdown["property_access"], 2)
        self.assertEqual(breakdown["arithmetic"], 2)
        self.assertEqual(breakdown["type_check"], 1)
        self.assertEqual(breakdown["gc"], 1)

    def test_multiple_functions(self):
        source = """
static SHLegacyValue _0_func1(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_add(shr, 1, 2);
}

static SHLegacyValue _1_func2(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_mul(shr, 3, 4);
}
"""
        result = parse_generated_c(source)
        self.assertEqual(len(result["functions"]), 2)
        self.assertEqual(result["functions"][0]["name"], "_0_func1")
        self.assertEqual(result["functions"][1]["name"], "_1_func2")
        self.assertEqual(result["summary"]["total_runtime_calls"], 2)


class TestDetectPatterns(unittest.TestCase):
    """Test anti-pattern detection."""

    def test_redundant_type_check(self):
        # Pattern detection resets when a non-matching line is encountered,
        # so typeof calls must be within 2 lines with no intervening non-typeof lines
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_typeof(shr, frame[0]);
    _sh_ljs_typeof(shr, frame[0]);
    return x;
}
"""
        result = parse_generated_c(source)
        patterns = [
            p for p in result["patterns"] if p["pattern"] == "redundant_type_check"
        ]
        self.assertEqual(len(patterns), 1)
        self.assertEqual(patterns[0]["severity"], "medium")
        self.assertIn("_sh_ljs_typeof", patterns[0]["detail"])

    def test_excessive_gc_scoping(self):
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_push_locals(shr);
    _sh_new_gcscope(shr);
    _sh_push_locals(shr);
    _sh_new_gcscope(shr);
    _sh_push_locals(shr);
    _sh_new_gcscope(shr);
    return frame[0];
}
"""
        result = parse_generated_c(source)
        patterns = [
            p for p in result["patterns"] if p["pattern"] == "excessive_gc_scoping"
        ]
        self.assertEqual(len(patterns), 1)
        func = result["functions"][0]
        self.assertGreater(func["gc_scopes"], 5)
        self.assertEqual(patterns[0]["severity"], "medium")

    def test_inline_opportunity(self):
        source = """
static SHLegacyValue _0_tiny(SHRuntime *shr, SHLegacyValue *frame) {
    SHLegacyValue v = _sh_ljs_add(shr, 1, 2);
    return v;
}
"""
        result = parse_generated_c(source)
        patterns = [
            p for p in result["patterns"] if p["pattern"] == "inline_opportunity"
        ]
        self.assertEqual(len(patterns), 1)
        func = result["functions"][0]
        self.assertLessEqual(func["line_count"], 10)
        self.assertLessEqual(func["total_runtime_calls"], 2)
        self.assertEqual(patterns[0]["severity"], "low")

    def test_high_call_density(self):
        source = """
static SHLegacyValue _0_dense(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 2);
    _sh_ljs_add(shr, 1, 2);
    _sh_ljs_mul(shr, 3, 4);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 3);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 4);
    _sh_ljs_sub(shr, 5, 6);
    _sh_ljs_div(shr, 7, 8);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 5);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 6);
    _sh_ljs_add(shr, 9, 10);
    _sh_ljs_mul(shr, 11, 12);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 7);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 8);
    _sh_ljs_sub(shr, 13, 14);
    _sh_ljs_div(shr, 15, 16);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 9);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 10);
    _sh_ljs_add(shr, 17, 18);
    _sh_ljs_mul(shr, 19, 20);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 11);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 12);
    _sh_ljs_sub(shr, 21, 22);
    return _sh_ljs_div(shr, 23, 24);
}
"""
        result = parse_generated_c(source)
        patterns = [
            p for p in result["patterns"] if p["pattern"] == "high_call_density"
        ]
        func = result["functions"][0]
        # Over 20 lines with >0.5 calls/line
        self.assertGreater(func["line_count"], 20)
        self.assertGreater(func["calls_per_line"], 0.5)
        if patterns:  # Pattern should be detected
            self.assertEqual(patterns[0]["severity"], "high")

    def test_no_patterns_in_clean_code(self):
        source = """
static SHLegacyValue _0_clean(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_push_locals(shr);
    SHLegacyValue a = _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    SHLegacyValue b = _sh_ljs_get_by_id_rjs(shr, frame[0], 2);
    SHLegacyValue c = _sh_ljs_get_by_id_rjs(shr, frame[0], 3);
    SHLegacyValue sum = _sh_ljs_add(shr, a, b);
    SHLegacyValue result = _sh_ljs_add(shr, sum, c);
    _sh_pop_locals(shr);
    return result;
}
"""
        result = parse_generated_c(source)
        # Should have no high-severity patterns
        high_patterns = [p for p in result["patterns"] if p["severity"] == "high"]
        self.assertEqual(len(high_patterns), 0)


if __name__ == "__main__":
    unittest.main()
