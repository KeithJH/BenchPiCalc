# SerialPi
Basic serial solution without anything fancy, so likely to be the slowest solution to use as a baseline. There are, however, some interesting interactions with compiler flags which mostly changes usage of scalar and vectorized instructions. While there are many possible combinations the two dimensions analyzed below is the usage of `-march=native` and `-ffast-math`.

## Default
As expected, this is the slowest of the configurations, with neither `-march` or `-ffast-math` specified.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SerialPi                                         1             1     3.43423 s
```

Nothing looks out of the ordinary with a quick `perf stat` as branches are well predicted (there's only really the loop) and we aren't really processing data from memory.

```
$ perf stat ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 "[!benchmark][SerialPi]"
...
Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [!benchmark][SerialPi]':

          9,264.30 msec task-clock:u              #    1.000 CPUs utilized          
                 0      context-switches:u        #    0.000 /sec                   
                 0      cpu-migrations:u          #    0.000 /sec                   
           440,141      page-faults:u             #   47.509 K/sec                  
    50,093,796,728      cycles:u                  #    5.407 GHz                      (62.44%)
         2,719,456      stalled-cycles-frontend:u #    0.01% frontend cycles idle     (62.47%)
       105,716,245      stalled-cycles-backend:u  #    0.21% backend cycles idle      (62.52%)
   111,769,520,514      instructions:u            #    2.23  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.56%)
    11,469,175,739      branches:u                #    1.238 G/sec                    (62.57%)
         5,126,395      branch-misses:u           #    0.04% of all branches          (62.52%)
     5,757,833,717      L1-dcache-loads:u         #  621.508 M/sec                    (62.49%)
        78,098,737      L1-dcache-load-misses:u   #    1.36% of all L1-dcache accesses  (62.44%)
   <not supported>      LLC-loads:u                                                 
   <not supported>      LLC-load-misses:u                                           

       9.264622253 seconds time elapsed

       8.960447000 seconds user
       0.304015000 seconds sys
```

As expected, nearly all the cycles are spent inside `PiLib::SerialPi`, with the next next runner-ups looking to be related to timing.

```
$ perf report --stdio
...
# Overhead  Command  Shared Object         Symbol                                                                           
# ........  .......  ....................  .................................................................................
#
    76.86%  PiBench  PiBench               [.] PiLib::SerialPi
     4.94%  PiBench  [vdso]                [.] 0x00000000000006e8
     4.73%  PiBench  libstdc++.so.6.0.30   [.] std::chrono::_V2::steady_clock::now
     3.69%  PiBench  libc.so.6             [.] clock_gettime@@GLIBC_2.17
...
```

Drilling in further, the bulk of the work is unsurprisingly in the math of accumulating the sum in each iteration. Unfortunately the math is all scalar versions, even if the vector registers are being used, as the instructions end in `sd` for "scalar double".

```
$ perf annotate PiLib::SerialPi --stdio
 Percent |      Source code & Disassembly of PiBench for cycles:u (28529 samples, percent: local period)
--------------------------------------------------------------------------------------------------------
...
         : 15    sum += 4.0 / (1.0 + x * x);
    0.05 :   1b009:  mulsd  %xmm1,%xmm1
   20.42 :   1b00d:  addsd  %xmm4,%xmm1
    0.00 :   1b011:  divsd  %xmm1,%xmm3
   42.96 :   1b015:  addsd  %xmm3,%xmm2
         : 10    for (int64_t i = 0; i < iterations; i++)
   36.57 :   1b019:  cmp    %rax,%rdi
    0.00 :   1b01c:  jne    1aff0 <PiLib::SerialPi(long)+0x40>
...
```

## Fast-Math
Fast math on it's own doesn't appear to change anything.

```
$ objdump out/build/linux-gcc-profile/PiLib/libPiLib.a -d | grep sd
   8:	f2 0f 10 25 00 00 00 	movsd  0x0(%rip),%xmm4        # 10 <_ZN5PiLib8SerialPiEl+0x10>
  10:	f2 48 0f 2a cf       	cvtsi2sd %rdi,%xmm1
  19:	f2 0f 5e c1          	divsd  %xmm1,%xmm0
  22:	f2 0f 10 35 00 00 00 	movsd  0x0(%rip),%xmm6        # 2a <_ZN5PiLib8SerialPiEl+0x2a>
  2a:	f2 0f 10 2d 00 00 00 	movsd  0x0(%rip),%xmm5        # 32 <_ZN5PiLib8SerialPiEl+0x32>
  48:	f2 48 0f 2a c8       	cvtsi2sd %rax,%xmm1
  51:	f2 0f 58 ce          	addsd  %xmm6,%xmm1
  55:	f2 0f 59 c8          	mulsd  %xmm0,%xmm1
  59:	f2 0f 59 c9          	mulsd  %xmm1,%xmm1
  5d:	f2 0f 58 cc          	addsd  %xmm4,%xmm1
  61:	f2 0f 5e d9          	divsd  %xmm1,%xmm3
  65:	f2 0f 58 d3          	addsd  %xmm3,%xmm2
  6e:	f2 0f 59 c2          	mulsd  %xmm2,%xmm0

```

## Native
Using `-march=native` on it's own does improve performance.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SerialPi                                         1             1     2.21348 s 
```

