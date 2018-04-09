#include <malloc.h>

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
// define data region; on entry create field w[0:N] on device by copy from host
// on entry create z[0:N] on device by copy from host
// on entry create uninitialized field y[0:N] on device, copy-out on exit
#pragma acc data copyin(w[0:N]) \
                 copyin(z[0:N]) \
                 copyout(y[0:N])
    {                               // data region starts here
#pragma acc kernels                 // execution of code-block on GPU, compiler tries to derive appropriate method
   	{
	        for (int i=0;i<N;i++)
        	        y[i]=series(i,z,w);
	}
	second_kernel(y,z,w);
    }                               // data region ends here
}
