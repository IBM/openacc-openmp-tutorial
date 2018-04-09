#include <malloc.h>

#define N 65536

float *w, a[4] = {0};
#pragma acc declare copyin(a)

#pragma acc routine
float series(float x, const float *z, const float *w) {
	float r=0;
	for (int i=0;i<N;i++) 
		r += (((z[i]*x+a[0])*x+a[1])*x+a[2])*x+a[3]+w[i]*x;
	return r;
}

void first_kernel(float *y, const float *z, const float *w) {
#pragma acc data async(1) present(y[0:N],z[0:N],w[0:N])
#pragma acc kernels async(1)
	for (int i=0;i<N;i++)
		y[i]=series(i,z,w);
}

void second_kernel(float *y, const float *z, const float *w) {
#pragma acc data async(2) present(y[0:N],z[0:N],w[0:N])
#pragma acc kernels async(2)
	for (int i=0;i<N;i+=64)
	for (int j=0;j<N;j+=64)
#pragma acc atomic
		y[(i*j)%N]+=z[i]*w[j];
}

void third_kernel(float *y, const float *z, const float *w, const float *x) {
#pragma acc data async(4) wait(1,2,3) present(y[0:N],z[0:N],w[0:N])
#pragma acc kernels async(4)
	for (int i=0;i<N;i++)
		y[i]=z[i]*w[i]*x[i];
}

void host_kernel(float *y, const float *z, const float *w) {
	for (int i=0;i<N;i++)
		y[i]=log(abs(sin(z[i])+cos(w[i])));
}

int main() {
        float y[N], y1[N], y2[N], y3[N], z[N]={1};
	w=malloc(sizeof(*w)*N); w[0]=1;
	a[0]=1; a[1]=2; a[2]=3; a[3]=4;
#pragma acc update device(a)
// define data region
// on entry create fields w[0:N], z[0:N] on device by copy-in from host
// on entry create uninitialized field y[0:N] on device, copy-out on exit
// on entry create uninitialized fields y1[0:N], y2[0:N] on device
#pragma acc data copyin(w[0:N],z[0:N])  \
                 copyout(y[0:N]) \
                 create(y1[0:N],y2[0:N])
    {
	first_kernel(y1,z,w);
	second_kernel(y2,z,w);
	host_kernel(y3,z,w);
#pragma acc data async(3) copyin(y3[0:N])
	third_kernel(y,y1,y2,y3);
#pragma acc wait
    }
}
