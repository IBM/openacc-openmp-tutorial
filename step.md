# openacc-openmp-tutorial
This is an advanced tutorial to OpenACC and OpenMP.

## Step 4
In this step we are going to 
* resolve the warp divergence slowdown,
* learn about OpenACC data regions,
* and optimizing nested loops.

### Introduction to this step
In previous [Step 3](../../blob/step3/step.md) you have witnessed that for the kernel
```C
#pragma acc loop parallel
   for (int i=0;i<NumInsects;i++) {
      engage_enemies(i,insects,actions);
   }
``` 
execution times increased from 30ms for iteration 0 to 130ms for iteration 200, which was due to an increase in warp divergence of ~3x, starting with a "Warp Execution Efficiency" of 100% for iteration 0 down to ~30% in the later iterations. 

Your task was to think about why this happens. The explanation is that 
* when the simulation starts, each leader-insect has the same number of descendants, but as it progresses there will be leaders with more descendants than others, since some insects desert,
* and as an insect deserts, it arranges itself as a child to the insect it lost against. This results in a leadership hierarchy whose depth varies by insect: Some insects have only a few levels to go to reach its leader, some have many.

The result is that when two functions `engage_enemies(idx)` are executed in parallel for two different insects `idx_1` and `idx_2`, one will need to iterate more times through the function's loop than the other. Also, for each loop iteration child counts will be different, and the list of descendants of they child will also be different.
In the end, that paths taken though the leadership-graph by the two insects will be very different.

At this point one has to know that parallel threads on a GPU are executed in so-called *warps* (CUDA nomenclature, or *worker* in OpenACC nomenclature), and that all of these threads in a warp have to execute instructions in lock-step, (meaning that there can not be two or more different threads executing a different instruction in the same cycle). This forces threads to wait while other threads execute other instructions, before continuing in lock-step, and therefore the "Warp Execution Efficiency" goes down. The phenomenon is also known as "Branching", or "Warp Divergence".

This is no way of escaping this architectural restriction. Maximum efficiency can only be achieved if all of the threads execute identical instructions.

For this reason, in addition to the solution of [Step 3](../../blob/step3/step.md), an alternative method (i.e. implementation) of engaging enemies was added to the source code, but not yet enabled:
```C
void engage() {
   for (int i=0;i<NumLeaders;i++) {
      struct leader_data *l=&leaders[i];
      int ne=l->num_enemies;
      int nd=l->num_descendants;
      for (int d=0;d<nd;d++) {
         for (int e=0;e<ne;e++) {
            int attack=l->descendants[d];
            int defend=l->enemies[e];
            attack_defend_fight(attack,defend,insects,actions);
         }
      }
   }
}
```
This method may be enabled by uncommenting the commented calls to `engage()` and `identify_enemies()`, and removing the original calls to `engage_enemies(i,...)`.

As you can see the new implementation is a 3-level nested loop over the 3-tuple `(leader, descendant, enemy)`, and relies on the arrays `descendants` and `enemies` for each leader. Those arrays are filled head of theis new kernel using the function `identify_enemies()`.

### OpenACC Data Regions 
We're reusing the example from subdirectory [acc-example](acc-example) and add a `second_kernel()`:
```C
#define N 65536

float *w, a[4] = {0};
#pragma acc declare copyin(a)       // create memory space for variable a on device and copy to device once

#pragma acc routine                 // now function may be called from ACC region
float series(float x, const float *z, const float *w) {
	float r=0;
	for (int i=0;i<N;i++) 
		r += (((z[i]*x+a[0])*x+a[1])*x+a[2])*x+a[3]+w[i]*x;
	return r;
}

//no #pragma acc routine
void second_kernel(float *y, const float *z, const float *w) {
#pragma acc data present(y[0:N],z[0:N],w[0:N])  // needed for compiler to assign device fields to pointers
#pragma acc kernels                 // execution of code-block on GPU, compiler tries to derive appropriate method
	for (int i=0;i<N;i+=8)
	for (int j=0;j<N;j+=8)
#pragma acc atomic
		y[(i*j)%N]+=z[i]*w[j];
}

int main() {
        float y[N], z[N]={1};
	w=malloc(sizeof(*w)*N); w[0]=1;
	a[0]=1; a[1]=2; a[2]=3; a[3]=4;
#pragma acc update device(a)        // update variable a on device by those on host
#pragma acc data copyin(w[0:N]) \   // define data region; on entry create field w[0:N] on device by copy from host
                 copyin(z[0:N]) \   // on entry create z[0:N] on device by copy from host
                 copyout(y[0:N])    // on entry create uninitialized field y[0:N] on device, copy-out on exit
    {                               // data region starts here
#pragma acc kernels                 // execution of code-block on GPU, compiler tries to derive appropriate method
   	{
	        for (int i=0;i<N;i++)
        	        y[i]=series(i,z,w);
	}
	second_kernel(y,z,w);
    }                               // data region ends here
}
```

It is important to understand that the compiler requires the `#pragma acc data present` even though the `second_kernel()` is called with a `#pragma acc data` region: The compiler does not change the meaning of variables in such regions: Variables never become device-variables outside OpenACC compute constructs like `#pragma acc kernels` or `#pragma acc routine`.

Compared to the example in [Step 2](../../blob/step2/step.md), where the compiler infered `implicit copy(z[:]), implicit copyout(y[:])`, it still infers same for this example, but the parallel region for these constructs ends ahead of the `second_kernel`. Since the `second_kernel` requires the same arrays (the compiler being unable to infer this), we need to make these explicit here.

### Your Tasks
* Accelerate the execution of `engage()` using the OpenACC, bringing down the total model iteration time to `0.26s`. As earlier, have a validation relative deviation in `kinetic_energy` less than `10^-8`. Use `make results` to check your progress.
* Try to minimize data transfer host-to-device and device-to-host: Only open and close one OpenACC data region per iteration and call both kernels within the same data region.


### Hints
* Due to some miraculous coincident, the nested loop in function `engage()` has 3-levels, just like there are 3 levels of parallelization in OpenACC: gang, worker and vector. 
  You may be hint to the compiler on how it should treat the loops, by prefixing each loop with one or more of:
  ```C
  #pragma acc loop seq                          // loop is sequential or serial
  #pragma acc loop vector                       // distribute over CUDA-cores
  #pragma acc loop worker                       // distribute over warps
  #pragma acc loop gang                         // distribute over blocks
  #pragma acc loop (independent|parallel)       // force parallel
  #pragma acc loop collapse(n)                  // collapse the n layers of nested loops into one
  #pragma acc loop reduction(operator:variable) // combine loop-private variable using operator at end
  ```

## Next Step
Continue with [Step 5](../../blob/step5/step.md)