Most metrics are the same, though there is a decrease in instructions per cycle.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 "[!benchmark][SerialPi]"
...
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [!benchmark][SerialPi]':

          6,723.54 msec task-clock:u              #    1.000 CPUs utilized          
                 0      context-switches:u        #    0.000 /sec                   
                 0      cpu-migrations:u          #    0.000 /sec                   
           440,138      page-faults:u             #   65.462 K/sec                  
    35,967,717,948      cycles:u                  #    5.350 GHz                      (62.51%)
         2,763,711      stalled-cycles-frontend:u #    0.01% frontend cycles idle     (62.52%)
       109,325,279      stalled-cycles-backend:u  #    0.30% backend cycles idle      (62.52%)
    40,227,194,997      instructions:u            #    1.12  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.53%)
     4,414,769,955      branches:u                #  656.614 M/sec                    (62.52%)
         2,932,147      branch-misses:u           #    0.07% of all branches          (62.47%)
     5,642,266,572      L1-dcache-loads:u         #  839.180 M/sec                    (62.46%)
        72,663,951      L1-dcache-load-misses:u   #    1.29% of all L1-dcache accesses  (62.46%)
   <not supported>      LLC-loads:u                                                 
   <not supported>      LLC-load-misses:u                                           

       6.724447120 seconds time elapsed

       6.479510000 seconds user
       0.243981000 seconds sys
...
```

There are still many scalar instructions taking up most of the cycles, however, there are some actual vector instructions ending with `pd` ("packed double") including a `fmadd` for fused multiply and add.

```
$ perf annotate PiLib::SerialPi --stdio
Percent |      Source code & Disassembly of PiBench for cycles:u (17717 samples, percent: local period)
--------------------------------------------------------------------------------------------------------
         : 15    sum += 4.0 / (1.0 + x * x);
    0.00 :   1b0a1:  vfmadd132pd %zmm1,%zmm8,%zmm1
    1.36 :   1b0a7:  vdivpd %zmm1,%zmm7,%zmm1
    0.00 :   1b0ad:  vunpckhpd %xmm1,%xmm1,%xmm4
    0.00 :   1b0b1:  vextractf64x2 $0x1,%ymm1,%xmm3
    0.00 :   1b0b8:  vaddsd %xmm1,%xmm13,%xmm2
    4.75 :   1b0bc:  vextractf64x4 $0x1,%zmm1,%ymm1
    0.00 :   1b0c3:  vaddsd %xmm4,%xmm2,%xmm2
   22.35 :   1b0c7:  vaddsd %xmm3,%xmm2,%xmm2
   37.11 :   1b0cb:  vunpckhpd %xmm3,%xmm3,%xmm3
    0.00 :   1b0cf:  vaddsd %xmm3,%xmm2,%xmm2
   24.33 :   1b0d3:  vunpckhpd %xmm1,%xmm1,%xmm3
    0.00 :   1b0d7:  vaddsd %xmm1,%xmm2,%xmm2
    6.15 :   1b0db:  vextractf64x2 $0x1,%ymm1,%xmm1
    0.00 :   1b0e2:  vaddsd %xmm3,%xmm2,%xmm2
    0.48 :   1b0e6:  vaddsd %xmm1,%xmm2,%xmm2
    0.73 :   1b0ea:  vunpckhpd %xmm1,%xmm1,%xmm1
    0.00 :   1b0ee:  vaddsd %xmm1,%xmm2,%xmm13
         : 10    for (int64_t i = 0; i < iterations; i++)
    2.74 :   1b0f2:  cmp    %rax,%rcx
    0.00 :   1b0f5:  jne    1b080 <PiLib::SerialPi(long)+0x80>
```

## Both

We finally get to some rather good performance for a simple solution with both `-march` and `-ffast-math`.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SerialPi                                         1             1    831.513 ms
```

With perhaps gaining some instructions per cycle back.
```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 "[!benchmark][SerialPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [!benchmark][SerialPi]':

          3,936.98 msec task-clock:u              #    1.000 CPUs utilized          
                 0      context-switches:u        #    0.000 /sec                   
                 0      cpu-migrations:u          #    0.000 /sec                   
           420,142      page-faults:u             #  106.717 K/sec                  
    20,354,103,205      cycles:u                  #    5.170 GHz                      (62.51%)
         2,467,222      stalled-cycles-frontend:u #    0.01% frontend cycles idle     (62.51%)
        87,636,125      stalled-cycles-backend:u  #    0.43% backend cycles idle      (62.51%)
    25,533,521,687      instructions:u            #    1.25  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.51%)
     4,317,423,623      branches:u                #    1.097 G/sec                    (62.51%)
         7,771,863      branch-misses:u           #    0.18% of all branches          (62.48%)
     5,559,869,732      L1-dcache-loads:u         #    1.412 G/sec                    (62.48%)
        71,591,296      L1-dcache-load-misses:u   #    1.29% of all L1-dcache accesses  (62.48%)
   <not supported>      LLC-loads:u                                                 
   <not supported>      LLC-load-misses:u                                           

       3.937230757 seconds time elapsed

       3.601089000 seconds user
       0.336101000 seconds sys
```

But most importantly, more reliance on actual vector instructions!

```
$ perf annotate PiLib::SerialPi --stdio
 Percent |      Source code & Disassembly of PiBench for cycles:u (6668 samples, percent: local period)
-------------------------------------------------------------------------------------------------------
...
         : 15    sum += 4.0 / (1.0 + x * x);
    5.91 :   1b081:  vfmadd132pd %zmm1,%zmm7,%zmm1
    1.14 :   1b087:  vdivpd %zmm1,%zmm6,%zmm1
   83.16 :   1b08d:  vaddpd %zmm1,%zmm3,%zmm3
         : 10    for (int64_t i = 0; i < iterations; i++)
    9.78 :   1b093:  cmp    %rax,%rcx
    0.00 :   1b096:  jne    1b060 <PiLib::SerialPi(long)+0x80>
...
```
