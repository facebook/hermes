#!/usr/bin/env python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import sys


LEAFNODE = "__LEAFNODE"


class Accumulation:
    """Groups allocations and counts them both per group and globally.
    The input is essentially:

         Function    Type                DedupKey    Size
         foo.js:1    data:string:header  42          8
         bar.js:1    data:string:header  42          8
         bar.js:2    data:string:header  1337        8

     and this is counted both globally (string header key 42 is used 2
     times, 1337 once), and per filename (generally: group). foo.js uses
     string header key 42, bar.js uses 42 and 1337.

     Once all the data is counted, total space used by each group is
     tallied, summed and itemized in a nested way, with three sums:

     * Private:   Data used only by this group
     * Shared:    All data used by this group
     * Amortized: Data used divided by number of users

     In the example above, for bar.js:

     * Private usage is 8 because #1337 is only used by bar.js
     * Shared usage is 16, because bar.js uses #42 and #1337
     * Amortized usage is 12, because bar.js owns all of #1337
       but splits #42 equally with foo.js (8/1+8/2 = 12).

     The final result is a JSON object essentially representing:

                             Pri Am  Shr
     bar.js                   8  10  24
        data                  8  10  24
           string             8  10  24
              header          8   8  16
              chars           0   2   8

    """

    def __init__(self):
        self.groups = {}
        self.counts = {}
        self.total = {}

    def insertLeaf(self, dic, obj):
        """Registers an allocation unit as being used by a group."""
        for key in obj["type"].split(":"):
            assert LEAFNODE not in dic
            dic = getOrAssignEmpty(dic, key)

        dic[LEAFNODE] = True

        if obj["dedupKey"] in dic:
            # All units from different sources should be the same size
            assert obj["size"] == dic[obj["dedupKey"]]

        dic[obj["dedupKey"]] = obj["size"]

    def insert(self, obj):
        """Updates both global and local counts for an allocation unit."""
        key = groupKey(obj)
        group = getOrAssignEmpty(self.groups, key)
        for use in obj["usage"]:
            use["type"] = str(use["type"])
            self.insertLeaf(group, use)
            getOrAssign(self.counts, (":" + use["type"], use["dedupKey"]), set).add(key)
            self.total[(":" + use["type"], use["dedupKey"])] = use["size"]

    def tally(self, groupName):
        """Sums up all uses for a group name."""
        assert groupName in self.groups
        group = self.groups[groupName]
        m = {}
        m["name"] = groupName
        m["usage"] = self.subtotal("", groupName, group)
        return m

    def subtotal(self, path, groupName, group):
        """Recursively calculates subtotals for each nested category."""
        total = {}
        total["sum"] = {}

        if LEAFNODE in group:
            private = shared = amortized = 0.0

            for dedupKey in group:
                if dedupKey == LEAFNODE:
                    continue
                count = len(self.counts[(path, dedupKey)])
                size = group[dedupKey]

                shared += size
                amortized += 1.0 * size / count
                if count == 1:
                    private += size

            total["sum"]["private"] = private
            total["sum"]["shared"] = shared
            total["sum"]["amortized"] = amortized
            return total

        total["itemized"] = {}
        for key in group:
            total["itemized"][key] = self.subtotal(
                path + ":" + key, groupName, group[key]
            )
            for k in total["itemized"][key]["sum"]:
                total["sum"][k] = (
                    total["sum"].get(k, 0) + total["itemized"][key]["sum"][k]
                )
        return total

    def getResult(self):
        """Get all groups in a single object that can be JSON serialized."""
        groups = []
        for group in self.groups.keys():
            groups.append(self.tally(group))
        return {"attribution": groups}


def groupKey(obj):
    if "file" in obj["location"]:
        return obj["location"]["file"] or "unknown"
    return str(obj["location"]["virtualOffset"])


def getOrAssign(dic, key, gen):
    if key in dic:
        return dic[key]

    obj = gen()
    dic[key] = obj
    return obj


def getOrAssignEmpty(dic, key):
    return getOrAssign(dic, key, lambda: {})


if __name__ == "__main__":
    acc = Accumulation()
    for line in sys.stdin:
        obj = json.loads(line)
        acc.insert(obj)
    sys.stdout.write(json.dumps(acc.getResult()))
    sys.stdout.write("\n")
