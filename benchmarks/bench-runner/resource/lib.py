# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

"""Retrieves resources from the pex containing this python module"""

import os
import shutil

from importlib import resources


def resourceResolver(dir, resource):
    """Finds the resource given inside the zipped contents of the pex, copies
    the file into the given directory, and returns the name of the file.
    Also caches this file so multiple requests will return the same file name"""
    # Cache results so that files aren't copied multiple times.
    if resource not in resourceResolver.cachedFiles:
        # Copy the file to a temporary location. This is necessary because
        # resource_filename doesn't work with zip archives.
        output_path = os.path.join(dir, resource)
        if not os.path.exists(os.path.dirname(output_path)):
            os.makedirs(os.path.dirname(output_path))
        ref = resources.files(__package__).joinpath('test-suites').joinpath(resource)
        with open(output_path, "w+b") as dst, ref.open('rb') as src:
            shutil.copyfileobj(src, dst)
            resourceResolver.cachedFiles[resource] = dst.name
    return resourceResolver.cachedFiles[resource]


resourceResolver.cachedFiles = {}
