MaskedVByte
===========

Fast decoder for VByte-compressed integers in C.

It includes fast differential coding.

It assumes a recent Intel processor (e.g., haswell) but should work
with most x64 processors (supporting SSE instruction sets).

The code should build using most C compilers. The provided makefile
expects a Linux-like system.

Usage:

      make
      ./unit 

See example.c for an example.

Short code sample:

        size_t compsize = vbyte_encode(datain, N, compressedbuffer); // encoding
        // here the result is stored in compressedbuffer using compsize bytes
        size_t compsize2 = masked_vbyte_decode(compressedbuffer, recovdata, N); // decoding (fast)


Reference
-------------

Jeff Plaisance, Nathan Kurz, Daniel Lemire, Vectorized VByte Decoding, 
International Symposium on Web Algorithms 2015, 2015.


