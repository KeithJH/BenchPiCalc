# AvxPi

Serial solution using Avx vector instructions, including fused multiply add. With 256-bit registers this can perform 4 "iterations" at once, which is twice as many as the SSE2 solution. As the problem scales particularly well, we also get about twice the performance.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
AvxPi                                           10             1    820.518 ms 
```

As intrinsics are directly used there is little difference in the compiler flags used and below are the high level statistics.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][AvxPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [AvxPi]':

         11,381.24 msec task-clock:u              #    1.000 CPUs utilized          
                 0      context-switches:u        #    0.000 /sec                   
                 0      cpu-migrations:u          #    0.000 /sec                   
           440,143      page-faults:u             #   38.673 K/sec                  
    61,685,936,756      cycles:u                  #    5.420 GHz                      (62.47%)
         3,090,361      stalled-cycles-frontend:u #    0.01% frontend cycles idle     (62.47%)
       102,948,035      stalled-cycles-backend:u  #    0.17% backend cycles idle      (62.48%)
   103,589,429,297      instructions:u            #    1.68  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.52%)
    14,510,411,087      branches:u                #    1.275 G/sec                    (62.56%)
         7,090,506      branch-misses:u           #    0.05% of all branches          (62.54%)
     5,825,800,984      L1-dcache-loads:u         #  511.877 M/sec                    (62.50%)
        77,219,057      L1-dcache-load-misses:u   #    1.33% of all L1-dcache accesses  (62.47%)
   <not supported>      LLC-loads:u                                                 
   <not supported>      LLC-load-misses:u                                           

      11.381673143 seconds time elapsed

      11.089253000 seconds user
       0.292033000 seconds sys
```

As expected the bulk of the work is done by vector instructions in the hot loop. Notably there isn't any loop unrolling, which would be intriguing to explore if/how much that would help.

```
$ perf annotate PiLib::AvxPi --stdio --no-source
 Percent |      Source code & Disassembly of PiBench for cycles:u (36227 samples, percent: local period)
--------------------------------------------------------------------------------------------------------
...
    0.00 :   1cea0:  vmulpd %ymm1,%ymm4,%ymm0
    0.00 :   1cea4:  add    $0x1,%rax
    0.00 :   1cea8:  vaddpd %ymm3,%ymm1,%ymm1
    0.00 :   1ceac:  vfmadd132pd %ymm0,%ymm5,%ymm0
    0.00 :   1ceb1:  vdivpd %ymm0,%ymm3,%ymm0
   46.52 :   1ceb5:  vaddpd %ymm0,%ymm2,%ymm2
   53.48 :   1ceb9:  cmp    %rax,%rdx
    0.00 :   1cebc:  jne    1cea0 <PiLib::AvxPi(long)+0x80>
...
```
