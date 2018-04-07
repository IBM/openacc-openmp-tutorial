# openacc-openmp-tutorial
This is an advanced tutorial to OpenACC and OpenMP.

## Step 1
In this step we are going to introduce OpenMP, and do some simple application profiling to identify hot-spots and apply some trivial parallelization using OpenMP pragmas.

### Literature
* [Wikipedia article on OpenMP](https://en.wikipedia.org/wiki/OpenMP)
* [OpenMP 4.5 Syntax Reference Guide](http://www.openmp.org/wp-content/uploads/OpenMP-4.5-1115-CPP-web.pdf)
* [OpenMP 4.5 API Documentation](http://www.openmp.org/wp-content/uploads/openmp-4.5.pdf)
* [Excellent tutorial on OpenMP](https://computing.llnl.gov/tutorials/openMP)

### OpenMP
Annotations to program source code that suggest to a compiler to execute code in so-called *parallel regions* in a specific parallel way.
For each parallel region, the master thread forks into threads each executing the parallel region, before joining back into the single master thread.
![OpenMP parallel region flow](https://computing.llnl.gov/tutorials/openMP/images/fork_join2.gif)

```c
void code() {
  int a,b,c[8];
  serial_code(a,b,c); 
#pragma omp parallel private(a, b) shared(c)
  {
    parallel_code(a,c);     // executed by each of the threads
  }
#pragma omp parallel for
  for (int i=0;i<100;i++) { 
    loop_code(i,a,b);       // each threads works on a portion of the loop
  }
#pragma omp parallel for
  for (int i=0;i<100;i++) {
    int j = i % 8;
    int v = value(i,a,b);
#pragma omp atomic
    c[j] += v;              // each thread makes an atomic update to c[j]
  }
  serial_code(a,b,c);
}
```

### Building and Running the Application

First clone the full repository, build the code for step 1, the run the application.
It is assumed that you run on machine with PGI Compiler 17.10 in the PATH.
```
git clone https://github.com/ibm/openacc-openmp-tutorial
cd openacc-openmp-tutorial
git fetch --all
git checkout step1
make
make run
make results
```

The application should run for a minute and the output should be similar to below, and pass validation.
```
enabling rivalism
iteration 0
[...]
iteration 15
[table with deviations from reference output]
[table with run-times]
```

### Profiling the application
We generate a profile of CPU-time used by the application using the following commands
```
SANDBOX="perf record -g" make run
perf report
```
Look at the resulting text-user-interface reporting fractions of run-time spent in specific functions, including functions called by them, and them, and ... (use arrow up/down, enter, escape)

### Your Tasks
* Find which functions in our application (Hint: descendants of function `main`) take the most run-time, look for them in the source code, and identify which of these sections may profit from parallelization. You should be able to identify a central loop containing most of the run-time.
  * Hints: Only look at [main.c](main.c), and [model.c](model.c)
* Assess which subfunctions, when executed in parallel by threads, may or will not influence each others execution, and therefore are safe (or unsafe) to parallelize. This may become easy to assess when you put yourself into the position of an insect and think about what actions you can take without impacting other insects.
* Add OpenMP `pragmas` to parallelize sections and bring down `iteration.time_total` from ~60 seconds to ~4.1 seconds, with a validation relative deviation in `kinetic_energy` less than `10^-8`. Use `make results` to check your progress.
  * Hint 1: No need to change `Makefile` or run-script `run`. They are setup to build and execute with 20 OpenMP threads.
  * Hint 2: Sometimes the number of required atomic updates may be reduced by splitting loops.
* Optional: Try a similar strategy with the loop in function `render.c:createImage`

## Next Step
Continue with [Step 2](../../blob/step2/step.md)






