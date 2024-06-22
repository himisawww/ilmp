# Integer Library for Multi-Precision arithmetic

File structure:

 -  ilmp: code for creating shared library `ilmp.dll` for Windows or `libilmp.so` for Linux.
 
 - ilmPi: code using the shared library to do arithmetic and calculate Pi to arbitray precision in any base(radix).

# Build

- On Windows: Compilable in Visual Studio 2019.

- On Linux: `make`.
  
  Pre-requisites `nasm` and `g++`. Tested with `nasm --version` 2.14.02 and `g++ --version` 10.2.0.

# Features & Introduction

- All computation are done in memory: for every 1 billion decimal digits of Pi, about 10 GiB physical memory is required.

- Single thread: currently no multi-thread support or GPU acceleration.

- Thread safe: all the functions provided in this library is thread safe and reenterable, as long as they do not operate on same resources(memory/object). You can write multi-thread program using this library as you want.

- Full integer: All the multiprecision arithmetics are done internally with integer instructions (, basically, on `uint64`). Free of mistakes introduced by building up cut-off errors of floating point operations.
 
- Fair performance: This library has similar performance with GNU-MP, as well as many other software using GNU-MP (e.g. Mathematica).

(Note: However y-cruncher and prime95 is faster than this library by a factor of 4+ even in single thread, potentially due to usage of highly optimized variants of floating point FFT and SIMD(SSE, AVX, ...) acceleration.)

On my computer(i7-7700HQ), calculating Pi to {10^6, 10^7, 10^8, 10^9} decimal digits will cost {0.47, 7.02, 111.7, 1604} seconds correspondingly.

Functions to compute E, log(2), and Prime-Buenos-Aires-constant (see https://doi.org/10.1080/00029890.2019.1530554 ) are also implemented. One can run this program to calculate these constants instead of Pi.

An example.cpp exists in ilmPi directory, one can easily modify the code to run this example and learn how to use this library.

Example output is as follows:

```
Integer division may result into floating point values.
      a = 25
      b = 7
  a / b = 3.57142857143
Unless the division is exact
  where the result will also be an integer.
      a = 42
      b = 7
  a / b = 6
This is same for sqrt
     b  = 7
sqrt(b) = 2.64575131106
     b  = 81
sqrt(b) = 9

Number class can handle arbitrary precision arithmetic.
      a = 123456789012345678901234567890
      b = 233333333333333333333333333333
  a + b = 356790122345679012234567901223
  a - b = -109876544320987654432098765443
  a * b = 28806584102880658410288065840958847736995884773699588477370
  a / b = 0.5291005243386243381481481481007558578919

floating point initialization may not be accurate...
      a = 0.69999999
      b = 0.200000003
  a + b = 0.89999999
using string initialization can avoid that inaccuracy.
      a = 0.7000000000
      b = 0.20000000000
  a + b = 0.9000000000

Number class supports a very large value range.
Note:   To avoid confusion when i/o in radices larger than 10,
        we use *^ rather than 'e' as the floating point exponent marker,
        e.g. speed of light is 3*^8, not 3e8, m/s.
 Max supported = 2.98363890926236297245314539471301204537964168*^2711437152599295
 Min supported = 3.35161200940105487500326727912842813360915276*^-2711437152599296
   Overflow    = #inf
  -Overflow    = -#inf
  Underflow    = 0
Indeterminate  = #nan

Some other examples:
 3 ^ 200 = 265613988875874769338781322035779626829233452653394495974574961739092490901302182994384699044001
 sqrt(2) = 1.41421356237309504880168872420969807856967187537694807317667973799073247846210703885038753432764157274
      Pi = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170680
       E = 2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274

Number class can automatically handle both integer and floating point values.
a string containing either exponent marker *^ or precision marker ` will be considered as floating point.
hybrid operations will automatically result in correct precision.
      a = 666
      b = 1.*^-30
  a + b = 666.000000000000000000000000000001
      c = 99999999999999999999999999999
      d = 123.45678900
  c * d = 1.2345678900*^31
What will happen if you calculate 1 - 1*^-10000000000000 ?
Please just don't do that.

Integer class only supports integers, but has better performance.
  Number: sum(10000000) = 50000005000000 in 0.544581s
 Integer: sum(10000000) = 50000005000000 in 0.270887s
       ...  but they are all slower than built-in types.
     int: sum(10000000) = 50000005000000 in 0.0028664s
```
