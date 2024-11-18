# ForOmpPi
Parallel solution using an OpenMP parallel for loop with a reduction clause for sum increment instead of a temporary vector of partial sums that get reduced.

# Default
Since the reduction step is such a small part of the overall algorithm, and since it gets optimized fairly well in most cases, there isn't really a performance difference compared to the "naive" solution.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][ForOmpPi]"
...
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
ForOmpPi                                        10             1     447.34 ms 
...
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][ForOmpPi]':

         79,008.57 msec task-clock                       #   10.553 CPUs utilized             
               617      context-switches                 #    7.809 /sec                      
                 1      cpu-migrations                   #    0.013 /sec                      
           420,188      page-faults                      #    5.318 K/sec                     
   435,629,266,046      cycles                           #    5.514 GHz                         (71.43%)
     8,452,109,082      stalled-cycles-frontend          #    1.94% frontend cycles idle        (71.43%)
   550,161,465,674      instructions                     #    1.26  insn per cycle            
                                                  #    0.02  stalled cycles per insn     (71.43%)
    48,112,656,269      branches                         #  608.955 M/sec                       (71.44%)
        85,770,333      branch-misses                    #    0.18% of all branches             (71.43%)
     6,570,785,755      L1-dcache-loads                  #   83.165 M/sec                       (71.42%)
       127,180,445      L1-dcache-load-misses            #    1.94% of all L1-dcache accesses   (71.42%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

       7.487118945 seconds time elapsed

      78.465237000 seconds user
       0.543091000 seconds sys
```

## Fast-math
Keeping with the trend, fast math on it's own doesn't seem to change anything.

## Native
Since the sum reduction pattern is described in the OpenMP pragmas, it appears that setting `-march=native` allows smarter optimizations than other OpenMP solutions.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][ForOmpPi]"
...
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
ForOmpPi                                        10             1    154.168 ms 
...
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][ForOmpPi]':

         29,561.51 msec task-clock                       #    6.997 CPUs utilized             
             2,719      context-switches                 #   91.978 /sec                      
                 2      cpu-migrations                   #    0.068 /sec                      
           420,187      page-faults                      #   14.214 K/sec                     
   162,984,791,270      cycles                           #    5.513 GHz                         (71.40%)
     8,052,760,989      stalled-cycles-frontend          #    4.94% frontend cycles idle        (71.39%)
   156,361,196,416      instructions                     #    0.96  insn per cycle            
                                                  #    0.05  stalled cycles per insn     (71.41%)
     9,184,413,060      branches                         #  310.688 M/sec                       (71.44%)
        77,392,166      branch-misses                    #    0.84% of all branches             (71.46%)
     6,413,115,316      L1-dcache-loads                  #  216.941 M/sec                       (71.48%)
       119,055,770      L1-dcache-load-misses            #    1.86% of all L1-dcache accesses   (71.45%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

       4.224654964 seconds time elapsed

      29.069926000 seconds user
       0.485016000 seconds sys
```

Which of course means that actual packed vector instructions are used! Even fused-multiply add, which is a bit strange without enabling fast math, perhaps due to shenanigans with multiple vector widths.
```
  4.26 │ c0:   vmovdqa64     %zmm5,%zmm0
  4.26 │       vpaddq        %zmm7,%zmm5,%zmm5
       │     for (int64_t i = 0; i < iterations; i++)
       │     {
       │     const double x = (i + 0.5) * step;
  4.27 │       inc           %rcx
  4.23 │       vcvtqq2pd     %zmm0,%zmm0
  4.13 │       vaddpd        %zmm10,%zmm0,%zmm0
  4.38 │       vmulpd        %zmm11,%zmm0,%zmm0
       │     sum += 4.0 / (1.0 + x * x);
  3.87 │       vfmadd132pd   %zmm0,%zmm9,%zmm0
  3.92 │       vdivpd        %zmm0,%zmm8,%zmm0
  3.96 │       valignq       $0x3,%ymm0,%ymm0,%ymm2
  3.82 │       vaddsd        %xmm0,%xmm4,%xmm1
  3.89 │       vunpckhpd     %xmm0,%xmm0,%xmm3
  3.87 │       vaddsd        %xmm3,%xmm1,%xmm1
  4.31 │       vextractf64x2 $0x1,%ymm0,%xmm3
  4.31 │       vextractf64x4 $0x1,%zmm0,%ymm0
  4.24 │       vaddsd        %xmm3,%xmm1,%xmm1
  4.29 │       vaddsd        %xmm2,%xmm1,%xmm1
  4.22 │       vunpckhpd     %xmm0,%xmm0,%xmm2
  4.30 │       vaddsd        %xmm0,%xmm1,%xmm1
  4.26 │       vaddsd        %xmm2,%xmm1,%xmm1
  4.26 │       vextractf64x2 $0x1,%ymm0,%xmm2
  4.22 │       valignq       $0x3,%ymm0,%ymm0,%ymm0
  4.19 │       vaddsd        %xmm2,%xmm1,%xmm1
  4.35 │       vaddsd        %xmm0,%xmm1,%xmm4
  4.19 │       cmp           %rcx,%rsi
       │     ↑ jne           c0
```

## Both
Having both `-march=native` and `-ffast-math` eeks out even more performance.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][ForOmpPi]"
...
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
ForOmpPi                                        10             1    103.906 ms 
...
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][ForOmpPi]':

         20,878.03 msec task-clock                       #    5.668 CPUs utilized             
             1,738      context-switches                 #   83.245 /sec                      
                 3      cpu-migrations                   #    0.144 /sec                      
           420,189      page-faults                      #   20.126 K/sec                     
   115,118,610,909      cycles                           #    5.514 GHz                         (71.41%)
     8,019,558,946      stalled-cycles-frontend          #    6.97% frontend cycles idle        (71.45%)
    78,574,599,735      instructions                     #    0.68  insn per cycle            
                                                  #    0.10  stalled cycles per insn     (71.48%)
     9,160,166,774      branches                         #  438.747 M/sec                       (71.45%)
        73,843,226      branch-misses                    #    0.81% of all branches             (71.42%)
     6,377,406,035      L1-dcache-loads                  #  305.460 M/sec                       (71.42%)
       114,661,047      L1-dcache-load-misses            #    1.80% of all L1-dcache accesses   (71.40%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

       3.683548165 seconds time elapsed

      20.413057000 seconds user
       0.458709000 seconds sys
```

And the vector instructions are a bit more straightforward!
```
 10.93 │ c0:   vmovdqa64     %zmm1,%zmm0
 10.96 │       inc           %rcx
 11.56 │       vpaddq        %zmm4,%zmm1,%zmm1
       │     for (int64_t i = 0; i < iterations; i++)
       │     {
       │     const double x = (i + 0.5) * step;
 11.49 │       vcvtqq2pd     %zmm0,%zmm0
  9.12 │       vaddpd        %zmm7,%zmm0,%zmm0
  9.05 │       vmulpd        %zmm8,%zmm0,%zmm0
       │     sum += 4.0 / (1.0 + x * x);
  7.69 │       vfmadd132pd   %zmm0,%zmm6,%zmm0
  8.94 │       vdivpd        %zmm0,%zmm5,%zmm0
 10.72 │       vaddpd        %zmm0,%zmm2,%zmm2
  9.55 │       cmp           %rcx,%rdi
       │     ↑ jne           c0
```
