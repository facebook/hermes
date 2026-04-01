#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for binary_size_analyzer.py."""

import json
import sys
import unittest
from pathlib import Path

TOOLS_DIR = Path(__file__).parent.parent / "tools"
sys.path.insert(0, str(TOOLS_DIR))

from binary_size_analyzer import (
    classify_symbol,
    compare_symbols,
    format_human,
    format_json,
    parse_nm_output,
)


class TestParseNmOutput(unittest.TestCase):
    def test_parse_text_symbols(self):
        nm_output = (
            "0000000000401000 00000100 T _main\n"
            "0000000000401100 00000200 T _sh_ljs_get_by_val_rjs\n"
            "0000000000401300 00000050 t helper_func\n"
        )
        syms = parse_nm_output(nm_output)
        self.assertEqual(syms["_main"], 100)
        self.assertEqual(syms["_sh_ljs_get_by_val_rjs"], 200)
        self.assertEqual(syms["helper_func"], 50)

    def test_skip_non_text_symbols(self):
        nm_output = (
            "0000000000401000 00000100 T _main\n"
            "0000000000402000 00000200 D _global_data\n"
            "0000000000403000 00000300 B _bss_var\n"
            "0000000000404000 00000400 R _rodata\n"
        )
        syms = parse_nm_output(nm_output)
        self.assertEqual(len(syms), 1)
        self.assertIn("_main", syms)

    def test_empty_input(self):
        syms = parse_nm_output("")
        self.assertEqual(syms, {})

    def test_malformed_lines_skipped(self):
        nm_output = "short\n0000000000401000 00000100 T _main\nbad line here\n"
        syms = parse_nm_output(nm_output)
        self.assertEqual(len(syms), 1)
        self.assertEqual(syms["_main"], 100)

    def test_duplicate_symbols_accumulated(self):
        nm_output = (
            "0000000000401000 00000100 T _func\n0000000000402000 00000050 T _func\n"
        )
        syms = parse_nm_output(nm_output)
        self.assertEqual(syms["_func"], 150)

    def test_invalid_size_skipped(self):
        nm_output = "0000000000401000 NOTANUMBER T _main\n"
        syms = parse_nm_output(nm_output)
        self.assertEqual(syms, {})


class TestCompareSymbols(unittest.TestCase):
    def test_size_increase(self):
        syms_a = {"_func": 100}
        syms_b = {"_func": 150}
        changes = compare_symbols(syms_a, syms_b)
        self.assertEqual(len(changes), 1)
        self.assertEqual(changes[0]["delta_bytes"], 50)
        self.assertFalse(changes[0]["is_new"])
        self.assertFalse(changes[0]["is_removed"])

    def test_size_decrease(self):
        syms_a = {"_func": 200}
        syms_b = {"_func": 150}
        changes = compare_symbols(syms_a, syms_b)
        self.assertEqual(changes[0]["delta_bytes"], -50)

    def test_new_symbol(self):
        syms_a = {"_existing": 100}
        syms_b = {"_existing": 100, "_new_func": 200}
        changes = compare_symbols(syms_a, syms_b)
        new_changes = [c for c in changes if c["is_new"]]
        self.assertEqual(len(new_changes), 1)
        self.assertEqual(new_changes[0]["symbol"], "_new_func")
        self.assertEqual(new_changes[0]["delta_bytes"], 200)

    def test_removed_symbol(self):
        syms_a = {"_existing": 100, "_old_func": 150}
        syms_b = {"_existing": 100}
        changes = compare_symbols(syms_a, syms_b)
        removed = [c for c in changes if c["is_removed"]]
        self.assertEqual(len(removed), 1)
        self.assertEqual(removed[0]["symbol"], "_old_func")
        self.assertEqual(removed[0]["delta_bytes"], -150)

    def test_no_changes(self):
        syms_a = {"_func": 100}
        syms_b = {"_func": 100}
        changes = compare_symbols(syms_a, syms_b)
        self.assertEqual(len(changes), 0)

    def test_sorted_by_absolute_delta(self):
        syms_a = {"_small": 100, "_big": 100, "_medium": 100}
        syms_b = {"_small": 110, "_big": 300, "_medium": 50}
        changes = compare_symbols(syms_a, syms_b)
        self.assertEqual(changes[0]["symbol"], "_big")  # +200
        self.assertEqual(changes[1]["symbol"], "_medium")  # -50
        self.assertEqual(changes[2]["symbol"], "_small")  # +10

    def test_empty_inputs(self):
        changes = compare_symbols({}, {})
        self.assertEqual(changes, [])


