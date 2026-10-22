#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Check regenerated sandbox C against what HermesSandboxRuntime.cpp expects.

Usage: python3 verify_generated.py <path to hermes> [--stack-size BYTES]

<path to hermes> is the directory that contains API/hermes_sandbox. The script
checks the dbg and opt outputs in API/hermes_sandbox/external and exits with a
non-zero status if any check fails.
"""

import argparse
import glob
import os
import re
import sys

failures = []


def check(cond, msg):
    print(("  ok    " if cond else "  FAIL  ") + msg)
    if not cond:
        failures.append(msg)


def read(path):
    with open(path, errors="replace") as f:
        return f.read()


def host_imports(host_src):
    """The imports that HermesSandboxRuntime.cpp implements."""
    return set(re.findall(r"/\* import: '([^']*)' '([^']*)' \*/", host_src))


def host_exports(host_src):
    """The module functions that HermesSandboxRuntime.cpp calls."""
    names = set(re.findall(r"\bw2c_hermes_([A-Za-z0-9_]+)\(", host_src))
    # Functions starting with an underscore are imports that the host defines.
    return {n for n in names if not n.startswith("_import_")}


def bytecode_version(root):
    text = read(os.path.join(root, "include/hermes/BCGen/HBC/BytecodeVersion.h"))
    return int(re.search(r"BYTECODE_VERSION = (\d+);", text).group(1))


def data_segments(path):
    """The bytes of the data segments in one generated .c file."""
    out = bytearray()
    in_segment = False
    with open(path, errors="replace") as f:
        for line in f:
            if re.match(r"(static )?const u8 data_segment_data_", line):
                in_segment = True
            elif in_segment and line.startswith("};"):
                in_segment = False
            elif in_segment:
                out.extend(int(h, 16) for h in re.findall(r"0x([0-9a-f]{2})", line))
    return bytes(out)


def functions_containing(paths, needle):
    """Yield the bodies of generated functions whose name contains needle."""
    start = re.compile(r"^(?:static )?(?:u32|u64|f32|f64|void) (w2c_hermes_\w+)\(")
    for path in paths:
        name, body = None, []
        with open(path, errors="replace") as f:
            for line in f:
                m = start.match(line)
                if m and line.rstrip().endswith("{"):
                    if name and needle in name:
                        yield "".join(body)
                    name, body = m.group(1), []
                elif name is not None:
                    body.append(line)
        if name and needle in name:
            yield "".join(body)


def vtable_entries(root, host_src):
    abi = read(os.path.join(root, "API/hermes_abi/hermes_abi.h"))
    vtable = abi[abi.index("struct HermesABIRuntimeVTable {") :]
    vtable = vtable[: vtable.index("};")]
    header = re.findall(r"\(\s*\*\s*(\w+)\s*\)", vtable)
    mirror = host_src[host_src.index("#define SANDBOX_CONTEXT_VTABLE_FUNCTIONS") :]
    mirror = re.findall(r"F\((\w+),", mirror[: mirror.index("\n\n")])
    return header, mirror


def check_variant(ext, variant, imports, exports, version, stack_size):
    print("== %s" % variant)
    cfiles = sorted(
        glob.glob(os.path.join(ext, "hermes_sandbox_impl_%s_compiled_*.c" % variant))
    )
    header = os.path.join(ext, "hermes_sandbox_impl_%s_compiled.h" % variant)
    impl = os.path.join(ext, "hermes_sandbox_impl_%s_compiled-impl.h" % variant)
    if not cfiles or not os.path.exists(header) or not os.path.exists(impl):
        check(False, "the %s output exists" % variant)
        return
    hdr = read(header)
    src = "".join(read(p) for p in cfiles)

    found = set(re.findall(r"/\* import: '([^']*)' '([^']*)' \*/", hdr))
    check(
        found == imports,
        "imports are exactly the %d that the host implements (extra: %s, missing: %s)"
        % (len(imports), sorted(found - imports), sorted(imports - found)),
    )
    declared = set(re.findall(r"\bw2c_hermes_(\w+)\(", hdr))
    check(
        exports <= declared,
        "the module has the exports the host calls (missing: %s)"
        % sorted(exports - declared),
    )

    constants = []
    for needle in ("sanityCheck", "createBCProviderFromBuffer"):
        for body in functions_containing(cfiles, needle):
            constants += [int(c) for c in re.findall(r"= (\d+)u;", body)]
    check(
        version in constants,
        "the bytecode version check uses BYTECODE_VERSION %d" % version,
    )

    m = re.search(
        r"var_i0 = (\d+)u;\s*\n\s*instance->w2c_0x5F_stack_base = var_i0;\s*\n"
        r"\s*var_i0 = (\d+)u;\s*\n\s*instance->w2c_0x5F_stack_end = var_i0;",
        src,
    )
    check(
        m is not None and int(m.group(1)) - int(m.group(2)) == stack_size,
        "the stack is %d bytes (found %s)"
        % (stack_size, int(m.group(1)) - int(m.group(2)) if m else "nothing"),
    )

    data = b"".join(data_segments(p) for p in cfiles)
    paths = sorted(set(re.findall(rb"/(?:Users|home|data/users)/[ -~]{0,60}", data)))
    check(not paths, "the data has no absolute paths (found: %s)" % paths[:3])
    check(
        not re.search(r"(?m)^(<<<<<<< |>>>>>>> )", src + hdr),
        "there are no conflict markers",
    )
    longest = max(
        (len(t) for t in re.findall(r"\w{2048,}", src + hdr + read(impl))),
        default=0,
    )
    check(
        not longest,
        "no identifier is longer than 2047 characters, which MSVC rejects"
        + (
            " (found %d; run API/hermes_sandbox/shorten_names.py)" % longest
            if longest
            else ""
        ),
    )

    for path in cfiles + [header, impl]:
        print("        %11d %s" % (os.path.getsize(path), os.path.basename(path)))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("root", help="the directory that contains API/hermes_sandbox")
    parser.add_argument(
        "--stack-size",
        type=int,
        default=256 * 1024,
        help="the -sSTACK_SIZE of the wasm build, in bytes",
    )
    args = parser.parse_args()

    host_src = read(
        os.path.join(args.root, "API/hermes_sandbox/HermesSandboxRuntime.cpp")
    )
    ext = os.path.join(args.root, "API/hermes_sandbox/external")
    imports, exports = host_imports(host_src), host_exports(host_src)
    version = bytecode_version(args.root)
    for variant in ("dbg", "opt"):
        check_variant(ext, variant, imports, exports, version, args.stack_size)

    print("== ABI")
    header, mirror = vtable_entries(args.root, host_src)
    check(
        header[: len(mirror)] == mirror,
        "the %d mirror entries are a prefix of the %d entries in hermes_abi.h"
        % (len(mirror), len(header)),
    )

    print(
        "\n"
        + (
            "All checks passed."
            if not failures
            else "%d checks failed." % len(failures)
        )
    )
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
