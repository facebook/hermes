@echo off
cmake .. -B "..\build_vs2019_x64" -G"Visual Studio 16" -A x64 -DASMJIT_TEST=1
