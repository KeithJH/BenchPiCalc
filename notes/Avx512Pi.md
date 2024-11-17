# Avx512Pi

Serial solution using Avx512 vector instructions, including fused multiply add. With 512-bit registers this can perform 8 "iterations" at once, which is twice as many as the AVX solution. This, however, does not appear to be giving any more performance gains, which needs more investigation.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
Avx512Pi                                        10             1    827.005 ms 
```

As intrinsics are directly used there is little difference in the compiler flags used and below are the high level statistics.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][Avx512Pi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][Avx512Pi]':

         11,425.51 msec task-clock:u              #    1.000 CPUs utilized          
                 0      context-switches:u        #    0.000 /sec                   
                 0      cpu-migrations:u          #    0.000 /sec                   
           440,143      page-faults:u             #   38.523 K/sec                  
    61,964,935,828      cycles:u                  #    5.423 GHz                      (62.51%)
         2,730,870      stalled-cycles-frontend:u #    0.00% frontend cycles idle     (62.51%)
       103,852,953      stalled-cycles-backend:u  #    0.17% backend cycles idle      (62.51%)
    59,386,731,613      instructions:u            #    0.96  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.50%)
     8,980,599,571      branches:u                #  786.013 M/sec                    (62.51%)
         7,394,460      branch-misses:u           #    0.08% of all branches          (62.49%)
     5,769,329,491      L1-dcache-loads:u         #  504.951 M/sec                    (62.49%)
        76,275,377      L1-dcache-load-misses:u   #    1.32% of all L1-dcache accesses  (62.49%)
   <not supported>      LLC-loads:u                                                 
   <not supported>      LLC-load-misses:u                                           

      11.425801767 seconds time elapsed

      11.177638000 seconds user
       0.248036000 seconds sys
```

As expected the bulk of the work is done by vector instructions in the hot loop. Notably there isn't any loop unrolling, which would be intriguing to explore if/how much that would help.

```
$ perf annotate PiLib::Avx512Pi --stdio --no-source
 Percent |      Source code & Disassembly of PiBench for cycles:u (36337 samples, percent: local period)
--------------------------------------------------------------------------------------------------------
...
    0.00 :   1cc10:  vmulpd %zmm1,%zmm3,%zmm0
    0.00 :   1cc16:  add    $0x1,%rax
    0.00 :   1cc1a:  vaddpd %zmm4,%zmm1,%zmm1
    0.00 :   1cc20:  vfmadd132pd %zmm0,%zmm6,%zmm0
    0.00 :   1cc26:  vdivpd %zmm0,%zmm5,%zmm0
   99.72 :   1cc2c:  vaddpd %zmm0,%zmm2,%zmm2
    0.28 :   1cc32:  cmp    %rax,%rdx
    0.00 :   1cc35:  jne    1cc10 <PiLib::Avx512Pi(long)+0x90>
...
```
