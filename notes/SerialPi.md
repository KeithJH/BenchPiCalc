# SerialPi
Basic serial solution without anything fancy, so likely to be the slowest solution to use as a baseline. There are, however, some interesting interactions with compiler flags which mostly changes usage of scalar and vectorized instructions. While there are many possible combinations the two dimensions analyzed below is the usage of `-march=native` and `-ffast-math`.

## Default
As expected, this is the slowest of the configurations, with neither `-march` or `-ffast-math` specified.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SerialPi                                         1             1     3.43423 s
```

Perf stats need a bit more investigation, but at least branches and cache look good (as there's only really the loop and no real processing from memory).

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SerialPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SerialPi]':

         41,357.35 msec task-clock                       #    1.000 CPUs utilized             
               147      context-switches                 #    3.554 /sec                      
                 4      cpu-migrations                   #    0.097 /sec                      
           420,157      page-faults                      #   10.159 K/sec                     
   228,333,255,160      cycles                           #    5.521 GHz                         (71.43%)
     8,285,158,134      stalled-cycles-frontend          #    3.63% frontend cycles idle        (71.43%)
   549,300,485,416      instructions                     #    2.41  insn per cycle            
                                                  #    0.02  stalled cycles per insn     (71.43%)
    47,920,266,285      branches                         #    1.159 G/sec                       (71.43%)
        80,668,178      branch-misses                    #    0.17% of all branches             (71.43%)
     6,434,253,968      L1-dcache-loads                  #  155.577 M/sec                       (71.43%)
       115,477,561      L1-dcache-load-misses            #    1.79% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      41.359781486 seconds time elapsed

      40.859595000 seconds user
       0.497982000 seconds sys

$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SerialPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SerialPi]':

        71,390,980      ex_ret_brn_misp                  #      0.0 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.00%)
   555,386,556,760      de_src_op_disp.all                                                      (25.00%)
           474,686      resyncs_or_nc_redirects                                                 (25.00%)
   228,799,322,771      ls_not_halted_cyc                                                       (25.00%)
   554,914,348,227      ex_ret_ops                                                              (25.00%)
     5,146,579,719      ex_no_retire.load_not_complete   #     53.1 %  backend_bound_cpu      
                                                  #      2.5 %  backend_bound_memory     (25.00%)
   763,314,291,096      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
   115,004,469,145      ex_no_retire.not_complete                                               (25.00%)
   228,759,804,488      ls_not_halted_cyc                                                       (25.00%)
     5,510,222,951      ex_ret_ucode_ops                 #     40.0 %  retiring_fastpath      
                                                  #      0.4 %  retiring_microcode       (24.99%)
   228,841,683,026      ls_not_halted_cyc                                                       (24.99%)
   555,175,764,260      ex_ret_ops                                                              (24.99%)
    53,811,244,721      de_no_dispatch_per_slot.no_ops_from_frontend #      0.4 %  frontend_bound_bandwidth  (25.00%)
     8,148,430,882      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      3.6 %  frontend_bound_latency   (25.00%)
   228,889,915,295      ls_not_halted_cyc                                                       (25.00%)

      41.416746880 seconds time elapsed

      40.959431000 seconds user
       0.453971000 seconds sys
```

As expected, nearly all the cycles are spent inside `PiLib::SerialPi`

```
$ perf report --stdio
...
# Overhead  Command  Shared Object         Symbol                                                                                                                    
# ........  .......  ....................  ..........................................................................................................................
#
    93.97%  PiBench  PiBench               [.] PiLib::SerialPi(long)
     3.26%  PiBench  [vdso]                [.] 0x0000000000000b00
     0.28%  PiBench  PiBench               [.] std::vector<double, std::allocator<double> > Catch::Benchmark::Detail::resolution<std::chrono::_V2::steady_clock>(int)
     0.21%  PiBench  PiBench               [.] Catch::Benchmark::Detail::weighted_average_quantile(int, int, double*, double*)
     0.16%  PiBench  libstdc++.so.6.0.33   [.] std::chrono::_V2::steady_clock::now()
     0.12%  PiBench  libc.so.6             [.] clock_gettime@@GLIBC_2.17
...
```

