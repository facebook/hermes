# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

import logging
import os
import subprocess
import tempfile

from runner import JSRunner, displayFilePath, parseSimpleStats, progress


logger = logging.getLogger(__name__)


class ShermesRunner(JSRunner):
    """Runner for Static Hermes (shermes).

    Unlike HermesRunner which compiles JS to bytecode then interprets it,
    ShermesRunner compiles JS to a native executable via C and runs that.
    """

    COMPILE_CMD_ARGS = ["-O"]

    def __init__(self, shermesCmd, numTimes, keepTmpFiles):
        JSRunner.__init__(self, shermesCmd)
        self.numTimes = numTimes
        self.keepTmpFiles = keepTmpFiles
        # On Windows, the compiled exe dynamically links hermesvm.dll and
        # shermes_console.dll.  There is no rpath on Windows, so we build an
        # env with the library directories on PATH.
        binDir = os.path.dirname(os.path.abspath(shermesCmd))
        buildDir = os.path.dirname(binDir)
        libDir = os.path.join(buildDir, "lib")
        consoleDir = os.path.join(buildDir, "tools", "shermes")
        self.execEnv = os.environ.copy()
        self.execEnv["PATH"] = (
            libDir + os.pathsep + consoleDir + os.pathsep
            + self.execEnv.get("PATH", "")
        )

    def compile(self, name, sourceFile, exeFile):
        """Compile the source file to a native executable."""
        compileCmd = (
            [self.runCmd]
            + ShermesRunner.COMPILE_CMD_ARGS
            + ["-o", exeFile, sourceFile]
        )
        displayCompileCmd = (
            [displayFilePath(self.runCmd)]
            + ShermesRunner.COMPILE_CMD_ARGS
            + ["-o", displayFilePath(exeFile), displayFilePath(sourceFile)]
        )
        logger.info("Compiling {} ({})".format(name, " ".join(displayCompileCmd)))
        proc = subprocess.run(
            compileCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        if proc.returncode:
            raise Exception(
                "Failed to compile {} ({}):\n{}".format(
                    name,
                    " ".join(compileCmd),
                    proc.stderr.decode("utf-8", errors="replace"),
                )
            )

    def run(self, name, jsfile, gcMinHeap, gcInitHeap, gcMaxHeap):
        # gcMinHeap, gcInitHeap, gcMaxHeap are ignored — shermes-compiled
        # binaries don't accept GC tuning flags.
        exeFile = tempfile.NamedTemporaryFile(
            suffix=".exe", delete=False
        )
        exeFile.close()
        exePath = exeFile.name

        try:
            self.compile(name, jsfile, exePath)
            execCmd = [exePath]
            displayCmd = [displayFilePath(exePath)]

            logging.info(" ".join(displayCmd))
            progress("Running {} x {:d} ".format(name, self.numTimes), end="")

            # Run the benchmark one time to warm up disk caches.
            subprocess.run(execCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                           env=self.execEnv)
            for _ in range(self.numTimes):
                proc = subprocess.run(
                    execCmd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    encoding="utf-8",
                    env=self.execEnv,
                )
                if proc.returncode:
                    logger.error(
                        "Error running exec command: {}\n==stdout==\n{}\n==stderr==\n{}\n".format(
                            displayCmd, proc.stdout, proc.stderr
                        )
                    )
                yield (
                    parseSimpleStats(proc.stdout)
                )
                progress(".", end="")
            progress()
        finally:
            if not self.keepTmpFiles:
                try:
                    os.unlink(exePath)
                except OSError:
                    pass
