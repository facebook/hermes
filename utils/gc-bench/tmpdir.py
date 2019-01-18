#!/usr/bin/env python2.7
# Copyright (c) 2019-present, Facebook, Inc.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

"""This module brings the implementation of tempfile.TemporaryDirectory into python2"""

from __future__ import absolute_import, division, print_function, unicode_literals

from shutil import rmtree
from tempfile import mkdtemp


class TemporaryDirectory:
    def __init__(self, keep_tmp=False):
        self.keep_tmp = keep_tmp

    def __enter__(self):
        self.name = mkdtemp()
        return self.name

    def __exit__(self, exc, value, tb):
        if not self.keep_tmp:
            rmtree(self.name)