Drilling in further, the bulk of the work is unsurprisingly in the math of accumulating the sum in each iteration. Unfortunately the math is all scalar versions, even if the vector registers are being used, as the instructions end in `sd` for "scalar double".

```
$ perf annotate PiLib::SerialPi --stdio
 Percent |      Source code & Disassembly of PiBench for cycles:u (28529 samples, percent: local period)
--------------------------------------------------------------------------------------------------------
...
         : 15    sum += 4.0 / (1.0 + x * x);
    0.05 :   1b009:  mulsd  %xmm1,%xmm1
   20.42 :   1b00d:  addsd  %xmm4,%xmm1
    0.00 :   1b011:  divsd  %xmm1,%xmm3
   42.96 :   1b015:  addsd  %xmm3,%xmm2
         : 10    for (int64_t i = 0; i < iterations; i++)
   36.57 :   1b019:  cmp    %rax,%rdi
    0.00 :   1b01c:  jne    1aff0 <PiLib::SerialPi(long)+0x40>
...
```

## Fast-Math
Fast math on it's own doesn't appear to change anything.

```
$ objdump out/build/linux-gcc-profile/PiLib/libPiLib.a -d | grep sd
   8:	f2 0f 10 25 00 00 00 	movsd  0x0(%rip),%xmm4        # 10 <_ZN5PiLib8SerialPiEl+0x10>
  10:	f2 48 0f 2a cf       	cvtsi2sd %rdi,%xmm1
  19:	f2 0f 5e c1          	divsd  %xmm1,%xmm0
  22:	f2 0f 10 35 00 00 00 	movsd  0x0(%rip),%xmm6        # 2a <_ZN5PiLib8SerialPiEl+0x2a>
  2a:	f2 0f 10 2d 00 00 00 	movsd  0x0(%rip),%xmm5        # 32 <_ZN5PiLib8SerialPiEl+0x32>
  48:	f2 48 0f 2a c8       	cvtsi2sd %rax,%xmm1
  51:	f2 0f 58 ce          	addsd  %xmm6,%xmm1
  55:	f2 0f 59 c8          	mulsd  %xmm0,%xmm1
  59:	f2 0f 59 c9          	mulsd  %xmm1,%xmm1
  5d:	f2 0f 58 cc          	addsd  %xmm4,%xmm1
  61:	f2 0f 5e d9          	divsd  %xmm1,%xmm3
  65:	f2 0f 58 d3          	addsd  %xmm3,%xmm2
  6e:	f2 0f 59 c2          	mulsd  %xmm2,%xmm0

```

