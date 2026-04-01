#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for type_coverage_analyzer.py"""

import sys
import unittest
from pathlib import Path

# Add tools directory to path to import the module
tools_dir = Path(__file__).parent.parent / "tools"
sys.path.insert(0, str(tools_dir))

from type_coverage_analyzer import compare, extract_runtime_calls


class TestExtractRuntimeCalls(unittest.TestCase):
    """Test extraction of runtime calls from generated C code."""

    def test_empty_source(self):
        result = extract_runtime_calls("")
        self.assertEqual(len(result["global_calls"]), 0)
        self.assertEqual(len(result["functions"]), 0)
        self.assertEqual(len(result["function_lines"]), 0)

    def test_count_calls_globally(self):
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 2);
    return _sh_ljs_add(shr, 1, 2);
}
"""
        result = extract_runtime_calls(source)
        self.assertEqual(result["global_calls"]["_sh_ljs_get_by_id_rjs"], 2)
        self.assertEqual(result["global_calls"]["_sh_ljs_add"], 1)
        self.assertEqual(sum(result["global_calls"].values()), 3)

    def test_count_calls_per_function(self):
        source = """
static SHLegacyValue _0_func1(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_add(shr, 1, 2);
}

static SHLegacyValue _1_func2(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_mul(shr, 3, 4);
    return _sh_ljs_div(shr, 5, 6);
}
"""
        result = extract_runtime_calls(source)
        self.assertEqual(len(result["functions"]), 2)
        self.assertIn("_0_func1", result["functions"])
        self.assertIn("_1_func2", result["functions"])
        self.assertEqual(result["functions"]["_0_func1"]["_sh_ljs_add"], 1)
        self.assertEqual(result["functions"]["_1_func2"]["_sh_ljs_mul"], 1)
        self.assertEqual(result["functions"]["_1_func2"]["_sh_ljs_div"], 1)

    def test_track_function_line_counts(self):
        source = """
static SHLegacyValue _0_short(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_add(shr, 1, 2);
}

static SHLegacyValue _1_longer(SHRuntime *shr, SHLegacyValue *frame) {
    int a = 1;
    int b = 2;
    int c = 3;
    return _sh_ljs_add(shr, a, b);
}
"""
        result = extract_runtime_calls(source)
        self.assertIn("_0_short", result["function_lines"])
        self.assertIn("_1_longer", result["function_lines"])
        self.assertLess(
            result["function_lines"]["_0_short"], result["function_lines"]["_1_longer"]
        )

    def test_multiple_calls_same_line(self):
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_add(shr, _sh_ljs_mul(shr, 1, 2), _sh_ljs_div(shr, 3, 4));
}
"""
        result = extract_runtime_calls(source)
        # Should count all three calls
        self.assertEqual(sum(result["global_calls"].values()), 3)


