# ThreadPi
Parallel solution manually using `std::thread` and `std::atomic<double>` to reduce to the final result.

# Default
Performance is in the same ballpark as all the other parallel solutions

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
ThreadPi                                        10             1    420.879 ms 
```

Stats are similar too

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][ThreadPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][ThreadPi]':

         75,972.26 msec task-clock                       #   10.675 CPUs utilized             
             5,835      context-switches                 #   76.804 /sec                      
                 4      cpu-migrations                   #    0.053 /sec                      
           420,411      page-faults                      #    5.534 K/sec                     
   418,847,781,775      cycles                           #    5.513 GHz                         (71.38%)
     8,485,126,220      stalled-cycles-frontend          #    2.03% frontend cycles idle        (71.43%)
   549,026,835,145      instructions                     #    1.31  insn per cycle            
                                                  #    0.02  stalled cycles per insn     (71.50%)
    47,916,821,332      branches                         #  630.715 M/sec                       (71.51%)
        84,033,726      branch-misses                    #    0.18% of all branches             (71.48%)
     6,410,742,784      L1-dcache-loads                  #   84.383 M/sec                       (71.45%)
       122,927,827      L1-dcache-load-misses            #    1.92% of all L1-dcache accesses   (71.39%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

       7.116801906 seconds time elapsed

      75.383555000 seconds user
       0.510556000 seconds sys
```

And of course, the assembly is much the same with the scalar vector instructions

```
  8.16 │50:   pxor     %xmm0,%xmm0
  8.16 │      movapd   %xmm5,%xmm2
 16.24 │      cvtsi2sd %rsi,%xmm0
  8.12 │      add      %rdx,%rsi
  8.13 │      addsd    %xmm6,%xmm0
  8.47 │      mulsd    %xmm4,%xmm0
  8.49 │      mulsd    %xmm0,%xmm0
  8.55 │      addsd    %xmm3,%xmm0
  8.53 │      divsd    %xmm0,%xmm2
  8.55 │      addsd    %xmm2,%xmm1
  8.59 │      cmp      %rsi,%rdi
       │    ↑ jg       50
```

# Native, Fast-math
Very little difference in performance with both `-march-native` and `-ffast-math`

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
ThreadPi                                        10             1    414.206 ms 
```

This is likely because it's still using scalar instead of packed vector instructions, unfortunately. It probably does not like the use of strides greater than 1 for the inner-loop.

```
 21.20 │50:   vcvtsi2sd   %rsi,%xmm4,%xmm0
 10.63 │      vaddsd      %xmm6,%xmm0,%xmm0
 11.08 │      vmulsd      %xmm2,%xmm0,%xmm0
 11.04 │      vfmadd132sd %xmm0,%xmm3,%xmm0
 11.05 │      add         %rdx,%rsi
 11.58 │      vdivsd      %xmm0,%xmm5,%xmm0
 11.73 │      vaddsd      %xmm0,%xmm1,%xmm1
 11.70 │      cmp         %rsi,%rdi
       │    ↑ jg          50
```
