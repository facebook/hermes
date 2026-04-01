#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for js_antipattern_scan.py"""

import os
import sys
import tempfile
import unittest

# Add tools directory to path to import the module
TOOLS_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "tools")
sys.path.insert(0, TOOLS_DIR)

from js_antipattern_scan import collect_js_files, format_human, scan_js_file


class TestJsAntipatternScan(unittest.TestCase):
    def test_eval_usage_detected(self):
        """Test detection of eval() usage."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("eval('console.log(1)');\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            eval_findings = [f for f in findings if f["pattern"] == "eval_usage"]
            self.assertEqual(len(eval_findings), 1)
            self.assertEqual(eval_findings[0]["severity"], "high")
        finally:
            os.unlink(filepath)

    def test_with_statement_detected(self):
        """Test detection of with statement."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("with (obj) { console.log(x); }\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            with_findings = [f for f in findings if f["pattern"] == "with_statement"]
            self.assertEqual(len(with_findings), 1)
        finally:
            os.unlink(filepath)

    def test_arguments_materialization_detected(self):
        """Test detection of arguments object usage."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("function f() { return arguments[0]; }\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            args_findings = [
                f for f in findings if f["pattern"] == "arguments_materialization"
            ]
            self.assertEqual(len(args_findings), 1)
        finally:
            os.unlink(filepath)

    def test_arguments_in_comment_not_detected(self):
        """Test that 'arguments' in comments is not flagged."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("// This function takes arguments\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            args_findings = [
                f for f in findings if f["pattern"] == "arguments_materialization"
            ]
            self.assertEqual(len(args_findings), 0)
        finally:
            os.unlink(filepath)

    def test_arguments_in_string_not_detected(self):
        """Test that 'arguments' in strings is not flagged."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("const s = 'arguments';\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            args_findings = [
                f for f in findings if f["pattern"] == "arguments_materialization"
            ]
            self.assertEqual(len(args_findings), 0)
        finally:
            os.unlink(filepath)

    def test_megamorphic_access_detected(self):
        """Test detection of megamorphic property access in loops."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("for (let i = 0; i < 10; i++) {\n")
            f.write("  console.log(obj[key]);\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            mega_findings = [
                f for f in findings if f["pattern"] == "megamorphic_access"
            ]
            self.assertEqual(len(mega_findings), 1)
        finally:
            os.unlink(filepath)

    def test_numeric_access_not_detected(self):
        """Test that numeric array access is not flagged as megamorphic."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("for (let i = 0; i < 10; i++) {\n")
            f.write("  console.log(arr[0]);\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            mega_findings = [
                f for f in findings if f["pattern"] == "megamorphic_access"
            ]
            self.assertEqual(len(mega_findings), 0)
        finally:
            os.unlink(filepath)

    def test_dynamic_property_name_detected(self):
        """Test detection of computed property names."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("const obj = { [key]: value };\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            dynamic_findings = [
                f for f in findings if f["pattern"] == "dynamic_property_name"
            ]
            self.assertEqual(len(dynamic_findings), 1)
        finally:
            os.unlink(filepath)

    def test_prototype_pollution_proto_detected(self):
        """Test detection of __proto__ usage."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("obj.__proto__ = other;\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            proto_findings = [
                f for f in findings if f["pattern"] == "prototype_pollution"
            ]
            self.assertEqual(len(proto_findings), 1)
        finally:
            os.unlink(filepath)

    def test_prototype_pollution_setPrototypeOf_detected(self):
        """Test detection of Object.setPrototypeOf usage."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("Object.setPrototypeOf(a, b);\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            proto_findings = [
                f for f in findings if f["pattern"] == "prototype_pollution"
            ]
            self.assertEqual(len(proto_findings), 1)
        finally:
            os.unlink(filepath)

    def test_delete_operator_detected(self):
        """Test detection of delete operator."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("delete obj.prop;\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            delete_findings = [f for f in findings if f["pattern"] == "delete_operator"]
            self.assertEqual(len(delete_findings), 1)
        finally:
            os.unlink(filepath)

    def test_try_catch_in_loop_detected(self):
        """Test detection of try/catch in loops."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("for (let i = 0; i < 10; i++) {\n")
            f.write("  try {\n")
            f.write("    doSomething();\n")
            f.write("  } catch (e) {}\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            try_findings = [f for f in findings if f["pattern"] == "try_catch_in_loop"]
            self.assertEqual(len(try_findings), 1)
        finally:
            os.unlink(filepath)

    def test_sparse_array_detected(self):
        """Test detection of sparse array access."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("arr[99999] = 1;\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            sparse_findings = [f for f in findings if f["pattern"] == "sparse_array"]
            self.assertEqual(len(sparse_findings), 1)
            self.assertIn("99999", sparse_findings[0]["detail"])
        finally:
            os.unlink(filepath)

    def test_small_index_not_sparse(self):
        """Test that small indices are not flagged as sparse."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("arr[100] = 1;\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            sparse_findings = [f for f in findings if f["pattern"] == "sparse_array"]
            self.assertEqual(len(sparse_findings), 0)
        finally:
            os.unlink(filepath)

    def test_implicit_global_detected(self):
        """Test detection of implicit global assignments."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("x = 5;\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            global_findings = [f for f in findings if f["pattern"] == "implicit_global"]
            self.assertEqual(len(global_findings), 1)
        finally:
            os.unlink(filepath)

    def test_declared_variable_not_implicit_global(self):
        """Test that declared variables are not flagged as implicit globals."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            f.write("const x = 5;\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            global_findings = [f for f in findings if f["pattern"] == "implicit_global"]
            self.assertEqual(len(global_findings), 0)
        finally:
            os.unlink(filepath)

    def test_block_comment_handling(self):
        """Test that block comments are properly skipped."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            # Multi-line block comment (single line with /* */ doesn't trigger the block comment logic)
            f.write("/* This is a comment\n")
            f.write("   with eval('test') in it\n")
            f.write("*/\n")
            f.write("const x = 1;\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_js_file(filepath)
            eval_findings = [f for f in findings if f["pattern"] == "eval_usage"]
            # Should not detect eval in block comment
            self.assertEqual(len(eval_findings), 0)
        finally:
            os.unlink(filepath)

    def test_format_human_empty(self):
        """Test format_human with no findings."""
        result = format_human([])
        self.assertEqual(result, "No JavaScript anti-patterns detected.")

    def test_format_human_groups_by_severity(self):
        """Test that format_human groups findings by severity and includes suggestions."""
        findings = [
            {
                "file": "test.js",
                "line": 10,
                "pattern": "eval_usage",
                "detail": "eval() detected",
                "severity": "high",
                "suggestion": "Don't use eval",
            },
            {
                "file": "test.js",
                "line": 20,
                "pattern": "delete_operator",
                "detail": "delete detected",
                "severity": "medium",
                "suggestion": "Set to undefined",
            },
        ]
        result = format_human(findings)
        self.assertIn("HIGH", result)
        self.assertIn("MEDIUM", result)
        self.assertIn("eval_usage", result)
        self.assertIn("Don't use eval", result)
        self.assertIn("Set to undefined", result)

    def test_collect_js_files_single_file(self):
        """Test collect_js_files with a single file."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".js", delete=False) as f:
            filepath = f.name

        try:
            files = collect_js_files(filepath)
            self.assertEqual(files, [filepath])
        finally:
            os.unlink(filepath)

    def test_collect_js_files_directory(self):
        """Test collect_js_files with a directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            # Create some JS files
            js1 = os.path.join(tmpdir, "file1.js")
            js2 = os.path.join(tmpdir, "file2.mjs")
            txt = os.path.join(tmpdir, "readme.txt")

            open(js1, "w").close()
            open(js2, "w").close()
            open(txt, "w").close()

            files = collect_js_files(tmpdir)
            self.assertEqual(len(files), 2)
            self.assertIn(js1, files)
            self.assertIn(js2, files)
            self.assertNotIn(txt, files)


if __name__ == "__main__":
    unittest.main()