class TestCompare(unittest.TestCase):
    """Test comparison of typed vs untyped generated code."""

    def test_calls_eliminated(self):
        untyped_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_typeof(shr, frame[0]);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    return _sh_ljs_add(shr, 1, 2);
}
"""
        typed_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    // Typed mode eliminated typeof check
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    return _sh_ljs_add(shr, 1, 2);
}
"""
        untyped_data = extract_runtime_calls(untyped_source)
        typed_data = extract_runtime_calls(typed_source)
        report = compare(typed_data, untyped_data)

        # _sh_ljs_typeof should be in eliminated_calls
        self.assertIn("_sh_ljs_typeof", report["eliminated_calls"])
        self.assertEqual(report["summary"]["unique_calls_eliminated"], 1)
        self.assertEqual(report["summary"]["calls_eliminated"], 1)

    def test_calls_reduced(self):
        untyped_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 2);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 3);
    return frame[0];
}
"""
        typed_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    return frame[0];
}
"""
        untyped_data = extract_runtime_calls(untyped_source)
        typed_data = extract_runtime_calls(typed_source)
        report = compare(typed_data, untyped_data)

        # Should show reduction in _sh_ljs_get_by_id_rjs
        reduced = [
            r for r in report["reduced_calls"] if r["call"] == "_sh_ljs_get_by_id_rjs"
        ]
        self.assertEqual(len(reduced), 1)
        self.assertEqual(reduced[0]["untyped"], 3)
        self.assertEqual(reduced[0]["typed"], 1)
        # (1 - 1/3) * 100 ≈ 66.7%
        self.assertAlmostEqual(reduced[0]["reduction_pct"], 66.7, places=1)

    def test_calls_increased(self):
        # Typed mode can add checks in some cases
        untyped_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_add(shr, 1, 2);
}
"""
        typed_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_to_number(shr, frame[0]);
    return _sh_ljs_add(shr, 1, 2);
}
"""
        untyped_data = extract_runtime_calls(untyped_source)
        typed_data = extract_runtime_calls(typed_source)
        report = compare(typed_data, untyped_data)

        # _sh_to_number should be in increased_calls
        increased = [
            r for r in report["increased_calls"] if r["call"] == "_sh_to_number"
        ]
        self.assertEqual(len(increased), 1)
        self.assertEqual(increased[0]["typed"], 1)
        self.assertEqual(increased[0]["untyped"], 0)

    def test_overall_reduction_pct(self):
        untyped_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_typeof(shr, frame[0]);
    _sh_ljs_typeof(shr, frame[1]);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    _sh_ljs_get_by_id_rjs(shr, frame[0], 2);
    return _sh_ljs_add(shr, 1, 2);
}
"""
        typed_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_get_by_id_rjs(shr, frame[0], 1);
    return _sh_ljs_add(shr, 1, 2);
}
"""
        untyped_data = extract_runtime_calls(untyped_source)
        typed_data = extract_runtime_calls(typed_source)
        report = compare(typed_data, untyped_data)

        # Untyped: 5 calls, Typed: 2 calls
        # Reduction = (1 - 2/5) * 100 = 60%
        self.assertEqual(report["summary"]["untyped_total_calls"], 5)
        self.assertEqual(report["summary"]["typed_total_calls"], 2)
        self.assertEqual(report["summary"]["calls_eliminated"], 3)
        self.assertEqual(report["summary"]["overall_reduction_pct"], 60.0)

    def test_per_function_coverage_pct(self):
        untyped_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_typeof(shr, frame[0]);
    _sh_ljs_typeof(shr, frame[1]);
    _sh_ljs_typeof(shr, frame[2]);
    _sh_ljs_typeof(shr, frame[3]);
    return frame[0];
}
"""
        typed_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_typeof(shr, frame[0]);
    return frame[0];
}
"""
        untyped_data = extract_runtime_calls(untyped_source)
        typed_data = extract_runtime_calls(typed_source)
        report = compare(typed_data, untyped_data)

        # Find the function in per_function
        func_report = [f for f in report["per_function"] if f["function"] == "_0_func"][
            0
        ]
        self.assertEqual(func_report["untyped_calls"], 4)
        self.assertEqual(func_report["typed_calls"], 1)
        self.assertEqual(func_report["calls_eliminated"], 3)
        # (1 - 1/4) * 100 = 75%
        self.assertEqual(func_report["coverage_pct"], 75.0)

    def test_summary_totals(self):
        untyped_source = """
static SHLegacyValue _0_func1(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_typeof(shr, frame[0]);
    return _sh_ljs_add(shr, 1, 2);
}

static SHLegacyValue _1_func2(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_typeof(shr, frame[0]);
    _sh_ljs_typeof(shr, frame[1]);
    return _sh_ljs_mul(shr, 3, 4);
}
"""
        typed_source = """
static SHLegacyValue _0_func1(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_add(shr, 1, 2);
}

static SHLegacyValue _1_func2(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_mul(shr, 3, 4);
}
"""
        untyped_data = extract_runtime_calls(untyped_source)
        typed_data = extract_runtime_calls(typed_source)
        report = compare(typed_data, untyped_data)

        # Untyped: 5 calls (3 typeof, 1 add, 1 mul)
        # Typed: 2 calls (1 add, 1 mul)
        self.assertEqual(report["summary"]["untyped_total_calls"], 5)
        self.assertEqual(report["summary"]["typed_total_calls"], 2)
        self.assertEqual(report["summary"]["calls_eliminated"], 3)

    def test_identical_sources_zero_reduction(self):
        source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    return _sh_ljs_add(shr, 1, 2);
}
"""
        data1 = extract_runtime_calls(source)
        data2 = extract_runtime_calls(source)
        report = compare(data1, data2)

        self.assertEqual(report["summary"]["overall_reduction_pct"], 0.0)
        self.assertEqual(report["summary"]["calls_eliminated"], 0)
        self.assertEqual(len(report["eliminated_calls"]), 0)
        self.assertEqual(len(report["reduced_calls"]), 0)

    def test_typed_zero_calls_hundred_pct_reduction(self):
        untyped_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    _sh_ljs_typeof(shr, frame[0]);
    _sh_ljs_typeof(shr, frame[1]);
    return frame[0];
}
"""
        typed_source = """
static SHLegacyValue _0_func(SHRuntime *shr, SHLegacyValue *frame) {
    // All calls eliminated
    return frame[0];
}
"""
        untyped_data = extract_runtime_calls(untyped_source)
        typed_data = extract_runtime_calls(typed_source)
        report = compare(typed_data, untyped_data)

        self.assertEqual(report["summary"]["typed_total_calls"], 0)
        self.assertEqual(report["summary"]["untyped_total_calls"], 2)
        self.assertEqual(report["summary"]["overall_reduction_pct"], 100.0)

    def test_empty_sources(self):
        data1 = extract_runtime_calls("")
        data2 = extract_runtime_calls("")
        report = compare(data1, data2)

        self.assertEqual(report["summary"]["typed_total_calls"], 0)
        self.assertEqual(report["summary"]["untyped_total_calls"], 0)
        self.assertEqual(report["summary"]["calls_eliminated"], 0)
        self.assertEqual(report["summary"]["overall_reduction_pct"], 0.0)


if __name__ == "__main__":
    unittest.main()