class TestClassifySymbol(unittest.TestCase):
    def test_get_by_val(self):
        self.assertEqual(classify_symbol("_sh_ljs_get_by_val_rjs"), "get_by_val")

    def test_get_by_id(self):
        self.assertEqual(classify_symbol("_sh_ljs_get_by_id_rjs"), "get_by_id")

    def test_property_put(self):
        self.assertEqual(classify_symbol("_sh_ljs_put_by_id_rjs"), "property_put")
        self.assertEqual(classify_symbol("_sh_ljs_put_by_val_rjs"), "property_put")

    def test_calls(self):
        self.assertEqual(classify_symbol("_sh_ljs_call_rjs"), "calls")
        self.assertEqual(classify_symbol("_sh_ljs_construct_rjs"), "calls")

    def test_arithmetic(self):
        self.assertEqual(classify_symbol("_sh_ljs_add_rjs"), "arithmetic")
        self.assertEqual(classify_symbol("_sh_ljs_sub_rjs"), "arithmetic")
        self.assertEqual(classify_symbol("_sh_ljs_mul_rjs"), "arithmetic")

    def test_generated_js(self):
        self.assertEqual(classify_symbol("_0_global"), "generated_js")
        self.assertEqual(classify_symbol("_1_foo"), "generated_js")
        self.assertEqual(classify_symbol("_23_myFunction"), "generated_js")

    def test_runtime(self):
        self.assertEqual(classify_symbol("_sh_enter"), "runtime")
        self.assertEqual(classify_symbol("_sh_leave"), "runtime")

    def test_other(self):
        self.assertEqual(classify_symbol("main"), "other")
        self.assertEqual(classify_symbol("unknown_func"), "other")


class TestFormatHuman(unittest.TestCase):
    def test_basic_output(self):
        syms_a = {"_func1": 100, "_func2": 200}
        syms_b = {"_func1": 150, "_func2": 200}
        output = format_human(syms_a, syms_b, 1000, 1050)
        self.assertIn("Binary Size Comparison", output)
        self.assertIn(".text section", output)
        self.assertIn("_func1", output)

    def test_empty_symbols(self):
        output = format_human({}, {}, 0, 0)
        self.assertIn("No symbol size changes", output)

    def test_category_summary(self):
        syms_a = {
            "_sh_ljs_get_by_val_rjs": 100,
            "_sh_ljs_get_by_id_rjs": 100,
        }
        syms_b = {
            "_sh_ljs_get_by_val_rjs": 200,
            "_sh_ljs_get_by_id_rjs": 100,
        }
        output = format_human(syms_a, syms_b, 500, 600)
        self.assertIn("Category Summary", output)
        self.assertIn("get_by_val", output)

    def test_new_symbol_tagged(self):
        syms_a = {"_old": 100}
        syms_b = {"_old": 100, "_new": 200}
        output = format_human(syms_a, syms_b, 100, 300)
        self.assertIn("[NEW]", output)

    def test_removed_symbol_tagged(self):
        syms_a = {"_old": 100, "_removed": 200}
        syms_b = {"_old": 100}
        output = format_human(syms_a, syms_b, 300, 100)
        self.assertIn("[REMOVED]", output)


class TestFormatJson(unittest.TestCase):
    def test_basic_structure(self):
        syms_a = {"_func": 100}
        syms_b = {"_func": 150}
        result = format_json(syms_a, syms_b, 1000, 1050, "a.bin", "b.bin")
        self.assertEqual(result["baseline"], "a.bin")
        self.assertEqual(result["optimized"], "b.bin")
        self.assertIn("text_section", result)
        self.assertIn("symbol_total", result)
        self.assertIn("categories", result)
        self.assertIn("top_changes", result)

    def test_text_section_delta(self):
        result = format_json({}, {}, 1000, 1200)
        self.assertEqual(result["text_section"]["delta_bytes"], 200)
        self.assertEqual(result["text_section"]["delta_pct"], 20.0)

    def test_zero_baseline_no_division_error(self):
        result = format_json({}, {}, 0, 100)
        self.assertEqual(result["text_section"]["delta_pct"], 0)
        self.assertEqual(result["symbol_total"]["delta_pct"], 0)

    def test_categories_populated(self):
        syms_a = {"_sh_ljs_get_by_val_rjs": 100}
        syms_b = {"_sh_ljs_get_by_val_rjs": 200}
        result = format_json(syms_a, syms_b, 0, 0)
        self.assertIn("get_by_val", result["categories"])
        self.assertEqual(result["categories"]["get_by_val"]["delta_bytes"], 100)

    def test_json_serializable(self):
        syms_a = {"_func": 100}
        syms_b = {"_func": 150, "_new": 50}
        result = format_json(syms_a, syms_b, 1000, 1100)
        # Should not raise
        json.dumps(result)

    def test_top_changes_limited(self):
        syms_a = {f"_func_{i}": 100 for i in range(50)}
        syms_b = {f"_func_{i}": 100 + i for i in range(50)}
        result = format_json(syms_a, syms_b, 0, 0)
        self.assertLessEqual(len(result["top_changes"]), 30)
        self.assertEqual(result["total_changed_symbols"], 49)  # _func_0 unchanged


if __name__ == "__main__":
    unittest.main()