## Native
Using `-march=native` on it's own does improve performance.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SerialPi                                         1             1     2.21348 s 
```

Many metrics are the same, though there is a decrease in instructions per cycle and an increase in backend_bound_cpu

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SerialPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SerialPi]':

         27,018.90 msec task-clock                       #    1.000 CPUs utilized             
               128      context-switches                 #    4.737 /sec                      
                 1      cpu-migrations                   #    0.037 /sec                      
           420,157      page-faults                      #   15.550 K/sec                     
   149,168,419,586      cycles                           #    5.521 GHz                         (71.43%)
     8,170,399,930      stalled-cycles-frontend          #    5.48% frontend cycles idle        (71.43%)
   156,134,142,796      instructions                     #    1.05  insn per cycle            
                                                  #    0.05  stalled cycles per insn     (71.43%)
     9,148,719,486      branches                         #  338.604 M/sec                       (71.43%)
        77,142,451      branch-misses                    #    0.84% of all branches             (71.43%)
     6,387,581,691      L1-dcache-loads                  #  236.412 M/sec                       (71.43%)
       115,902,549      L1-dcache-load-misses            #    1.81% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      27.020509481 seconds time elapsed

      26.521168000 seconds user
       0.497984000 seconds sys

$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SerialPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SerialPi]':

        76,197,150      ex_ret_brn_misp                  #      0.1 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.00%)
   157,335,428,335      de_src_op_disp.all                                                      (25.00%)
           461,947      resyncs_or_nc_redirects                                                 (25.00%)
   149,109,802,437      ls_not_halted_cyc                                                       (25.00%)
   156,487,685,055      ex_ret_ops                                                              (25.00%)
     5,085,041,967      ex_no_retire.load_not_complete   #     72.2 %  backend_bound_cpu      
                                                  #      4.2 %  backend_bound_memory     (25.00%)
   683,430,628,457      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
    91,463,796,122      ex_no_retire.not_complete                                               (25.00%)
   149,083,266,597      ls_not_halted_cyc                                                       (25.00%)
     5,481,957,355      ex_ret_ucode_ops                 #     16.9 %  retiring_fastpath      
                                                  #      0.6 %  retiring_microcode       (25.00%)
   149,143,519,674      ls_not_halted_cyc                                                       (25.00%)
   156,406,748,387      ex_ret_ops                                                              (25.00%)
    53,681,903,289      de_no_dispatch_per_slot.no_ops_from_frontend #      0.6 %  frontend_bound_bandwidth  (25.00%)
     8,100,159,491      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #      5.4 %  frontend_bound_latency   (25.00%)
   149,168,167,973      ls_not_halted_cyc                                                       (25.00%)

      26.992284776 seconds time elapsed

      26.543459000 seconds user
       0.446974000 seconds sys
```

There are still many scalar instructions taking up most of the cycles, however, there are some actual vector instructions ending with `pd` ("packed double") including a `fmadd` for fused multiply and add.

```
$ perf annotate PiLib::SerialPi --stdio
Percent |      Source code & Disassembly of PiBench for cycles:u (17717 samples, percent: local period)
--------------------------------------------------------------------------------------------------------
         : 15    sum += 4.0 / (1.0 + x * x);
    0.00 :   1b0a1:  vfmadd132pd %zmm1,%zmm8,%zmm1
    1.36 :   1b0a7:  vdivpd %zmm1,%zmm7,%zmm1
    0.00 :   1b0ad:  vunpckhpd %xmm1,%xmm1,%xmm4
    0.00 :   1b0b1:  vextractf64x2 $0x1,%ymm1,%xmm3
    0.00 :   1b0b8:  vaddsd %xmm1,%xmm13,%xmm2
    4.75 :   1b0bc:  vextractf64x4 $0x1,%zmm1,%ymm1
    0.00 :   1b0c3:  vaddsd %xmm4,%xmm2,%xmm2
   22.35 :   1b0c7:  vaddsd %xmm3,%xmm2,%xmm2
   37.11 :   1b0cb:  vunpckhpd %xmm3,%xmm3,%xmm3
    0.00 :   1b0cf:  vaddsd %xmm3,%xmm2,%xmm2
   24.33 :   1b0d3:  vunpckhpd %xmm1,%xmm1,%xmm3
    0.00 :   1b0d7:  vaddsd %xmm1,%xmm2,%xmm2
    6.15 :   1b0db:  vextractf64x2 $0x1,%ymm1,%xmm1
    0.00 :   1b0e2:  vaddsd %xmm3,%xmm2,%xmm2
    0.48 :   1b0e6:  vaddsd %xmm1,%xmm2,%xmm2
    0.73 :   1b0ea:  vunpckhpd %xmm1,%xmm1,%xmm1
    0.00 :   1b0ee:  vaddsd %xmm1,%xmm2,%xmm13
         : 10    for (int64_t i = 0; i < iterations; i++)
    2.74 :   1b0f2:  cmp    %rax,%rcx
    0.00 :   1b0f5:  jne    1b080 <PiLib::SerialPi(long)+0x80>
```

