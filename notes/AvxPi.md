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
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][AvxPi]':

         18,989.80 msec task-clock                       #    1.000 CPUs utilized             
                92      context-switches                 #    4.845 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
           420,156      page-faults                      #   22.125 K/sec                     
   104,838,147,991      cycles                           #    5.521 GHz                         (71.43%)
     8,033,075,338      stalled-cycles-frontend          #    7.66% frontend cycles idle        (71.43%)
   178,612,789,114      instructions                     #    1.70  insn per cycle            
                                                  #    0.04  stalled cycles per insn     (71.43%)
    23,683,097,575      branches                         #    1.247 G/sec                       (71.43%)
        74,687,544      branch-misses                    #    0.32% of all branches             (71.43%)
     6,359,120,038      L1-dcache-loads                  #  334.870 M/sec                       (71.43%)
       114,107,280      L1-dcache-load-misses            #    1.79% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      18.991871520 seconds time elapsed

      18.510092000 seconds user
       0.479950000 seconds sys

$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][AvxPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][AvxPi]':

        73,431,668      ex_ret_brn_misp                  #      0.2 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.00%)
   165,497,956,702      de_src_op_disp.all                                                      (25.00%)
           462,005      resyncs_or_nc_redirects                                                 (25.00%)
   105,036,989,901      ls_not_halted_cyc                                                       (25.00%)
   164,421,923,897      ex_ret_ops                                                              (25.00%)
     5,164,675,949      ex_no_retire.load_not_complete   #     59.6 %  backend_bound_cpu      
                                                  #      4.4 %  backend_bound_memory     (25.00%)
   403,173,685,733      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
    74,857,151,606      ex_no_retire.not_complete                                               (25.00%)
   105,021,496,039      ls_not_halted_cyc                                                       (25.00%)
     5,458,038,363      ex_ret_ucode_ops                 #     25.2 %  retiring_fastpath      
                                                  #      0.9 %  retiring_microcode       (25.00%)
   105,061,826,471      ls_not_halted_cyc                                                       (25.00%)
   164,437,770,686      ex_ret_ops                                                              (25.00%)
    61,030,838,582      de_no_dispatch_per_slot.no_ops_from_frontend #      2.0 %  frontend_bound_bandwidth  (24.99%)
     8,019,190,455      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      7.6 %  frontend_bound_latency   (24.99%)
   105,079,829,837      ls_not_halted_cyc                                                       (24.99%)

      19.015071526 seconds time elapsed

      18.536473000 seconds user
       0.476960000 seconds sys


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
