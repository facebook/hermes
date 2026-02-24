#!/bin/bash

ctags -x --c++-kinds=f $1 | sort -k3
