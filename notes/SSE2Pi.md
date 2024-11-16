# SSE2Pi

Serial solution using SSE2 vector instructions. When compiling x86\_64 these instructions are enabled by default, meaning the compiler was free to optimize even the SerialPi solution with the same instruction set. Manually using intrinsics allows for finer control and results in better performance. This roughly halves the performance of the Default SerialPi solution, which is on par with what one would expect going from scalar to vector instructions of two packed doubles.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SSE2Pi                                           1             1     1.64459 s 
```

As intrinsics are directly used there is no difference in the compiler flags used and below are the high level statistics.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 "[!benchmark][SSE2Pi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [!benchmark][SSE2Pi]':

          5,559.52 msec task-clock                #    1.000 CPUs utilized          
                36      context-switches          #    6.475 /sec                   
                 1      cpu-migrations            #    0.180 /sec                   
           440,144      page-faults               #   79.169 K/sec                  
    30,774,636,581      cycles                    #    5.535 GHz                      (62.44%)
         3,859,518      stalled-cycles-frontend   #    0.01% frontend cycles idle     (62.44%)
       123,237,512      stalled-cycles-backend    #    0.40% backend cycles idle      (62.44%)
    61,062,557,155      instructions              #    1.98  insn per cycle         
                                                  #    0.00  stalled cycles per insn  (62.51%)
     7,800,759,258      branches                  #    1.403 G/sec                    (62.58%)
         4,414,147      branch-misses             #    0.06% of all branches          (62.60%)
     6,421,723,136      L1-dcache-loads           #    1.155 G/sec                    (62.53%)
       120,534,495      L1-dcache-load-misses     #    1.88% of all L1-dcache accesses  (62.46%)
   <not supported>      LLC-loads                                                   
   <not supported>      LLC-load-misses                                             

       5.560120045 seconds time elapsed

       5.315842000 seconds user
       0.243992000 seconds sys
```

As expected the bulk of the work is done by vector instructions in the hot loop.

```
$ perf annotate PiLib::SSE2Pi --stdio --no-source
 Percent |      Source code & Disassembly of PiBench for cycles (13074 samples, percent: local period)
------------------------------------------------------------------------------------------------------
...
    0.00 :   1b650:  movapd %xmm4,%xmm0
    0.00 :   1b654:  movapd %xmm6,%xmm3
    0.00 :   1b658:  add    $0x1,%rax
    0.00 :   1b65c:  mulpd  %xmm1,%xmm0
    0.00 :   1b660:  addpd  %xmm5,%xmm1
    0.00 :   1b664:  mulpd  %xmm0,%xmm0
    0.00 :   1b668:  addpd  %xmm7,%xmm0
    3.04 :   1b66c:  divpd  %xmm0,%xmm3
   72.17 :   1b670:  addpd  %xmm3,%xmm2
   24.79 :   1b674:  cmp    %rax,%rdx
    0.00 :   1b677:  jne    1b650 <PiLib::SSE2Pi(long)+0x80>
...
```
