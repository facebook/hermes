# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

import json
import logging
import os
import subprocess
import sys
import tempfile


logger = logging.getLogger(__name__)


def progress(*args, **kwargs):
    """Output progress updates, on STDERR to avoid contaminating the GC bench
    output
    """
    print(*args, file=sys.stderr, flush=True, **kwargs)


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
        factor = 1024**2
    elif last == "G":
        factor = 1024**3
    return int(sz[: szLen - 1]) * factor


def collectorHeapSize(peakSize):
    return str(translateHeapSize(peakSize)) + "B"


def displayFilePath(file):
    relpath = os.path.relpath(file)
    abspath = os.path.abspath(file)
    return relpath if len(relpath) < len(abspath) else abspath

def parseTime(log):
    ix = log.find("Time: ")
    lineEnd = log.find("\n", ix)
    return float(log[ix+6:lineEnd]);

def parseSimpleStats(log):
    return { "general": { "totalTime": parseTime(log) } }

def parseStats(label, log):
    """Finds the label in the log output, and parses a JSON object immediately
    following it (if such an object exists).  Returns a python object
    representing the JSON upon success and None otherwise.
    """
    sigil = "{} stats:\n".format(label)
    ix = log.find(sigil)
    
    if ix == -1:
        # Sigil was not found.
        return None

    post_sigil_log = log[ix + len(sigil) :].strip()
    try:
        decoder = json.JSONDecoder()
        return decoder.raw_decode(post_sigil_log)[0]
    except json.JSONDecodeError as e:
        logger.warn("Log could not be parsed. Its contents are printed below.")
        logger.warn(log, e)
        return None


class JSRunner:
    def __init__(self, runCmd):
        self.runCmd = runCmd


class JSBytecodeRunner(JSRunner):
    COMPILE_CMD_ARGS = ["-O", "-Wno-undefined-variable", "-emit-binary"]

    def __init__(self, hermesCmd):
        JSRunner.__init__(self, hermesCmd)

    def compile(self, name, hermesBytecode, sourceFile, bytecodeFile):
        """Compile the source file to the bytecode file"""
        compileCmd = (
            [self.runCmd]
            + JSBytecodeRunner.COMPILE_CMD_ARGS
            + ["-out", bytecodeFile.name]
            + [sourceFile]
        )
        displayCompileCmd = (
            [displayFilePath(self.runCmd)]
            + JSBytecodeRunner.COMPILE_CMD_ARGS
            + ["-out", bytecodeFile.name]
            + [displayFilePath(sourceFile)]
        )
        logger.info("Compiling {} ({})".format(name, " ".join(displayCompileCmd)))
        proc = subprocess.run(
            compileCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        if proc.returncode:
            raise Exception(
                "Failed to compile {} ({})".format(name, " ".join(compileCmd))
            )


class HermesRunner(JSBytecodeRunner):
    def __init__(self, hermesCmd, numTimes, keepTmpFiles):
        JSBytecodeRunner.__init__(self, hermesCmd)
        self.numTimes = numTimes
        self.keepTmpFiles = keepTmpFiles

    def run(self, name, jsfile, gcMinHeap, gcInitHeap, gcMaxHeap):
        execCmdArgs = [
            "-gc-print-stats",
            "-gc-sanitize-handles=0",
            "-b",
        ]
        if gcMinHeap:
            execCmdArgs += ["-gc-min-heap=" + collectorHeapSize(gcMinHeap)]
        if gcInitHeap:
            execCmdArgs += ["-gc-init-heap=" + collectorHeapSize(gcInitHeap)]
        if gcMaxHeap:
            execCmdArgs += ["-gc-max-heap=" + collectorHeapSize(gcMaxHeap)]
        with tempfile.NamedTemporaryFile(
            suffix=".hbc", delete=not self.keepTmpFiles
        ) as bcFile:
            self.compile(name, True, jsfile, bcFile)
            execCmd = [self.runCmd] + execCmdArgs + [bcFile.name]
            displayCmd = (
                [displayFilePath(self.runCmd)]
                + execCmdArgs
                + [displayFilePath(bcFile.name)]
            )

            logging.info(" ".join(displayCmd))
            progress("Running {} x {:d} ".format(name, self.numTimes), end="")

            # Run the benchmark one time to warm up disk caches.
            # Makes a dramatic difference.
            subprocess.run(execCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            for _ in range(self.numTimes):
                proc = subprocess.run(
                    execCmd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    encoding="utf-8",
                )
                if proc.returncode:
                    # log to stderr instead of raise Exception because there
                    # may still be stats in stderr
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


class SynthBenchmarkRunner:
    def __init__(self, interpreter, numTimes):
        self.interpreter = interpreter
        self.numTimes = numTimes

    def run(
        self,
        name,
        gcMinHeap,
        gcInitHeap,
        gcMaxHeap,
        tracefile,
        bytecodefiles,
        marker=None,
    ):
        progress("Running {} x {:d}".format(name, self.numTimes))
        run_args = [
            self.interpreter,
            tracefile,
            *bytecodefiles,
            # Run synth with no GC before TTI.
            "-gc-alloc-young=false",
            "-gc-revert-to-yg-at-tti=true",
        ]
        if gcMinHeap:
            run_args += ["-gc-min-heap=" + collectorHeapSize(gcMinHeap)]
        if gcInitHeap:
            run_args += ["-gc-init-heap=" + collectorHeapSize(gcInitHeap)]
        if gcMaxHeap:
            run_args += ["-gc-max-heap=" + collectorHeapSize(gcMaxHeap)]
        if marker:
            run_args += ["-m=" + marker]
        logger.debug("Run command: {}".format(" ".join(run_args)))
        for i in range(self.numTimes):
            proc = subprocess.run(
                run_args,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                encoding="utf-8",
            )
            if proc.returncode:
                raise Exception(
                    "Error running the synth benchmark interpreter\n==stdout==\n{}\n==stderr==\n{}\n".format(
                        proc.stdout, proc.stderr
                    )
                )
            # Basic block profiler dumps output to stderr.
            if len(proc.stderr) > 0:
                profilerOutFile = "{}_{}.bbprofile".format(tracefile, i)
                progress(
                    "Saving profiler trace for {} to {}".format(
                        tracefile, profilerOutFile
                    )
                )
                with open(profilerOutFile, "w") as output_file:
                    output_file.write(proc.stderr)

            # The stats output is written to stdout.
            yield (parseStats("GC", proc.stdout), parseStats("Process", proc.stdout))

class V8Runner(JSBytecodeRunner):
    def __init__(self, v8Cmd, numTimes, jitless):
        JSRunner.__init__(self, v8Cmd)
        self.numTimes = numTimes
        self.jitless = jitless

    def run(self, name, jsfile, gcMinHeap, gcInitHeap, gcMaxHeap):
        execCmdArgs = []
        if self.jitless:
            execCmdArgs += ["--jitless"]
        execCmd = [self.runCmd] + execCmdArgs + [jsfile]
        displayCmd = (
            [displayFilePath(self.runCmd)]
            + execCmdArgs
            + [displayFilePath(jsfile)]
        )
        
        logging.info(" ".join(displayCmd))
        progress("Running {} x {:d} ".format(name, self.numTimes), end="")

        # Run the benchmark one time to warm up disk caches.
        # Makes a dramatic difference.
        subprocess.run(execCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        for _ in range(self.numTimes):
            proc = subprocess.run(
                execCmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                encoding="utf-8",
            )
            if proc.returncode:
                # log to stderr instead of raise Exception because there
                # may still be stats in stderr
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
            
