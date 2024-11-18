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

        136,177.78 msec task-clock                       #   12.452 CPUs utilized             
             6,622      context-switches                 #   48.628 /sec                      
                10      cpu-migrations                   #    0.073 /sec                      
           420,187      page-faults                      #    3.086 K/sec                     
   750,981,259,503      cycles                           #    5.515 GHz                         (71.44%)
     8,935,762,215      stalled-cycles-frontend          #    1.19% frontend cycles idle        (71.43%)
   985,060,806,919      instructions                     #    1.31  insn per cycle            
                                                  #    0.01  stalled cycles per insn     (71.42%)
    84,312,604,183      branches                         #  619.136 M/sec                       (71.42%)
        96,713,828      branch-misses                    #    0.11% of all branches             (71.43%)
     6,561,544,051      L1-dcache-loads                  #   48.184 M/sec                       (71.44%)
       136,636,259      L1-dcache-load-misses            #    2.08% of all L1-dcache accesses   (71.44%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      10.936137925 seconds time elapsed

     135.664467000 seconds user
       0.506192000 seconds sys

$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][NaiveOmpPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][NaiveOmpPi]':

        97,872,097      ex_ret_brn_misp                  #      0.0 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.01%)
   991,925,540,818      de_src_op_disp.all                                                      (25.01%)
           494,483      resyncs_or_nc_redirects                                                 (25.01%)
   748,646,141,099      ls_not_halted_cyc                                                       (25.01%)
   991,224,784,318      ex_ret_ops                                                              (25.01%)
     5,739,696,146      ex_no_retire.load_not_complete   #     40.7 %  backend_bound_cpu      
                                                  #      0.5 %  backend_bound_memory     (25.01%)
 1,850,404,234,229      de_no_dispatch_per_slot.backend_stalls                                        (25.01%)
   505,195,468,339      ex_no_retire.not_complete                                               (25.01%)
   748,561,787,119      ls_not_halted_cyc                                                       (25.01%)
     5,862,369,987      ex_ret_ucode_ops                 #     21.9 %  retiring_fastpath      
                                                  #      0.1 %  retiring_microcode       (24.99%)
   748,899,878,838      ls_not_halted_cyc                                                       (24.99%)
   991,394,385,598      ex_ret_ops                                                              (24.99%)
    59,402,072,745      de_no_dispatch_per_slot.no_ops_from_frontend #      0.1 %  frontend_bound_bandwidth  (25.00%)
     8,870,592,648      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      1.2 %  frontend_bound_latency   (25.00%)
   749,005,998,766      ls_not_halted_cyc                                                       (25.00%)

      10.956023193 seconds time elapsed

     134.864446000 seconds user
       0.758196000 seconds sys
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
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][NaiveOmpPi]':

        134,031.45 msec task-clock                       #   12.420 CPUs utilized             
             6,876      context-switches                 #   51.301 /sec                      
                 6      cpu-migrations                   #    0.045 /sec                      
           420,187      page-faults                      #    3.135 K/sec                     
   739,095,978,772      cycles                           #    5.514 GHz                         (71.43%)
     8,933,400,291      stalled-cycles-frontend          #    1.21% frontend cycles idle        (71.44%)
   743,948,448,565      instructions                     #    1.01  insn per cycle            
                                                  #    0.01  stalled cycles per insn     (71.44%)
    84,324,392,927      branches                         #  629.139 M/sec                       (71.44%)
       100,894,727      branch-misses                    #    0.12% of all branches             (71.43%)
     6,753,288,475      L1-dcache-loads                  #   50.386 M/sec                       (71.42%)
       137,978,814      L1-dcache-load-misses            #    2.04% of all L1-dcache accesses   (71.42%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      10.791983187 seconds time elapsed

     133.499108000 seconds user
       0.512160000 seconds sys

