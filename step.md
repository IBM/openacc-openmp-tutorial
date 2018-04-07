# openacc-openmp-tutorial
This is an advanced tutorial to OpenACC and OpenMP.

## Step 3
In this step we are going to 
* use NVTX markers to better visualize code sections on host and device,
* parallelize a non-trivial loop via OpenACC
* and look into warp branching.

### Literature
* [CUDA Toolkit Documentation - NVTX Markers](http://docs.nvidia.com/cuda/profiler-users-guide/index.html#marking-regions-of-cpu-activity)
* [CUDA Toolkit Documentation - NVPROF command line arguments](http://docs.nvidia.com/cuda/profiler-users-guide/index.html#nvprof-overview)

### Tools
* Code for simple use of the NVTX code is contained in files [nvtx_markers.c](nvtx_markers.c) and [nvtx_markers.h](nvtx_markers.h).

### Your Tasks
* Generate a CPU profile using `perf record -g`, as in step 1, and try to figure out what run-time is taken by the remaining non-accelerated sections of the code. You should find that using OpenMP and/or OpenACC in your application will make such CPU profiles less transparent as they have been initially for serial code.
* In addition to the solution of the tasks in the previous step, the code contains additional sections that were instrumented using NVTX markers. As earlier, generate a GPU timeline using nvprof and visualize it using the NVIDIA Visual Profiler.
* Accelerate, using OpenACC, the following loop in [model.c](model.c), previously parallelized using OpenMP, very similar to the method used in [Step 2](../../blob/step2/step.md).
  ```C
  #pragma omp parallel for
     for (int i=0;i<NumInsects;i++) {
        engage_enemies(i);
     }
  ```
  Goal is to bring the total model time down to 0.67s. Track your progress using `make results` with the same acceptance criterion as in previous steps.
* Run more iterations of the application using the command
  ```
  [user@host openacc-openmp-tutorial]$ ITERATIONS=256 make run SANDBOX="nvprof -f -o profile.nvvp --kernels ::calculate_forces:(^1|01|51)$$ --analysis-metrics --analysis-metrics"
  ==158963== NVPROF is profiling process 158963, command: ./main
  enabling rivalism
  iteration 0
  ==158963== Some kernel(s) will be replayed on device 0 in order to collect all events/metrics.
  Replaying kernel "calculate_forces_74_gpu" (done)
  Replaying kernel "calculate_forces_81_gpu" (done)
  iteration 1
  iteration 2
  iteration 3
  iteration 4
  iteration 5
  iteration 6
  iteration 7
  iteration 8
  iteration 9
  iteration 10
  iteration 11
  iteration 12
  iteration 13
  iteration 14
  iteration 15
  [...]
  ==158963== Generated result file: openacc-openmp-tutorial/profile.nvvp
  ```
  This will enable detailed kernel profiling (for NVVP analysis) of every 50th invocation of each kernel containing the string `calculate_forces` (for each context, for each stream).
  * Use the NVVP GUI to select the profiled kernels and read the analysis. Specifically look for "Warp Execution Efficiency".
  * Use the command `nvprof -i profile.nvvp --print-gpu-trace |& less -S` to find the same using the console.
* Compare iterations profiles: Inspect run-times, and think about why the Warp Execution Efficiency goes down from almost 100% during the first iterations to 30% in the later iterations. Also look at `out/log-leaders.txt`, or `make results`.

### Hints
* The OpenACC analog to the OpenMP `#pragma omp atomic` is `#pragma acc atomic`

## Next Step
Continue with [Step 4](../../blob/step4/step.md)






