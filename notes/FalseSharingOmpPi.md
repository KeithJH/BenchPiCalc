# FalseSharingOmpPi
Basic parallel solution with `#pragma omp parallel` but intentionally causes false sharing issues. This is accomplished by using a vector of accumulators with each thread getting it's own. No thread is actually sharing any memory, but since they are all using memory on the same cache line this should cause issues as the entire line is marked dirty in cache on update of any given record. While a bit contrived, the implementation uses `volatile` forces reads to not be optimized away so that the effects of false sharing can be analyzed.

As expected, this results in rather poor performance, even worse than the serial implementation.

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
FalseSharingOmpPi                                5             1     6.80325 s 
```

Issues are immediately evident looking at the instructions per cycle and L1 misses

```
$ perf stat -d out/build/linux-gcc-release/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 5 "[pi][FalseSharingOmpPi]"
 Performance counter stats for 'out/build/linux-gcc-release/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [pi][FalseSharingOmpPi]':

        164,521.67 msec task-clock:u                     #   10.141 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
           420,186      page-faults:u                    #    2.554 K/sec                     
   903,126,719,008      cycles:u                         #    5.489 GHz                         (71.42%)
     6,520,973,852      stalled-cycles-frontend:u        #    0.72% frontend cycles idle        (71.42%)
   119,585,628,179      instructions:u                   #    0.13  insn per cycle            
                                                  #    0.05  stalled cycles per insn     (71.43%)
    11,088,594,817      branches:u                       #   67.399 M/sec                       (71.43%)
         6,530,811      branch-misses:u                  #    0.06% of all branches             (71.44%)
    13,927,784,581      L1-dcache-loads:u                #   84.656 M/sec                       (71.44%)
     1,622,535,545      L1-dcache-load-misses:u          #   11.65% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads:u                                                           
   <not supported>      LLC-load-misses:u                                                     

      16.222671843 seconds time elapsed

     164.022241000 seconds user
       0.495970000 seconds sys
```

Even more evidence from the `backend_bound_memory`

```
 Performance counter stats for 'out/build/linux-gcc-release/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [pi][FalseSharingOmpPi]':

         9,412,476      ex_ret_brn_misp:u                #      0.0 %  bad_speculation_mispredicts
                                                  #      0.0 %  bad_speculation_pipeline_restarts  (25.00%)
   126,084,070,259      de_src_op_disp.all:u                                                    (25.00%)
           432,825      resyncs_or_nc_redirects:u                                               (25.00%)
   893,820,129,868      ls_not_halted_cyc:u                                                     (25.00%)
   125,287,914,639      ex_ret_ops:u                                                            (25.00%)
   816,551,000,461      ex_no_retire.load_not_complete:u #      4.8 %  backend_bound_cpu      
                                                  #     89.7 %  backend_bound_memory     (25.00%)
 5,066,718,887,020      de_no_dispatch_per_slot.backend_stalls:u                                        (25.00%)
   860,095,759,180      ex_no_retire.not_complete:u                                             (25.00%)
   893,536,108,292      ls_not_halted_cyc:u                                                     (25.00%)
     5,373,486,313      ex_ret_ucode_ops:u               #      2.2 %  retiring_fastpath      
                                                  #      0.1 %  retiring_microcode       (25.00%)
   894,003,646,322      ls_not_halted_cyc:u                                                     (25.00%)
   125,408,614,137      ex_ret_ops:u                                                            (25.00%)
    43,291,180,371      de_no_dispatch_per_slot.no_ops_from_frontend:u #      0.1 %  frontend_bound_bandwidth  (25.01%)
     6,549,057,625      cpu/de_no_dispatch_per_slot.no_ops_from_frontend,cmask=0x6/u #      0.7 %  frontend_bound_latency   (25.01%)
   894,218,593,210      ls_not_halted_cyc:u                                                     (25.01%)

      16.191116801 seconds time elapsed

     162.164628000 seconds user
       0.507092000 seconds sys
```

Next we can specifically look for cache issues

```
$ perf c2c record out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 "[pi][FalseSharingOmpPi]"
Shared Data Cache Line Table     (3 entries, sorted on Total HITMs)
       ----------- Cacheline ----------      Tot  ------- Load Hitm -------
Index             Address  Node  PA cnt     Hitm    Total  LclHitm  RmtHitm
    0      0x59eab1751d80     0   38924   48.52%     5632     5632        0
    1      0x59eab1751dc0     0   13731   32.24%     3742     3742        0
    2      0x59eab1751d40     0   11825   19.23%     2232     2232        0
