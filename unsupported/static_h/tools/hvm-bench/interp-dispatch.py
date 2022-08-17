# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals


def bench(lc, fc):
    res = 0.0
    while lc > 0.0:
        lc -= 1.0
        n = fc
        fact = n
        while n > 2.0:
            n -= 1.0
            fact *= n
        res += fact
    return res


print(bench(float(4e5), 100.0))
