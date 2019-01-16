#!/usr/bin/env python2.7

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
