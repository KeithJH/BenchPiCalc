# SimdThreadPi
Parallel solution manually using `std::thread` and AVX512 vector instructions. It would likely be just as good, if not better, to adjust `ThreadPi` so that the optimizer uses vector instructions. Perhaps by adjusting the hot loop to have a stride of 1, similar to how this manual solution works.

Performance is in the same ballpark as `ForOmpPi` with `-march=native` and `-ffast-math`, which is probably to be expected

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SimdThreadPi                                    10             1    104.933 ms
```

Stats are similar too

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SimdThreadPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SimdThreadPi]':

         20,980.74 msec task-clock                       #    5.586 CPUs utilized             
               398      context-switches                 #   18.970 /sec                      
                 2      cpu-migrations                   #    0.095 /sec                      
           420,414      page-faults                      #   20.038 K/sec                     
   115,686,638,242      cycles                           #    5.514 GHz                         (71.34%)
     8,143,654,159      stalled-cycles-frontend          #    7.04% frontend cycles idle        (71.40%)
    61,814,256,284      instructions                     #    0.53  insn per cycle            
                                                  #    0.13  stalled cycles per insn     (71.69%)
     9,114,315,043      branches                         #  434.413 M/sec                       (71.76%)
        76,895,424      branch-misses                    #    0.84% of all branches             (71.55%)
     6,354,734,672      L1-dcache-loads                  #  302.884 M/sec                       (71.45%)
       114,112,094      L1-dcache-load-misses            #    1.80% of all L1-dcache accesses   (71.37%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

       3.755955804 seconds time elapsed

      20.376128000 seconds user
       0.524099000 seconds sys
```

And of course, the assembly is much the same with the packed vector instructions

```
 14.02 │ 78:   vmulpd        %zmm2,%zmm0,%zmm1
 13.85 │       add           $0x1,%rdi
 13.92 │       vaddpd        %zmm4,%zmm2,%zmm2
 13.77 │       vfmadd132pd   %zmm1,%zmm6,%zmm1
 15.44 │       vdivpd        %zmm1,%zmm5,%zmm1
 15.08 │       vaddpd        %zmm1,%zmm3,%zmm3
 13.93 │       cmp           %rdi,%rsi
       │     ↑ jne           78
```
