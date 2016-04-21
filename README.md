MaskedVByte
===========
[![Build Status](https://travis-ci.org/lemire/MaskedVByte.png)](https://travis-ci.org/lemire/MaskedVByte)

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

```C
size_t compsize = vbyte_encode(datain, N, compressedbuffer); // encoding
// here the result is stored in compressedbuffer using compsize bytes
size_t compsize2 = masked_vbyte_decode(compressedbuffer, recovdata, N); // decoding (fast)
```

Interesting applications 
-----------------------

Greg Bowyer has integrated Masked VByte into Lucene, for higher speeds :

https://github.com/GregBowyer/lucene-solr/tree/intrinsics


Reference
-------------

Jeff Plaisance, Nathan Kurz, Daniel Lemire, Vectorized VByte Decoding, 
International Symposium on Web Algorithms 2015, 2015.
http://arxiv.org/abs/1503.07387


See also
------------

* libvbyte: A fast implementation for varbyte 32bit/64bit integer compression https://github.com/cruppstahl/libvbyte
* https://github.com/lemire/streamvbyte
* https://github.com/lemire/simdcomp


