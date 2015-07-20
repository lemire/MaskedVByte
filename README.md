MaskedVByte
===========

Fast decoder for VByte-compressed integers in C.

It assumes a recent Intel processor (e.g., haswell) but should work
with most x64 processors (supporting SSE instruction sets).

The code should build using most C compilers. The provided makefile
expects a Linux-like system.


Usage:

      make
      ./unit 

See example.c for an example.