$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][NaiveOmpPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [!benchmark][NaiveOmpPi]':

       100,491,496      ex_ret_brn_misp                  #      0.0 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.01%)
   753,897,422,066      de_src_op_disp.all                                                      (25.01%)
           492,944      resyncs_or_nc_redirects                                                 (25.01%)
   737,714,363,333      ls_not_halted_cyc                                                       (25.01%)
   752,845,402,436      ex_ret_ops                                                              (25.01%)
     5,748,514,254      ex_no_retire.load_not_complete   #     50.8 %  backend_bound_cpu      
                                                  #      0.5 %  backend_bound_memory     (25.00%)
 2,271,711,524,607      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
   574,057,714,665      ex_no_retire.not_complete                                               (25.00%)
   737,610,168,961      ls_not_halted_cyc                                                       (25.00%)
     5,849,727,200      ex_ret_ucode_ops                 #     16.7 %  retiring_fastpath      
                                                  #      0.1 %  retiring_microcode       (25.00%)
   737,942,023,177      ls_not_halted_cyc                                                       (25.00%)
   746,263,542,911      ex_ret_ops                                                              (25.00%)
   151,701,655,890      de_no_dispatch_per_slot.no_ops_from_frontend #      2.2 %  frontend_bound_bandwidth  (24.99%)
     8,836,052,216      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      1.2 %  frontend_bound_latency   (24.99%)
   738,020,187,714      ls_not_halted_cyc                                                       (24.99%)

      10.778714235 seconds time elapsed

     133.140306000 seconds user
       0.496341000 seconds sys
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

Oddly there doesn't seem to be more improvement with both `-march` and `-ffast-math`.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
NaiveOmpPi                                      10             1    415.363 ms 
```

Statistics:

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][NaiveOmpPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][NaiveOmpPi]':

        133,143.59 msec task-clock                       #   12.322 CPUs utilized             
             7,234      context-switches                 #   54.332 /sec                      
                12      cpu-migrations                   #    0.090 /sec                      
           420,186      page-faults                      #    3.156 K/sec                     
   734,166,001,264      cycles                           #    5.514 GHz                         (71.42%)
     8,929,282,181      stalled-cycles-frontend          #    1.22% frontend cycles idle        (71.43%)
   743,629,856,718      instructions                     #    1.01  insn per cycle            
                                                  #    0.01  stalled cycles per insn     (71.44%)
    84,374,610,854      branches                         #  633.711 M/sec                       (71.44%)
        99,672,335      branch-misses                    #    0.12% of all branches             (71.44%)
     6,750,782,966      L1-dcache-loads                  #   50.703 M/sec                       (71.42%)
       142,736,494      L1-dcache-load-misses            #    2.11% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      10.804999013 seconds time elapsed

     132.514096000 seconds user
       0.608678000 seconds sys

$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][NaiveOmpPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [!benchmark][NaiveOmpPi]':

        95,871,810      ex_ret_brn_misp                  #      0.0 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.00%)
   750,127,318,990      de_src_op_disp.all                                                      (25.00%)
           495,555      resyncs_or_nc_redirects                                                 (25.00%)
   738,326,367,893      ls_not_halted_cyc                                                       (25.00%)
   749,412,937,569      ex_ret_ops                                                              (25.00%)
     5,784,421,123      ex_no_retire.load_not_complete   #     51.7 %  backend_bound_cpu      
                                                  #      0.5 %  backend_bound_memory     (25.00%)
 2,314,170,360,324      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
   574,688,544,067      ex_no_retire.not_complete                                               (25.00%)
   738,189,918,039      ls_not_halted_cyc                                                       (25.00%)
     5,836,052,609      ex_ret_ucode_ops                 #     16.8 %  retiring_fastpath      
                                                  #      0.1 %  retiring_microcode       (25.00%)
   738,481,223,867      ls_not_halted_cyc                                                       (25.00%)
   749,965,933,953      ex_ret_ops                                                              (25.00%)
    58,694,866,966      de_no_dispatch_per_slot.no_ops_from_frontend #      0.1 %  frontend_bound_bandwidth  (25.01%)
     8,778,798,269      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      1.2 %  frontend_bound_latency   (25.01%)
   738,668,489,497      ls_not_halted_cyc                                                       (25.01%)

      10.778114963 seconds time elapsed

     133.236928000 seconds user
       0.509181000 seconds sys
```

It looks like we are not getting the same packed vector instructions as we were seeing in the same configuration for SerialPi

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
