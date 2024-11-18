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

         19,110.87 msec task-clock                       #    1.000 CPUs utilized             
                64      context-switches                 #    3.349 /sec                      
                 2      cpu-migrations                   #    0.105 /sec                      
           420,158      page-faults                      #   21.985 K/sec                     
   105,508,569,005      cycles                           #    5.521 GHz                         (71.42%)
     8,017,030,521      stalled-cycles-frontend          #    7.60% frontend cycles idle        (71.43%)
    97,956,848,664      instructions                     #    0.93  insn per cycle            
                                                  #    0.08  stalled cycles per insn     (71.43%)
    13,610,372,324      branches                         #  712.180 M/sec                       (71.43%)
        68,562,234      branch-misses                    #    0.50% of all branches             (71.43%)
     6,196,380,537      L1-dcache-loads                  #  324.233 M/sec                       (71.43%)
       110,569,793      L1-dcache-load-misses            #    1.78% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      19.112972381 seconds time elapsed

      18.592124000 seconds user
       0.518947000 seconds sys

$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][Avx512Pi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][Avx512Pi]':

        73,846,352      ex_ret_brn_misp                  #      0.2 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.00%)
    95,052,506,441      de_src_op_disp.all                                                      (25.00%)
           463,234      resyncs_or_nc_redirects                                                 (25.00%)
   105,619,833,379      ls_not_halted_cyc                                                       (25.00%)
    93,943,579,125      ex_ret_ops                                                              (25.00%)
     5,036,308,790      ex_no_retire.load_not_complete   #     71.5 %  backend_bound_cpu      
                                                  #      4.4 %  backend_bound_memory     (25.00%)
   480,522,837,700      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
    87,389,897,172      ex_no_retire.not_complete                                               (25.00%)
   105,598,034,281      ls_not_halted_cyc                                                       (25.00%)
     5,447,384,862      ex_ret_ucode_ops                 #     14.0 %  retiring_fastpath      
                                                  #      0.9 %  retiring_microcode       (25.00%)
   105,641,921,876      ls_not_halted_cyc                                                       (25.00%)
    93,938,664,040      ex_ret_ops                                                              (25.00%)
    57,773,673,933      de_no_dispatch_per_slot.no_ops_from_frontend #      1.6 %  frontend_bound_bandwidth  (25.00%)
     7,987,372,736      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      7.6 %  frontend_bound_latency   (25.00%)
   105,661,601,675      ls_not_halted_cyc                                                       (25.00%)

      19.119576664 seconds time elapsed

      18.631119000 seconds user
       0.486976000 seconds sys
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