...
Cacheline 0x59eab1751d80
----- HITM -----  ------- Store Refs ------  ------- CL --------                      ---------- cycles ----------    Total       cpu                                   Shared
RmtHitm  LclHitm   L1 Hit  L1 Miss      N/A    Off  Node  PA cnt        Code address  rmt hitm  lcl hitm      load  records       cnt                          Symbol   Object               Source:Line  Node
  0.00%   16.03%    0.00%    0.00%    0.00%    0x0     0       1      0x59eab108b0ec         0      1425         5     1810         1  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    0.00%   12.42%    0.00%   14.92%    0x0     0       1      0x59eab108b0f0         0         0         0     2947         1  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%   11.63%    0.00%    0.00%    0.00%    0x8     0       1      0x59eab108b0ec         0      1381       252     2343         5  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    0.00%   12.53%    0.00%   10.93%    0x8     0       1      0x59eab108b0f0         0         0         0     2719         5  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%   11.19%    0.00%    0.00%    0.00%   0x10     0       1      0x59eab108b0ec         0      1424       240     2094         5  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    0.00%   12.70%    0.00%   10.96%   0x10     0       1      0x59eab108b0f0         0         0         0     2749         6  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%   11.42%    0.00%    0.00%    0.00%   0x18     0       1      0x59eab108b0ec         0      1474       262     2287         1  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    0.00%   12.67%    0.00%   12.37%   0x18     0       1      0x59eab108b0f0         0         0         0     2831         1  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    8.98%    0.00%    0.00%    0.00%   0x20     0       1      0x59eab108b0ec         0      1412       365     2607         3  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    0.00%   12.55%    0.00%   10.11%   0x20     0       1      0x59eab108b0f0         0         0         0     2672         3  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    9.82%    0.00%    0.00%    0.00%   0x28     0       1      0x59eab108b0ec         0      1342       260     2186         3  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    0.00%   12.47%    0.00%   10.49%   0x28     0       1      0x59eab108b0f0         0         0         0     2682         3  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%   14.90%    0.00%    0.00%    0.00%   0x30     0       1      0x59eab108b0ec         0      1388         0     1714         4  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    0.00%   12.48%    0.00%   14.64%   0x30     0       1      0x59eab108b0f0         0         0         0     2940         4  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%   16.03%    0.00%    0.00%    0.00%   0x38     0       1      0x59eab108b0ec         0      1436         7     1708         4  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
  0.00%    0.00%   12.20%    0.00%   15.58%   0x38     0       1      0x59eab108b0f0         0         0         0     2952         4  [.] PiLib::FalseSharingOmpPi(l  PiBench  FalseSharingOmpPi.cpp:34   0
...
```

As expected, this is the line that increments the sums

```
$ sed '34q;d' PiLib/FalseSharingOmpPi.cpp 
			sum[id] += 4.0 / (1.0 + x * x);
```

While it's easy enough to just avoid using such a "shared" vector of values, you can observe the difference if the values were at least on different cache lines with a bad solution such as aligning each element of the vector in a hacky way such as:

```
	struct alignas(64) BadSolution
	{
		double value;
	};

	std::vector<BadSolution> sum (threadCount);
```

```
$ perf stat -d out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 "[pi][FalseSharingOmpPi]"
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
FalseSharingOmpPi ("fixed")                                1             1    551.898 ms 
 Performance counter stats for 'out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 [pi][FalseSharingOmpPi]':

         20,373.57 msec task-clock                       #    5.491 CPUs utilized             
               240      context-switches                 #   11.780 /sec                      
                 8      cpu-migrations                   #    0.393 /sec                      
           420,189      page-faults                      #   20.624 K/sec                     
   112,253,030,558      cycles                           #    5.510 GHz                         (71.40%)
     8,071,281,166      stalled-cycles-frontend          #    7.19% frontend cycles idle        (71.41%)
   122,522,372,006      instructions                     #    1.09  insn per cycle            
                                                  #    0.07  stalled cycles per insn     (71.43%)
    11,706,499,155      branches                         #  574.592 M/sec                       (71.45%)
        74,405,041      branch-misses                    #    0.64% of all branches             (71.47%)
    14,406,478,142      L1-dcache-loads                  #  707.116 M/sec                       (71.47%)
       116,772,686      L1-dcache-load-misses            #    0.81% of all L1-dcache accesses   (71.42%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

       3.710509097 seconds time elapsed

      19.863492000 seconds user
       0.504707000 seconds sys
```