## Both

We finally get to some rather good performance for a simple solution with both `-march` and `-ffast-math`.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SerialPi                                         1             1    831.513 ms
```

With perhaps gaining some instructions per cycle back but an increase in frontend_bound_latency
```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SerialPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SerialPi]':

         11,708.24 msec task-clock                       #    1.000 CPUs utilized             
                53      context-switches                 #    4.527 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
           420,157      page-faults                      #   35.886 K/sec                     
    64,638,360,775      cycles                           #    5.521 GHz                         (71.42%)
     7,954,397,398      stalled-cycles-frontend          #   12.31% frontend cycles idle        (71.42%)
    78,248,045,518      instructions                     #    1.21  insn per cycle            
                                                  #    0.10  stalled cycles per insn     (71.43%)
     9,039,585,506      branches                         #  772.071 M/sec                       (71.43%)
        67,472,744      branch-misses                    #    0.75% of all branches             (71.43%)
     6,164,254,509      L1-dcache-loads                  #  526.489 M/sec                       (71.44%)
       108,870,491      L1-dcache-load-misses            #    1.77% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      11.709396590 seconds time elapsed

      11.231407000 seconds user
       0.476974000 seconds sys
$ perf stat -M PipelineL2 ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][SerialPi]"
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][SerialPi]':

        71,869,268      ex_ret_brn_misp                  #      0.2 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.00%)
    79,617,320,810      de_src_op_disp.all                                                      (25.00%)
           461,373      resyncs_or_nc_redirects                                                 (25.00%)
    64,868,259,823      ls_not_halted_cyc                                                       (25.00%)
    78,739,407,632      ex_ret_ops                                                              (25.00%)
     5,016,783,031      ex_no_retire.load_not_complete   #     54.4 %  backend_bound_cpu      
                                                  #      6.7 %  backend_bound_memory     (25.00%)
   238,025,025,275      de_no_dispatch_per_slot.backend_stalls                                        (25.00%)
    45,579,219,995      ex_no_retire.not_complete                                               (25.00%)
    64,855,676,073      ls_not_halted_cyc                                                       (25.00%)
     5,447,092,462      ex_ret_ucode_ops                 #     18.8 %  retiring_fastpath      
                                                  #      1.4 %  retiring_microcode       (24.99%)
    64,879,369,971      ls_not_halted_cyc                                                       (24.99%)
    78,751,066,062      ex_ret_ops                                                              (24.99%)
    71,408,783,951      de_no_dispatch_per_slot.no_ops_from_frontend #      6.1 %  frontend_bound_bandwidth  (25.00%)
     7,933,105,359      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/ #     12.2 %  frontend_bound_latency   (25.00%)
    64,893,763,604      ls_not_halted_cyc                                                       (25.00%)

      11.743191364 seconds time elapsed

      11.251013000 seconds user
       0.490956000 seconds sys
```

But most importantly, more reliance on actual vector instructions!

```
$ perf annotate PiLib::SerialPi --stdio
 Percent |      Source code & Disassembly of PiBench for cycles:u (6668 samples, percent: local period)
-------------------------------------------------------------------------------------------------------
...
         : 15    sum += 4.0 / (1.0 + x * x);
    5.91 :   1b081:  vfmadd132pd %zmm1,%zmm7,%zmm1
    1.14 :   1b087:  vdivpd %zmm1,%zmm6,%zmm1
   83.16 :   1b08d:  vaddpd %zmm1,%zmm3,%zmm3
         : 10    for (int64_t i = 0; i < iterations; i++)
    9.78 :   1b093:  cmp    %rax,%rcx
    0.00 :   1b096:  jne    1b060 <PiLib::SerialPi(long)+0x80>
...
```
