# SSE2Pi

Serial solution using SSE2 vector instructions. When compiling x86\_64 these instructions are enabled by default, meaning the compiler was free to optimize even the SerialPi solution with the same instruction set. Manually using intrinsics allows for finer control and results in better performance. This roughly halves the performance of the Default SerialPi solution, which is on par with what one would expect going from scalar to vector instructions of two packed doubles.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SSE2Pi                                           1             1     1.64459 s 
```

As intrinsics are directly used there is no difference in the compiler flags used and below are the high level statistics.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SSE2Pi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SSE2Pi]':

         20,675.16 msec task-clock                       #    1.000 CPUs utilized             
                57      context-switches                 #    2.757 /sec                      
                 1      cpu-migrations                   #    0.048 /sec                      
           420,158      page-faults                      #   20.322 K/sec                     
   114,145,666,736      cycles                           #    5.521 GHz                         (71.43%)
     8,077,348,842      stalled-cycles-frontend          #    7.08% frontend cycles idle        (71.43%)
   194,844,574,559      instructions                     #    1.71  insn per cycle            
                                                  #    0.04  stalled cycles per insn     (71.43%)
    25,745,103,533      branches                         #    1.245 G/sec                       (71.43%)
        75,447,952      branch-misses                    #    0.29% of all branches             (71.43%)
     6,378,478,722      L1-dcache-loads                  #  308.509 M/sec                       (71.43%)
       114,486,855      L1-dcache-load-misses            #    1.79% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      20.676146121 seconds time elapsed

      20.183434000 seconds user
       0.491986000 seconds sys

$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SSE2Pi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SSE2Pi]':

        67,547,998      ex_ret_brn_misp                  #      0.1 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.00%)
   178,653,560,927      de_src_op_disp.all                                                      (25.00%)
           459,393      resyncs_or_nc_redirects                                                 (25.00%)
   113,737,090,376      ls_not_halted_cyc                                                       (25.00%)
   178,263,369,026      ex_ret_ops                                                              (25.00%)
     5,092,850,954      ex_no_retire.load_not_complete   #     60.4 %  backend_bound_cpu      
                                                  #      4.4 %  backend_bound_memory     (25.00%)
   442,134,252,372      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
    75,055,097,217      ex_no_retire.not_complete                                               (25.00%)
   113,718,912,884      ls_not_halted_cyc                                                       (25.00%)
     5,469,774,684      ex_ret_ucode_ops                 #     25.3 %  retiring_fastpath      
                                                  #      0.8 %  retiring_microcode       (25.00%)
   113,763,434,310      ls_not_halted_cyc                                                       (25.00%)
   178,331,587,382      ex_ret_ops                                                              (25.00%)
    61,741,892,891      de_no_dispatch_per_slot.no_ops_from_frontend #      2.0 %  frontend_bound_bandwidth  (25.00%)
     7,977,841,412      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      7.0 %  frontend_bound_latency   (25.00%)
   113,784,989,539      ls_not_halted_cyc                                                       (25.00%)

      20.589487062 seconds time elapsed

      20.130879000 seconds user
       0.456974000 seconds sys
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
