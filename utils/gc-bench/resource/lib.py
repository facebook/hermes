#!/usr/bin/env python2.7
# Copyright (c) 2019-present, Facebook, Inc.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

"""Retrieves resources from the pex containing this python module"""

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import shutil

import pkg_resources


def resourceResolver(dir, resource):
    """Finds the resource given inside the zipped contents of the pex, copies
    the file into the given directory, and returns the name of the file.
    Also caches this file so multiple requests will return the same file name"""
    # Cache results so that files aren't copied multiple times.
    if resource not in resourceResolver.cachedFiles:
        # Copy the file to a temporary location. This is necessary because
        # resource_filename doesn't work with zip archives.
        with open(
            os.path.join(dir, os.path.basename(resource)), "w+b"
        ) as dst, pkg_resources.resource_stream(__name__, resource) as src:
            shutil.copyfileobj(src, dst)
            resourceResolver.cachedFiles[resource] = dst.name
    return resourceResolver.cachedFiles[resource]


resourceResolver.cachedFiles = {}
