# NaiveOmpPi
Basic parallel solution with `#pragma omp parallel`. Without any intrinsics this solution again has some interesting interactions with compiler flags which mostly changes usage of scalar and vectorized instructions. While there are many possible combinations the two dimensions analyzed below is the usage of `-march=native` and `-ffast-math`. Testing below attempts to use the max number of threads, which is 16 for the test system. Unsurprisingly a large portion of the observations, including the differences with compiler flags, has overlap with the SerialPi solution.

## Default
As expected, this is the slowest of the configurations, with neither `-march` or `-ffast-math` specified. Compared to the equivalent SerialPi configuration, this is ~7.81x faster.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
NaiveOmpPi                                      10             1    439.837 ms 
```

High level statistics:

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][NaiveOmpPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][NaiveOmpPi]':

         76,731.97 msec task-clock:u              #   10.823 CPUs utilized          
                 0      context-switches:u        #    0.000 /sec                   
                 0      cpu-migrations:u          #    0.000 /sec                   
           440,183      page-faults:u             #    5.737 K/sec                  
   422,309,824,993      cycles:u                  #    5.504 GHz                      (62.47%)
         3,584,785      stalled-cycles-frontend:u #    0.00% frontend cycles idle     (62.45%)
       106,494,823      stalled-cycles-backend:u  #    0.03% backend cycles idle      (62.49%)
   546,446,349,676      instructions:u            #    1.29  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.51%)
    47,774,725,796      branches:u                #  622.618 M/sec                    (62.54%)
         5,950,978      branch-misses:u           #    0.01% of all branches          (62.59%)
     5,892,750,887      L1-dcache-loads:u         #   76.797 M/sec                    (62.57%)
        81,151,059      L1-dcache-load-misses:u   #    1.38% of all L1-dcache accesses  (62.49%)
   <not supported>      LLC-loads:u                                                 
   <not supported>      LLC-load-misses:u                                           

       7.089704304 seconds time elapsed

      76.428985000 seconds user
       0.299267000 seconds sys
```

Much like with `PiLib::SerialPi` the hot path is a bunch of scalar versions of vector operations.

```
$ perf annotate PiLib::NaiveOmpPi --stdio
 Percent |      Source code & Disassembly of PiBench for cycles:u (297109 samples, percent: local period)
---------------------------------------------------------------------------------------------------------
...
    0.00 :   1d430:  pxor   %xmm0,%xmm0
         : 29    innerSum += 4.0 / (1.0 + x * x);
    0.00 :   1d434:  movapd %xmm4,%xmm2
         : 28    const double x = (i + 0.5) * step;
    0.00 :   1d438:  cvtsi2sd %rdx,%xmm0
         : 26    for (int64_t i = id; i < iterations; i += threads)
    0.05 :   1d43d:  add    %rax,%rdx
         : 28    const double x = (i + 0.5) * step;
    0.00 :   1d440:  addsd  %xmm6,%xmm0
    0.00 :   1d444:  mulsd  %xmm3,%xmm0
         : 29    innerSum += 4.0 / (1.0 + x * x);
    5.98 :   1d448:  mulsd  %xmm0,%xmm0
    6.23 :   1d44c:  addsd  %xmm5,%xmm0
    0.01 :   1d450:  divsd  %xmm0,%xmm2
   78.54 :   1d454:  addsd  %xmm2,%xmm1
         : 26    for (int64_t i = id; i < iterations; i += threads)
    9.19 :   1d458:  cmp    %rdx,%rbx
    0.00 :   1d45b:  jg     1d430 <PiLib::NaiveOmpPi(long, unsigned long) [clone ._omp_fn.0]+0x60>
...
```

## Fast-Math
Fast math on it's own doesn't appear to change much.

```
$ objdump -d out/build/linux-gcc-profile/PiLib/CMakeFiles/PiLib.dir/NaiveOmpPi.cpp.o
  6d:	48 01 c2             	add    %rax,%rdx
  70:	f2 0f 58 c6          	addsd  %xmm6,%xmm0
  74:	f2 0f 59 c3          	mulsd  %xmm3,%xmm0
  78:	f2 0f 59 c0          	mulsd  %xmm0,%xmm0
  7c:	f2 0f 58 c5          	addsd  %xmm5,%xmm0
  80:	f2 0f 5e d0          	divsd  %xmm0,%xmm2
  84:	f2 0f 58 ca          	addsd  %xmm2,%xmm1
  88:	48 39 d3             	cmp    %rdx,%rbx
  8b:	7f d3                	jg     60 <_ZN5PiLib10NaiveOmpPiElm._omp_fn.0+0x60>
```

