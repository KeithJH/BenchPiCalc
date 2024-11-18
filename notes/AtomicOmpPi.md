# AtomicOmpPi
Parallel solution using basic OpenMP constructs, but includes an atomic for sum increment instead of a temporary vector of partial sums that get reduced.

Since the reduction step is such a small part of the overall algorithm, and since it gets optimized fairly well in most cases, there isn't really a performance difference compared to the "naive" solution.

```
$ perf stat -d ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 "[!benchmark][AtomicOmpPi]"
...
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
AtomicOmpPi                                     10             1    446.516 ms 
...
 Performance counter stats for './out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 10 [!benchmark][AtomicOmpPi]':

         79,265.22 msec task-clock                       #   10.609 CPUs utilized             
               574      context-switches                 #    7.242 /sec                      
                 8      cpu-migrations                   #    0.101 /sec                      
           420,189      page-faults                      #    5.301 K/sec                     
   437,063,188,860      cycles                           #    5.514 GHz                         (71.43%)
     8,456,487,789      stalled-cycles-frontend          #    1.93% frontend cycles idle        (71.43%)
   549,725,163,060      instructions                     #    1.26  insn per cycle            
                                                  #    0.02  stalled cycles per insn     (71.44%)
    48,072,718,964      branches                         #  606.479 M/sec                       (71.43%)
        87,053,024      branch-misses                    #    0.18% of all branches             (71.42%)
     6,579,063,680      L1-dcache-loads                  #   83.001 M/sec                       (71.43%)
       123,977,835      L1-dcache-load-misses            #    1.88% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

       7.471674458 seconds time elapsed

      78.718899000 seconds user
       0.540882000 seconds sys

```

While it doesn't seem to impact performance, we can see the atomic instructions produced, such as `lock cmpxchg %rsi,(%rcx)`

```
$ objdump -d out/build/linux-gcc-profile/PiLib/CMakeFiles/PiLib.dir/AtomicOmpPi.cpp.o
...
  58:	66 0f ef c0          	pxor   %xmm0,%xmm0
  5c:	66 0f 28 d4          	movapd %xmm4,%xmm2
  60:	f2 48 0f 2a c2       	cvtsi2sd %rdx,%xmm0
  65:	48 01 c2             	add    %rax,%rdx
  68:	f2 0f 58 c6          	addsd  %xmm6,%xmm0
  6c:	f2 0f 59 c3          	mulsd  %xmm3,%xmm0
  70:	f2 0f 59 c0          	mulsd  %xmm0,%xmm0
  74:	f2 0f 58 c5          	addsd  %xmm5,%xmm0
  78:	f2 0f 5e d0          	divsd  %xmm0,%xmm2
  7c:	f2 0f 58 ca          	addsd  %xmm2,%xmm1
  80:	48 39 d5             	cmp    %rdx,%rbp
  83:	7f d3                	jg     58 <_ZN5PiLib11AtomicOmpPiElm._omp_fn.0+0x58>
  85:	49 8b 4c 24 10       	mov    0x10(%r12),%rcx
  8a:	48 8b 11             	mov    (%rcx),%rdx
  8d:	66 48 0f 6e fa       	movq   %rdx,%xmm7
  92:	48 89 d0             	mov    %rdx,%rax
  95:	f2 0f 58 f9          	addsd  %xmm1,%xmm7
  99:	66 48 0f 7e fe       	movq   %xmm7,%rsi
  9e:	f0 48 0f b1 31       	lock cmpxchg %rsi,(%rcx)
  a3:	75 11                	jne    b6 <_ZN5PiLib11AtomicOmpPiElm._omp_fn.0+0xb6>
  a5:	48 83 c4 10          	add    $0x10,%rsp
  a9:	5b                   	pop    %rbx
  aa:	5d                   	pop    %rbp
  ab:	41 5c                	pop    %r12
  ad:	c3                   	ret
  ae:	66 90                	xchg   %ax,%ax
  b0:	66 0f ef c9          	pxor   %xmm1,%xmm1
  b4:	eb cf                	jmp    85 <_ZN5PiLib11AtomicOmpPiElm._omp_fn.0+0x85>
  b6:	48 89 c2             	mov    %rax,%rdx
  b9:	eb d2                	jmp    8d <_ZN5PiLib11AtomicOmpPiElm._omp_fn.0+0x8d>
  bb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
...
```
