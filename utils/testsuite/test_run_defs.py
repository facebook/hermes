# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import annotations

import json

import os
from abc import ABC, abstractmethod
from copy import deepcopy
from dataclasses import dataclass
from typing import Optional

from .es_ast import diff
from .hermes import compile_and_run, CompileRunArgs, ExtraCompileVMArgs, generate_ast
from .preprocess import generate_source, StrictMode
from .skiplist import SkipCategory, SkippedPathsOrFeatures
from .typing_defs import PathT
from .utils import get_hermes_supported_test262_features, TestCaseResult, TestResultCode


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
    lazy: bool
    """Whether to force lazy evaluation."""
    shermes: bool
    """Whether to test with shermes"""
    opt: bool
    """Whether to enable optimizer"""
    extra_compile_vm_args: ExtraCompileVMArgs
    """Extra compiler and VM arguments"""
    timeout: int
    """Timeout (in seconds) for compiling/running each test."""


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
            # Search from the end in case the path has repeated folder names.
            return path[: path.rfind(s) + len(s)]

        if "test262/" in path:
            return Test262Suite(get_suite_directory("test262"))
        elif "mjsunit/" in path:
            return MjsunitSuite(get_suite_directory("mjsunit"))
        elif "CVEs/" in path:
            return CvesSuite(get_suite_directory("CVEs"))
        elif "esprima/" in path:
            return EsprimaAstCheckSuite(get_suite_directory("esprima"))
        elif "flow/" in path:
            # Use "flow/" to avoid matching flowtest/
            return FlowAstCheckSuite(get_suite_directory("flow/"))
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
        Get the full test name. This is useful when printing a relatively
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
    """
    Support running test262 testsuite. This is also used as base for the CVEs
    testsuite since a few tests there use the same style frontmatter as test262.
    """

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
        # Check if we need to skip this test due to unsupported features.
        for f in test_case.features:
            # If it's not running shermes test and this feature is specifically
            # supported by the built hermes binary, we don't need to test if it
            # is in the skiplist. We may still skip it due to the rest features
            # of this test case though.
            if not args.shermes and f in get_hermes_supported_test262_features(
                args.binary_directory
            ):
                continue
            if skip_result := args.skipped_paths_features.try_skip(
                f,
                [
                    SkipCategory.UNSUPPORTED_FEATURES,
                    SkipCategory.PERMANENT_UNSUPPORTED_FEATURES,
                ],
                full_test_name,
            ):
                return skip_result

        base_name = os.path.basename(args.test_file)
        base_name_no_ext = os.path.splitext(base_name)[0]

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

        compile_run_args = CompileRunArgs(
            full_test_name,
            test_case.strict_mode,
            args.binary_directory,
            test_case.expected_failure,
            disable_handle_san,
            args.lazy,
            args.shermes,
            args.opt,
            args.timeout,
            args.extra_compile_vm_args,
        )
        return await compile_and_run(js_sources, compile_run_args)


class CvesSuite(Test262Suite):
    """
    Support running the CVE testsuite.
    """

    @property
    def name(self) -> str:
        return "CVEs"


class MjsunitSuite(Suite):
    """
    Support running mjsunit testsuite. This can be used to run any testsuite
    that requires running source directly and checking compiler/runtime error.
    """

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
        extra_compile_vm_args = deepcopy(args.extra_compile_vm_args)
        extra_compile_vm_args.compile_args += ["-Xes6-class", "-Xenable-tdz"]
        compile_run_args = CompileRunArgs(
            full_test_name,
            test_case.strict_mode,
            args.binary_directory,
            test_case.expected_failure,
            disable_handle_san,
            args.lazy,
            args.shermes,
            args.opt,
            args.timeout,
            extra_compile_vm_args,
        )
        return await compile_and_run([js_source], compile_run_args)


