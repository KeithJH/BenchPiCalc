# KomputePi
General Purpose GPU (GPGPU) solution using Vulkan compute shaders dispatched using `kompute`. This is likely the least fleshed out solution, but is a good proof-of-concept that the algorithm can also run on a GPU. The implementation notably runs the bulk of the work in a single compute shader dispatch, which appears to not be friendly with Timeout Detection and Recovery (TDR) and can cause system hanging if run for larger data sets.

Running on a Radeon 6900 XT the performance is decent:

```
benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
KomputePi                                        1             1     159.68 ms
```

Additional work worth investigating:
* Split compute dispatch into smaller chunks for better scaling and to avoid system hangs.
* Running with sanitizers reveals a potential memory leak, though it appears to be in the `kompute` library itself.
