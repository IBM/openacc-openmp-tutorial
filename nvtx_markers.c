#include <stddef.h>
#include "nvtx_markers.h"
#include "nvToolsExt.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <limits.h>

typedef struct {
    double r;       // percent
    double g;       // percent
    double b;       // percent
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // percent
    double v;       // percent
} hsv;

rgb nvtx_hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

uint32_t rgb2nv(rgb c) {
	int r=(int)(c.r*255);
	int g=(int)(c.g*255);
	int b=(int)(c.b*255);
	return r<<16|g<<8|b;
}

uint32_t fnv1a_hash(const char *s) {
	int n=strlen(s);
	uint32_t h=0;//0x811C9DC5;
	uint32_t c;
	while (c = *s++)
		h=(c ^ h) * 0x01000193;
	return h;
}

uint32_t nv_color(const char* s) {
	double f=fnv1a_hash(s)*360.0/UINT_MAX;
	hsv x={f,0.75,0.75};
	rgb c=nvtx_hsv2rgb(x);
	return rgb2nv(c);
}


void nv_push_range(const char* name) {
	nvtxEventAttributes_t eventAttrib = {0};
	eventAttrib.version = NVTX_VERSION;
	eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
	eventAttrib.colorType = NVTX_COLOR_ARGB;
	eventAttrib.color = nv_color(name);
	eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
	eventAttrib.message.ascii = name;
	nvtxRangePushEx(&eventAttrib);
}

void nv_push_range_f(const char* fmt, ...) {
	char name[4096];
	va_list args;
	va_start(args,fmt);
	vsprintf(name,fmt,args);
	va_end(args);
	nvtxEventAttributes_t eventAttrib = {0};
	eventAttrib.version = NVTX_VERSION;
	eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
	eventAttrib.colorType = NVTX_COLOR_ARGB;
	eventAttrib.color = nv_color(fmt);
	eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
	eventAttrib.message.ascii = name;
	nvtxRangePushEx(&eventAttrib);
}

void nv_pop_range() {
	nvtxRangePop();
}