class AstCheckSuite(Suite):
    """
    This works for both ESPRIMA and FLOW test suites, which checks the
    correctness of AST generated by Hermes.
    """

    @property
    def is_flow(self) -> bool:
        """We need to pass in different flags to Hermes for flow tests."""
        return False

    def get_expected_file_name(self, test_file: PathT) -> Optional[PathT]:
        """
        Get the expect file name, which could be one of following:
        - *.tree.json
        - *.failure.json (indicating failure)
        - *.token.json (we don't support it for now)
        If none is found, return None.
        """

        filename_stem = ""
        if test_file.endswith(".source.js"):
            filename_stem = test_file[:-10]
        else:
            # Must be *.js here
            filename_stem = test_file[:-3]
        tree_file = filename_stem + ".tree.json"
        if os.path.isfile(tree_file):
            return tree_file
        failure_file = filename_stem + ".failure.json"
        if os.path.isfile(failure_file):
            return failure_file
        token_file = filename_stem + ".tokens.json"
        if os.path.isfile(token_file):
            return token_file
        return None

    async def run_test(self, args: TestRunArgs) -> TestCaseResult:
        # Check if the test file is valid and its expect file can be found.
        full_test_name = self.get_full_test_name(args.test_file)
        if not args.test_file.endswith(".js"):
            msg = "FAIL: Invalid test case"
            return TestCaseResult(full_test_name, TestResultCode.TEST_FAILED, msg)
        expected_file = self.get_expected_file_name(args.test_file)
        if not expected_file:
            msg = "FAIL: Can't find expected file"
            return TestCaseResult(
                full_test_name,
                TestResultCode.TEST_FAILED,
                msg,
            )

        # Expect files end with ".tokens.json" expect the parser to produce
        # tokens, which Hermes does not support, so skip them here.
        if expected_file.endswith(".tokens.json"):
            msg = "SKIP: Expect file *.tokens.json is not supported"
            return TestCaseResult(full_test_name, TestResultCode.TEST_SKIPPED, msg)

        # This expects a failure of the current test. If the result_code is not
        # TEST_PASSED, rerun `generate_ast` with semantic validation and do
        # the check again.
        async def check_for_expected_failure(
            result_code: TestResultCode,
        ) -> TestCaseResult:
            nonlocal args, full_test_name
            if result_code == TestResultCode.TEST_PASSED:
                # If it should error and it didn't, try again with
                # semantic validation. Some tests only error in semantic
                # validation, but we want to avoid running it because it
                # also outputs errors for unsupported compiler features
                # that Hermes does parse correctly.
                result_code = (
                    await generate_ast(
                        full_test_name,
                        args.test_file,
                        args.binary_directory,
                        self.is_flow,
                        True,
                        args.timeout,
                    )
                ).code
            # Check again after possible retry.
            if result_code != TestResultCode.TEST_PASSED:
                msg = "PASS: Expected failure"
                return TestCaseResult(full_test_name, TestResultCode.TEST_PASSED, msg)
            else:
                msg = "FAIL: Test is expected to fail"
                return TestCaseResult(full_test_name, TestResultCode.TEST_FAILED, msg)

        result = await generate_ast(
            full_test_name,
            args.test_file,
            args.binary_directory,
            self.is_flow,
            False,
            args.timeout,
        )
        if expected_file.endswith(".tree.json"):
            try:
                with open(expected_file, "r") as json_f:
                    expected_ast = json.load(json_f)
            except (json.JSONDecodeError, OSError) as err:
                msg = f"FAIL: Failed to load expected AST file: {err}"
                return TestCaseResult(full_test_name, TestResultCode.TEST_FAILED, msg)

            if isinstance(expected_ast, dict) and "errors" in expected_ast:
                return await check_for_expected_failure(result.code)
            return diff(result.output, full_test_name, expected_ast)
        else:
            assert expected_file.endswith(".failure.json")
            return await check_for_expected_failure(result.code)


class FlowAstCheckSuite(AstCheckSuite):
    @property
    def name(self) -> str:
        return "flow"

    @property
    def is_flow(self) -> bool:
        return True


class EsprimaAstCheckSuite(AstCheckSuite):
    @property
    def name(self) -> str:
        return "esprima"


def get_suite(filepath: PathT) -> Optional[Suite]:
    """Get testsuite for given test file."""
    return Suite.create(filepath)
