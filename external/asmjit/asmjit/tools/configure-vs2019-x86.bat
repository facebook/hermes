@echo off
cmake .. -B "..\build_vs2019_x86" -G"Visual Studio 16" -A Win32 -DASMJIT_TEST=1
