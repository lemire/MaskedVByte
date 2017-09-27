MaskedVByte
===========
[![Build Status](https://travis-ci.org/lemire/MaskedVByte.png)](https://travis-ci.org/lemire/MaskedVByte)

Fast decoder for VByte-compressed integers in C.

It includes fast differential coding.

It assumes a recent Intel processor (e.g., haswell) but should work
with most x64 processors (supporting SSE instruction sets).

The code should build using most standard-compliant modern C compilers (C99). The provided makefile
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

* Daniel Lemire, Nathan Kurz, Christoph Rupp, Stream VByte: Faster Byte-Oriented Integer Compression, Information Processing Letters (to appear) https://arxiv.org/abs/1709.08990
* Jeff Plaisance, Nathan Kurz, Daniel Lemire, Vectorized VByte Decoding,  International Symposium on Web Algorithms 2015, 2015. http://arxiv.org/abs/1503.07387


See also
------------

* SIMDCompressionAndIntersection: A C++ library to compress and intersect sorted lists of integers using SIMD instructions https://github.com/lemire/SIMDCompressionAndIntersection
* The FastPFOR C++ library : Fast integer compression https://github.com/lemire/FastPFor
* High-performance dictionary coding https://github.com/lemire/dictionary
* LittleIntPacker: C library to pack and unpack short arrays of integers as fast as possible https://github.com/lemire/LittleIntPacker
* The SIMDComp library: A simple C library for compressing lists of integers using binary packing https://github.com/lemire/simdcomp
* StreamVByte: Fast integer compression in C using the StreamVByte codec https://github.com/lemire/streamvbyte
* CSharpFastPFOR: A C#  integer compression library  https://github.com/Genbox/CSharpFastPFOR
* JavaFastPFOR: A java integer compression library https://github.com/lemire/JavaFastPFOR
* Encoding: Integer Compression Libraries for Go https://github.com/zhenjl/encoding
* FrameOfReference is a C++ library dedicated to frame-of-reference (FOR) compression: https://github.com/lemire/FrameOfReference
* libvbyte: A fast implementation for varbyte 32bit/64bit integer compression https://github.com/cruppstahl/libvbyte
* TurboPFor is a C library that offers lots of interesting optimizations. Well worth checking! (GPL license) https://github.com/powturbo/TurboPFor
* Oroch is a C++ library that offers a usable API (MIT license) https://github.com/ademakov/Oroch