## Native
Using `-march=native` on it's own does improve performance.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
NaiveOmpPi                                      10             1     418.98 ms 
```

Statistics:

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][NaiveOmpPi]"
...
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][NaiveOmpPi]':

         75,675.08 msec task-clock:u              #   10.973 CPUs utilized          
                 0      context-switches:u        #    0.000 /sec                   
                 0      cpu-migrations:u          #    0.000 /sec                   
           440,183      page-faults:u             #    5.817 K/sec                  
   416,998,509,118      cycles:u                  #    5.510 GHz                      (62.55%)
        79,367,589      stalled-cycles-frontend:u #    0.02% frontend cycles idle     (62.53%)
        22,743,556      stalled-cycles-backend:u  #    0.01% backend cycles idle      (62.48%)
   414,273,409,658      instructions:u            #    0.99  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.44%)
    47,784,926,503      branches:u                #  631.449 M/sec                    (62.46%)
         3,800,018      branch-misses:u           #    0.01% of all branches          (62.52%)
     5,786,188,135      L1-dcache-loads:u         #   76.461 M/sec                    (62.55%)
        75,211,176      L1-dcache-load-misses:u   #    1.30% of all L1-dcache accesses  (62.57%)
   <not supported>      LLC-loads:u                                                 
   <not supported>      LLC-load-misses:u                                           

       6.896325723 seconds time elapsed

      75.358637000 seconds user
       0.315910000 seconds sys
...
```

Mostly scalar instructions on the hot path still:

```
$ perf annotate PiLib::NaiveOmpPi --stdio
 Percent |      Source code & Disassembly of PiBench for cycles:u (293694 samples, percent: local period)
---------------------------------------------------------------------------------------------------------
...
         : 29    const double x = (i + 0.5) * step;
    0.00 :   1d700:  vcvtsi2sd %rdx,%xmm6,%xmm0
    0.00 :   1d705:  vaddsd %xmm5,%xmm0,%xmm0
         : 26    for (int64_t i = id; i < iterations; i += threads)
    0.00 :   1d709:  add    %rax,%rdx
         : 28    const double x = (i + 0.5) * step;
    0.00 :   1d70c:  vmulsd %xmm2,%xmm0,%xmm0
         : 29    innerSum += 4.0 / (1.0 + x * x);
    0.02 :   1d710:  vfmadd132sd %xmm0,%xmm4,%xmm0
    1.95 :   1d715:  vdivsd %xmm0,%xmm3,%xmm0
   84.50 :   1d719:  vaddsd %xmm0,%xmm1,%xmm1
         : 26    for (int64_t i = id; i < iterations; i += threads)
   13.53 :   1d71d:  cmp    %rdx,%rbx
    0.00 :   1d720:  jg     1d700 <PiLib::NaiveOmpPi(long, unsigned long) [clone ._omp_fn.0]+0x60>
...
```

## Both

We finally get to some rather good performance for a simple solution with both `-march` and `-ffast-math`, though we are getting less pronounced benefit from multithreading, with only ~1.99x performance improvement from the same configuration with SerialPi.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
NaiveOmpPi                                      10             1    415.363 ms 
```

Statistics:

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][NaiveOmpPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][NaiveOmpPi]':

         75,198.44 msec task-clock:u              #   10.989 CPUs utilized          
                 0      context-switches:u        #    0.000 /sec                   
                 0      cpu-migrations:u          #    0.000 /sec                   
           440,183      page-faults:u             #    5.854 K/sec                  
   414,096,485,620      cycles:u                  #    5.507 GHz                      (62.45%)
         3,164,084      stalled-cycles-frontend:u #    0.00% frontend cycles idle     (62.53%)
       105,501,827      stalled-cycles-backend:u  #    0.03% backend cycles idle      (62.58%)
   414,344,804,034      instructions:u            #    1.00  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.63%)
    47,922,040,844      branches:u                #  637.274 M/sec                    (62.58%)
         2,524,893      branch-misses:u           #    0.01% of all branches          (62.51%)
     5,794,749,303      L1-dcache-loads:u         #   77.059 M/sec                    (62.42%)
        77,301,240      L1-dcache-load-misses:u   #    1.33% of all L1-dcache accesses  (62.42%)
   <not supported>      LLC-loads:u                                                 
   <not supported>      LLC-load-misses:u                                           

       6.842921991 seconds time elapsed

      74.904663000 seconds user
       0.292033000 seconds sys
```

And importantly, reliance on actual packed vector instructions!

```
$ perf annotate PiLib::NaiveOmpPi --stdio
 Percent |      Source code & Disassembly of PiBench for cycles:u (291858 samples, percent: local period)
---------------------------------------------------------------------------------------------------------
...
         : 29    const double x = (i + 0.5) * step;
    0.00 :   1d6b0:  vcvtsi2sd %rdx,%xmm6,%xmm0
    0.00 :   1d6b5:  vaddsd %xmm5,%xmm0,%xmm0
         : 26    for (int64_t i = id; i < iterations; i += threads)
    0.00 :   1d6b9:  add    %rax,%rdx
         : 28    const double x = (i + 0.5) * step;
    0.00 :   1d6bc:  vmulsd %xmm2,%xmm0,%xmm0
         : 29    innerSum += 4.0 / (1.0 + x * x);
    0.00 :   1d6c0:  vfmadd132sd %xmm0,%xmm4,%xmm0
    0.79 :   1d6c5:  vdivsd %xmm0,%xmm3,%xmm0
   85.12 :   1d6c9:  vaddsd %xmm0,%xmm1,%xmm1
         : 26    for (int64_t i = id; i < iterations; i += threads)
   14.09 :   1d6cd:  cmp    %rdx,%rbx
    0.00 :   1d6d0:  jg     1d6b0 <PiLib::NaiveOmpPi(long, unsigned long) [clone ._omp_fn.0]+0x60>
...
```
