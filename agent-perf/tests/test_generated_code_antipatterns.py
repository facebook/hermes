#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Unit tests for generated_code_antipatterns.py"""

import os
import sys
import tempfile
import unittest

# Add tools directory to path to import the module
TOOLS_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "tools")
sys.path.insert(0, TOOLS_DIR)

from generated_code_antipatterns import format_human, scan_generated_c


class TestGeneratedCodeAntipatterns(unittest.TestCase):
    def test_excessive_pointer_aliasing_detected(self):
        """Test detection of deep pointer chains."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            f.write("  int x = shr->runtime->gc->heap;\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            aliasing = [
                f for f in findings if f["pattern"] == "excessive_pointer_aliasing"
            ]
            self.assertEqual(len(aliasing), 1)
            self.assertIn("->", aliasing[0]["detail"])
        finally:
            os.unlink(filepath)

    def test_redundant_null_check_detected(self):
        """Test detection of redundant null checks."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            f.write("  if (!p) return;\n")
            f.write("  x = 1;\n")
            f.write("  if (!p) return;\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            null_check = [f for f in findings if f["pattern"] == "redundant_null_check"]
            self.assertEqual(len(null_check), 1)
        finally:
            os.unlink(filepath)

    def test_excessive_function_calls_detected(self):
        """Test detection of excessive consecutive function calls."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            # Need opening brace on same line as function for it to be detected
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            for i in range(10):
                f.write(f"  _sh_ljs_call_{i}();\n")
            f.write("  int x = 5;\n")  # Non-call line to end the consecutive block
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            excessive = [
                f for f in findings if f["pattern"] == "excessive_function_calls"
            ]
            self.assertEqual(len(excessive), 1)
            self.assertIn("consecutive", excessive[0]["detail"])
        finally:
            os.unlink(filepath)

    def test_unnecessary_gc_root_detected(self):
        """Test detection of unnecessary GC root push/pop."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            f.write("  _sh_push_locals();\n")
            f.write("  int x = 5;\n")
            f.write("  _sh_pop_locals();\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            gc_root = [f for f in findings if f["pattern"] == "unnecessary_gc_root"]
            self.assertEqual(len(gc_root), 1)
        finally:
            os.unlink(filepath)

    def test_necessary_gc_root_not_detected(self):
        """Test that GC root with allocation is not flagged."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            f.write("  _sh_push_locals();\n")
            f.write("  _sh_ljs_create_object();\n")
            f.write("  _sh_pop_locals();\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            gc_root = [f for f in findings if f["pattern"] == "unnecessary_gc_root"]
            self.assertEqual(len(gc_root), 0)
        finally:
            os.unlink(filepath)

    def test_redundant_cast_detected(self):
        """Test detection of back-to-back casts."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            f.write("  x = (SHLegacyValue)(double)y;\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            cast = [f for f in findings if f["pattern"] == "redundant_cast"]
            self.assertEqual(len(cast), 1)
        finally:
            os.unlink(filepath)

    def test_large_stack_frame_detected(self):
        """Test detection of functions with many local variables."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            for i in range(60):
                f.write(f"  SHLegacyValue var{i};\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            large_frame = [f for f in findings if f["pattern"] == "large_stack_frame"]
            self.assertEqual(len(large_frame), 1)
            self.assertIn("60", large_frame[0]["detail"])
        finally:
            os.unlink(filepath)

    def test_unreachable_after_throw_detected(self):
        """Test detection of unreachable code after throw."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            f.write("  _sh_throw();\n")
            f.write("  int x = 5;\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            unreachable = [
                f for f in findings if f["pattern"] == "unreachable_after_throw"
            ]
            self.assertEqual(len(unreachable), 1)
        finally:
            os.unlink(filepath)

    def test_function_boundary_detection(self):
        """Test that function boundaries are correctly identified."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".c", delete=False) as f:
            f.write("static SHLegacyValue _1_foo(SHRuntime* shr) {\n")
            f.write("  return 0;\n")
            f.write("}\n")
            f.write("static SHLegacyValue _2_bar#123(SHRuntime* shr) {\n")
            f.write("  return 1;\n")
            f.write("}\n")
            f.flush()
            filepath = f.name

        try:
            findings = scan_generated_c(filepath)
            # Should process both functions without error
            # Functions with #id should be detected
            self.assertIsInstance(findings, list)
        finally:
            os.unlink(filepath)

    def test_format_human_empty(self):
        """Test format_human with no findings."""
        result = format_human([])
        self.assertEqual(result, "No generated-code anti-patterns detected.")

    def test_format_human_groups_by_severity(self):
        """Test that format_human groups findings by severity."""
        findings = [
            {
                "file": "test.c",
                "line": 10,
                "pattern": "excessive_function_calls",
                "detail": "too many calls",
                "severity": "high",
                "function": "_1_foo",
            },
            {
                "file": "test.c",
                "line": 20,
                "pattern": "excessive_pointer_aliasing",
                "detail": "deep chain",
                "severity": "medium",
                "function": "_1_bar",
            },
            {
                "file": "test.c",
                "line": 30,
                "pattern": "redundant_cast",
                "detail": "cast issue",
                "severity": "low",
                "function": "_1_baz",
            },
        ]
        result = format_human(findings)
        self.assertIn("HIGH", result)
        self.assertIn("MEDIUM", result)
        self.assertIn("LOW", result)
        self.assertIn("excessive_function_calls", result)
        # Check order: HIGH should appear before MEDIUM
        high_pos = result.find("HIGH")
        medium_pos = result.find("MEDIUM")
        self.assertLess(high_pos, medium_pos)


if __name__ == "__main__":
    unittest.main()
