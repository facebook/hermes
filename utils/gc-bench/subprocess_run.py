#!/usr/bin/env python2.7
# Copyright (c) 2019-present, Facebook, Inc.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.


import subprocess


def run(*args, **kwargs):
    """Behaves similarly to the Python 3.5+ Standard Library function:
    subprocess.run.
    """
    proc = subprocess.Popen(*args, **kwargs)

    try:
        out, err = proc.communicate()
    except:
        proc.kill()
        proc.wait()
        raise

    ret = proc.poll()

    return ret, out, err
