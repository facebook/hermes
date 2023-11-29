# secdiff

Section-by-section interactive diff tool using Python's curses.

Input:
    A file with some number of sections that have lines delimiting the sections.

Usage:

    secdiff.py [-h] -s SEPARATOR filename

Provide a separator marker with '-s' and the file to diff the sections of.

Keybindings:
* l/h: Go to the next/prev pair of sections to diff.
* j/k: Scroll down/up in the current diff.
