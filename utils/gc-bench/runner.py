#!/usr/bin/env python2.7
# Copyright (c) 2019-present, Facebook, Inc.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import logging
import os
import subprocess
import sys
import tempfile

from gc_bench.subprocess_run import run


logger = logging.getLogger(__name__)


def progress(*args, **kwargs):
    """Output progress updates, on STDERR to avoid contaminating the GC bench
    output
    """
    print(*args, file=sys.stderr, **kwargs)


def translateHeapSize(sz):
    """Translate a "size spec" into a byte count.  The size spec is an
    integer followed by a suffix G, M, K, B, or no suffix.  When a
    suffix is present, it multiplies the integer by G=2^30, M=2^20,
    K=2^10, or B=1.  Returns the resulting byte size.
    """
    szLen = len(sz)
    last = sz[szLen - 1]
    factor = 1
    if last == "B":
        factor = 1
    elif last == "K":
        factor = 1024
    elif last == "M":
        factor = 1024 ** 2
    elif last == "G":
        factor = 1024 ** 3
    return int(sz[: szLen - 1]) * factor


def collectorHeapSize(peakSize):
    return str(translateHeapSize(peakSize)) + "B"


def displayFilePath(file):
    relpath = os.path.relpath(file)
    abspath = os.path.abspath(file)
    return relpath if len(relpath) < len(abspath) else abspath


class JSBytecodeRunner:
    COMPILE_CMD_ARGS = ["-O", "-Wno-undefined-variable", "-emit-binary"]

    def __init__(self, hermesCmd):
        self.hermesCmd = hermesCmd

    def compile(self, name, hermesBytecode, sourceFile, bytecodeFile):
        """Compile the source file to the bytecode file"""
        compileCmd = (
            [self.hermesCmd]
            + JSBytecodeRunner.COMPILE_CMD_ARGS
            + ["-out", bytecodeFile.name]
            + [sourceFile]
        )
        displayCompileCmd = (
            [displayFilePath(self.hermesCmd)]
            + JSBytecodeRunner.COMPILE_CMD_ARGS
            + ["-out", bytecodeFile.name]
            + [displayFilePath(sourceFile)]
        )
        logger.info("Compiling {} ({})".format(name, " ".join(displayCompileCmd)))
        ret, out, err = run(compileCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if ret:
            raise Exception(
                "Failed to compile {} ({})".format(name, " ".join(compileCmd))
            )


class HermesRunner(JSBytecodeRunner):
    def __init__(self, hermesCmd, numTimes, keepTmpFiles):
        JSBytecodeRunner.__init__(self, hermesCmd)
        self.numTimes = numTimes
        self.keepTmpFiles = keepTmpFiles

    def run(self, name, jsfile, gcMinHeap, gcMaxHeap):
        execCmdArgs = [
            "-gc-print-stats",
            "-gc-sanitize-handles=0",
            "-gc-min-heap=" + collectorHeapSize(gcMinHeap),
            "-gc-max-heap=" + collectorHeapSize(gcMaxHeap),
            "-Xes6-symbol",
            "-b",
        ]
        with tempfile.NamedTemporaryFile(
            suffix=".hbc", delete=not self.keepTmpFiles
        ) as bcFile:
            self.compile(name, True, jsfile, bcFile)
            execCmd = [self.hermesCmd] + execCmdArgs + [bcFile.name]
            displayCmd = (
                [displayFilePath(self.hermesCmd)]
                + execCmdArgs
                + [displayFilePath(bcFile.name)]
            )

            logging.info(" ".join(displayCmd))
            progress("Running {} x {:d} ".format(name, self.numTimes), end="")

            # Run the benchmark one time to warm up disk caches.
            # Makes a dramatic difference.
            run(execCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            for _ in range(self.numTimes):
                ret, out, err = run(
                    execCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE
                )
                if ret:
                    raise Exception("Error running exec command: {}".format(displayCmd))
                yield err
                progress(".", end="")
            progress()


class SynthBenchmarkRunner:
    def __init__(self, interpreter, numTimes):
        self.interpreter = interpreter
        self.numTimes = numTimes

    def run(self, name, gcMinHeap, gcMaxHeap, tracefile, bytecodefile, marker=None):
        progress("Running {} x {:d}".format(name, self.numTimes))
        for i in range(self.numTimes):
            ret, out, err = run(
                [
                    self.interpreter,
                    tracefile,
                    bytecodefile,
                    "-gc-min-heap=" + collectorHeapSize(gcMinHeap),
                    "-gc-max-heap=" + collectorHeapSize(gcMaxHeap),
                    # Run synth with no GC before TTI.
                    "-gc-alloc-young=false",
                    "-gc-revert-to-yg-at-tti=true",
                ]
                + (["-m=" + marker] if marker else []),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            if ret:
                raise Exception(
                    "Error running the synth benchmark interpreter\n==stdout==\n{}\n==stderr==\n{}\n".format(
                        out.decode(), err.decode()
                    )
                )
            # Basic block profiler dumps output to stderr.
            stderr = err.decode()
            if len(stderr) > 0:
                profilerOutFile = bytecodefile + "_{0}.bbporfile".format(i)
                progress(
                    "Saving profiler trace for {0} to {1}".format(
                        bytecodefile, profilerOutFile
                    )
                )
                with open(profilerOutFile, "w") as output_file:
                    output_file.write(stderr)

            # The stats output is written to stdout.
            yield out
