#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
import collections
import itertools
import re
import sys


def main(argv):
    sections = collections.defaultdict(list)
    toc = read_toc("docs/quickref_toc.txt")
    grab_sections("test/DocTests.java", sections)
    grab_sections("test/jni/doc_tests.cpp", sections)

    missing_code = toc.keys() - sections.keys()
    if missing_code:
        raise Exception(f"Missing code for sections: {' '.join(missing_code)}")
    missing_toc = sections.keys() - toc.keys()
    if missing_toc:
        raise Exception(f"Missing toc for sections: {' '.join(missing_toc)}")

    with open("docs/quickref.md", "w") as handle:
        handle.write("# Quick Reference\n")
        for section in toc:
            name = toc[section].strip()
            handle.write(f"- [{name}](#{anchor(name)})\n")
        for section in toc:
            render_section(handle, section, toc[section], sections[section])


def anchor(title):
    anchor = title.lower()
    anchor = re.sub(" ", "-", anchor)
    anchor = re.sub(r"[^-\w]", "", anchor)
    return anchor


def read_toc(fname):
    with open(fname) as handle:
        return collections.OrderedDict(line.split(" ", 1) for line in handle)


def grab_sections(fname, sections):
    extension = fname.split(".")[1]
    active_block = None

    with open(fname) as handle:
        for lnum, line in enumerate(handle):
            lnum += 1
            if line.strip().endswith("// END"):
                active_block = None
                continue
            m = re.search(r"// SECTION (\w+)$", line)
            if m:
                if active_block is not None:
                    raise Exception(f"Nested section at {fname}:{lnum}")
                active_group = m.group(1)
                active_block = []
                sections[active_group].append((extension, active_block))
                continue
            if line.strip().endswith(" MARKDOWN"):
                if active_block is None:
                    raise Exception(f"Orphaned markdown at {fname}:{lnum}")
                active_block = []
                sections[active_group].append(("md", active_block))
                continue
            if active_block is not None:
                active_block.append(line)


def render_section(out, name, title, blocks):
    out.write(f"## {title}")
    for syntax, lines in blocks:
        if not lines:
            # This happens with Markdown-first sections
            continue
        if syntax != "md":
            lines = itertools.chain(
                [f"```{syntax}\n"],
                lines,
                ["```\n"],
            )
        for line in lines:
            out.write(line)
    out.write("\n\n")


if __name__ == "__main__":
    sys.exit(main(sys.argv))
