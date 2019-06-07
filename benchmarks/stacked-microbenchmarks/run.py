# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
import subprocess
import timeit
from os import path


cached_jsc_results = {}
cached_hermes_results = {}

JSC_OSX = "/System/Library/Frameworks/JavaScriptCore.framework/Versions/A/Resources/jsc"
JSC_DEV = path.expanduser("~/fbsource/xplat/buck-out/gen/jsc/cli")
if path.isfile(JSC_DEV):
    JSC = JSC_DEV
elif path.isfile(JSC_OSX):
    JSC = JSC_OSX
else:
    raise RuntimeError("JSC is not found at '{}' or '{}'".format(JSC_DEV, JSC_OSX))

HERMESC = "~/fbsource/xplat/buck-out/gen/hermes/tools/hermes/hermes"
HVM = "~/fbsource/xplat/buck-out/gen/hermes/tools/hvm/hvm-lean"


def measure_cmd(cmd_str, N=5):
    full_cmd = 'subprocess.call("{}", shell=True)'.format(cmd_str)
    setup = "import subprocess"
    results = []
    for _ in range(0, N):
        results.append(timeit.timeit(full_cmd, setup=setup, number=1))
    return min(results)


def measure_with_jsc(file_name):
    if file_name in cached_jsc_results:
        return cached_jsc_results[file_name]

    execute_cmd = "{} {} --useJIT=False".format(JSC, file_name)
    return measure_cmd(execute_cmd)


def measure_with_hermes(file_name):
    if file_name in cached_hermes_results:
        return cached_hermes_results[file_name]

    OUTPUT_FILE = "out"
    compile_cmd = " ".join(
        [HERMESC, file_name, "-O", "-emit-binary", "-out", OUTPUT_FILE]
    )
    subprocess.call(compile_cmd, shell=True)

    results = []
    for heap_size in ["128M", "256M", "512M", "1G"]:
        execute_cmd = " ".join([HVM, OUTPUT_FILE, "-gc-init-heap", heap_size])
        results.append(measure_cmd(execute_cmd, N=10))
    return min(results)


class Benchmark:
    def __init__(self, name, file, base_file):
        self.name = name
        self.file = file
        self.base_file = base_file

    def measure(self):
        if self.base_file:
            jsc_base_result = measure_with_jsc(self.base_file)
            hermes_base_result = measure_with_hermes(self.base_file)
        else:
            jsc_base_result = 0
            hermes_base_result = 0

        jsc_cur_result = measure_with_jsc(self.file)
        hermes_cur_result = measure_with_hermes(self.file)

        return (hermes_cur_result - hermes_base_result) / (
            jsc_cur_result - jsc_base_result
        )


benchmarks = [
    Benchmark("Loop", "loop.js", None),
    Benchmark("Function Call", "loop_call.js", "loop.js"),
    Benchmark("Create Array", "loop_call_create_array.js", "loop_call.js"),
    Benchmark("Create Object", "loop_call_create_obj.js", "loop_call.js"),
    Benchmark(
        "Named String Property Write In Object",
        "loop_call_create_obj_write_named_str_prop.js",
        "loop_call_create_obj.js",
    ),
    Benchmark(
        "Named Int Property Write In Object",
        "loop_call_create_obj_write_named_int_prop.js",
        "loop_call_create_obj.js",
    ),
    Benchmark(
        "Named String Property Write in Array",
        "loop_call_create_array_write_named_str_prop.js",
        "loop_call_create_array.js",
    ),
    Benchmark(
        "Named Int Property Write in Array After Array Creation",
        "loop_call_create_array_write_named_int_prop.js",
        "loop_call_create_array.js",
    ),
    Benchmark(
        "Named Int Property Write in Array During Array Creation",
        "loop_call_create_array_write_named_int_prop_embedded.js",
        "loop_call_create_array.js",
    ),
    Benchmark(
        "Computed String Property Write In Object",
        "loop_call_with_str_args_create_obj_write_computed_str_prop.js",
        "loop_call_with_str_args_create_obj.js",
    ),
    Benchmark(
        "Computed Int Property Write In Object",
        "loop_call_with_int_args_create_obj_reduced_rep_write_computed_int_prop.js",
        "loop_call_with_int_args_create_obj_reduced_rep.js",
    ),
    Benchmark(
        "Named String Property Read in Object",
        "loop_call_with_obj_arg_and_str_arg_read_computed_prop.js",
        "loop_call_with_obj_arg_and_str_arg.js",
    ),
    Benchmark(
        "Computed String Property Read in Object",
        "loop_call_with_obj_arg_read_named_prop.js",
        "loop_call_with_obj_arg.js",
    ),
    Benchmark(
        "Constructor",
        "loop_call_with_obj_arg_constructor.js",
        "loop_call_with_obj_arg.js",
    ),
    Benchmark(
        "String Concat",
        "loop_call_with_str_args_str_concat.js",
        "loop_call_with_str_args.js",
    ),
    Benchmark(
        "Continuous String Concat",
        "loop_reduced_rep_cont_str_concat.js",
        "loop_reduced_rep.js",
    ),
    Benchmark("Numeric", "loop_numerical.js", "loop_numerical_base.js"),
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "--filter", dest="filter", type=str, help="Filter benchmark name by"
)
args = parser.parse_args()

print(
    "Requirement: run this script in this folder. And make sure you have built both hermes and hvm-lean in opt mode"
)
print(
    "Warning: the results should be used only as a directional data point (i.e. how far it is from 1). No effort has been put in to make the numbers highly accurate/stable."
)
for bench in benchmarks:
    if args.filter and bench.name.find(args.filter) == -1:
        continue
    print(bench.name + ": " + str(bench.measure()))
