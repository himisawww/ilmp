# ilmp
Integer Library for Multi-Precision arithmetic

Compilable in VS 2019.

 ilmp: code for creating ilmp.dll
 
ilmPi: code using ilmp.dll to do arithmetic and calculate Pi to arbitray precision in any base(radix).

- All computation are done in memory: for every 1 billion decimal digits of Pi, about 10 GiB physical memory are required.

- Single thread: currently no multi-thread support or GPU acceleration.

- Thread safe: all the functions provided in this library is thread safe and reenterable, as long as they do not operate on same resources(memory/object). You can write multi-thread program using this library as you want.

- Fair performance: This library has similar performance with GNU-MP, as well as many other software using GNU-MP (e.g. Mathematica).

However y-cruncher and prime95 is faster than this library by a factor of 4+ even in single thread, potentially due to usage of highly optimized variants of floating point FFT and SIMD(SSE, AVX, ...) acceleration.

On my computer(i7-7700HQ), calculating Pi to {10^6, 10^7, 10^8, 10^9} decimal digits will cost {0.47, 7.02, 111.7, 1604} seconds correspondingly.
