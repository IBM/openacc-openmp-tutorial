#ifndef NVTX_MARKERS_H
#define NVTX_MARKERS_H

#ifdef USE_NVTX
void nv_push_range(const char* name);
void nv_push_range_f(const char* fmt, ...);
void nv_pop_range();
#define NV_PUSH_RANGE(name) nv_push_range(name)
#define NV_PUSH_RANGE_F(...) nv_push_range_f(__VA_ARGS__)
#define NV_POP_RANGE() nv_pop_range()
#else
#define NV_PUSH_RANGE(name)
#define NV_PUSH_RANGE_F(...)
#define NV_POP_RANGE()
#endif

#endif

