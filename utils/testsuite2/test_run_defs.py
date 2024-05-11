# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import annotations

import os
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Optional

from hermes import compile_and_run, ExtraCompileVMArgs
from preprocess import generate_source, StrictMode
from skiplist import SkipCategory, SkippedPathsOrFeatures
from typing_defs import PathT
from utils import TestCaseResult, TestResultCode


@dataclass
class TestRunArgs(object):
    """
    Argument necessary to run a given test.
    """

    test_file: PathT
    """Target test file."""
    tests_home: PathT
    """Common ancestor path of passed in file paths."""
    work_dir: PathT
    """Work directory to store preprocessed source."""
    binary_directory: PathT
    """Directory path that contains necessary hermes executables."""
    skipped_paths_features: SkippedPathsOrFeatures
    """Helper for skipping certain tests."""
    test_skiplist: bool
    """Whether to run tests in skiplist."""


class Suite(ABC):
    def __init__(self, path: PathT) -> None:
        self._path = path

    @staticmethod
    def create(path: PathT) -> Optional[Suite]:
        """
        Create a Suite object depending on which testsuite the given file
        belongs to.
        """

        def get_suite_directory(s: str) -> PathT:
            return path[: path.find(s) + len(s)]

        if "test262/" in path:
            return Test262Suite(get_suite_directory("test262"))
        elif "mjsunit/" in path:
            return MjsunitSuite(get_suite_directory("mjsunit"))
        elif "CVEs/" in path:
            pass
        elif "esprima/" in path:
            pass
        elif "flow/" in path:
            pass
        return None

    @property
    def directory(self) -> PathT:
        """Return the testsuite directory path."""
        return self._path

    @property
    @abstractmethod
    def name(self) -> str:
        """Short suite name, e.g., test262."""
        raise NotImplementedError

    def get_full_test_name(self, test_file: PathT) -> str:
        """
        Get the full test name. This is usefuly when printing a relatively
        succinct test name.

        It has form of:
        {suite_name} :: {relative_path_to_suite_path}.

        For example, mjsunit :: regress-011.js.
        """
        return f"{self.name} :: {os.path.relpath(test_file, self.directory)}"

    @abstractmethod
    async def run_test(self, args: TestRunArgs) -> TestCaseResult:
        """
        Callback function to be called in run(). This should be implemented
        for each testsuite.

        Args:
        TestRunArgs: All arguments necessary to run the given test.

        Returns:
        TestCaseResult, indicating whether this test passes, if not, what is
        the error message.
        """
        raise NotImplementedError


class Test262Suite(Suite):
    @property
    def name(self) -> str:
        return "test262"

    def generate_js_source(
        self,
        tmp_dir: PathT,
        base_name_no_ext: str,
        flags_str: str,
        source: str,
        is_strict: bool,
    ) -> PathT:
        """
        Generate the JS source file to run, prepend the strict directive if
        necessary. The returned path encodes its strict mode and flags.
        """
        strict_indicator = ".strict" if is_strict else ""
        js_source = os.path.join(
            tmp_dir, f"{base_name_no_ext}{strict_indicator}{flags_str}.js"
        )
        with open(js_source, "wb") as writer:
            if is_strict:
                # Add the directive for strict mode
                writer.write("'use strict';\n".encode("utf-8"))
            writer.write(source.encode("utf-8"))
        return js_source

    async def run_test(self, args: TestRunArgs) -> TestCaseResult:
        """
        Load and preprocess the test file, check if it's skipped. If not, run
        the preprocessed source code, return the result.
        """
        with open(args.test_file, "rb") as reader:
            content = reader.read().decode("utf-8")
        full_test_name = self.get_full_test_name(args.test_file)
        test_case = generate_source(content, self.directory, full_test_name)
        if "testIntl.js" in test_case.includes:
            msg = f"SKIP: No support for multiple Intl constructors in {full_test_name}"
            return TestCaseResult(full_test_name, TestResultCode.TEST_SKIPPED, msg)

        flags = test_case.flags
        if "async" in flags:
            return TestCaseResult(
                full_test_name,
                TestResultCode.TEST_SKIPPED,
                "SKIP: Test has `async` flag",
            )
        if "module" in flags:
            return TestCaseResult(
                full_test_name,
                TestResultCode.TEST_SKIPPED,
                "SKIP: Test has `module` flag",
            )

        base_name = os.path.basename(args.test_file)
        base_name_no_ext = os.path.splitext(base_name)[0]

        # Check if we need to skip this test due to unsupported features.
        if not args.test_skiplist:
            for f in test_case.features:
                if skip_result := args.skipped_paths_features.try_skip(
                    f,
                    [
                        SkipCategory.UNSUPPORTED_FEATURES,
                        SkipCategory.PERMANENT_UNSUPPORTED_FEATURES,
                    ],
                    full_test_name,
                ):
                    return skip_result

        tmp_dir: PathT = os.path.join(
            args.work_dir,
            os.path.dirname(os.path.relpath(args.test_file, args.tests_home)),
        )
        os.makedirs(tmp_dir, exist_ok=True)
        flags_str = ("." + "_".join(sorted(flags))) if len(flags) > 0 else ""
        js_sources = []
        if StrictMode.STRICT in test_case.strict_mode:
            js_sources.append(
                self.generate_js_source(
                    tmp_dir, base_name_no_ext, flags_str, test_case.source, True
                )
            )

        if StrictMode.NO_STRICT in test_case.strict_mode:
            js_sources.append(
                self.generate_js_source(
                    tmp_dir, base_name_no_ext, flags_str, test_case.source, False
                )
            )

        disable_handle_san = args.skipped_paths_features.should_skip_cat(
            args.test_file, SkipCategory.HANDLESAN_SKIP_LIST
        )

        return await compile_and_run(
            full_test_name,
            js_sources,
            test_case.strict_mode,
            args.binary_directory,
            test_case.negative,
            disable_handle_san,
        )


class MjsunitSuite(Suite):
    @property
    def name(self) -> str:
        return "mjsunit"

    async def run_test(self, args: TestRunArgs) -> TestCaseResult:
        # Compute a temporary path to store preprocessed test code.
        base_name = os.path.basename(args.test_file)
        base_name_no_ext = os.path.splitext(base_name)[0]
        tmp_dir: PathT = os.path.join(
            args.work_dir,
            os.path.dirname(os.path.relpath(args.test_file, args.tests_home)),
        )
        os.makedirs(tmp_dir, exist_ok=True)
        js_source = os.path.join(tmp_dir, f"{base_name_no_ext}_generated.js")

        # Read the original test source, preprocess it and save the preprocessed
        # code to the temporary path.
        with open(args.test_file, "rb") as reader:
            content = reader.read().decode("utf-8")
        full_test_name = self.get_full_test_name(args.test_file)
        test_case = generate_source(content, self.directory, full_test_name)
        with open(js_source, "wb") as writer:
            writer.write(test_case.source.encode("utf-8"))

        # Run Hermes on the generated temporary JS file.
        disable_handle_san = args.skipped_paths_features.should_skip_cat(
            args.test_file, SkipCategory.HANDLESAN_SKIP_LIST
        )
        extra_compile_vm_args = ExtraCompileVMArgs(
            compile_args=["-Xes6-class", "-Xenable-tdz"]
        )
        return await compile_and_run(
            full_test_name,
            [js_source],
            test_case.strict_mode,
            args.binary_directory,
            test_case.negative,
            disable_handle_san,
            extra_compile_vm_args,
        )


def get_suite(filepath: PathT) -> Optional[Suite]:
    """Get testsuite for given test file."""
    return Suite.create(filepath)
