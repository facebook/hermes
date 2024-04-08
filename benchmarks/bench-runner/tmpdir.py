# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

"""This module brings the implementation of tempfile.TemporaryDirectory into python2"""

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
