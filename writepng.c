#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <png.h>
#include <stdint.h>

#include "support.h"
#include "writepng.h"

struct rgb hsv2rgb(struct hsv in)
{
	double hh, p, q, t, ff;
	long i;
	struct rgb out;

	if(in.s <= 0.0) {
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h*6;
	if(hh >= 6) hh = 0;
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

void setRGB(png_byte *ptr, struct rgb val)
{
	ptr[0]=MIN(val.r,1)*255;
	ptr[1]=MIN(val.g,1)*255;
	ptr[2]=MIN(val.b,1)*255;
}

void normalizeImage(struct image* img) {
	int width=img->width;
	int height=img->height;
	struct rgb* buffer=img->buffer;
	struct rgb rgb,max={0};
	int x,y;
	for (y=0 ; y<height ; y++) {
		for (x=0 ; x<width ; x++) {
			rgb=buffer[y*width + x];
			max.r=MAX(max.r,rgb.r);
			max.g=MAX(max.g,rgb.g);
			max.b=MAX(max.b,rgb.b);
		}
	}
	float f=1.0/MAX(MAX(max.r,max.g),max.b);
	for (y=0 ; y<height ; y++) {
		for (x=0 ; x<width ; x++) {
			buffer[y*width + x].r*=f;
			buffer[y*width + x].g*=f;
			buffer[y*width + x].b*=f;
		}
	}
}

int writeImage(const char* filename, const struct image* img, const char* title)
{
	int width=img->width;
	int height=img->height;
	struct rgb* buffer=img->buffer;
	int retval = 0;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		retval = 1;
		goto abort;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		retval = 1;
		goto abort;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		retval = 1;
		goto abort;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		retval = 1;
		goto abort;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	if (title != NULL) {
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = "Title";
		title_text.text = (char*)title;
		png_set_text(png_ptr, info_ptr, &title_text, 1);
	}

	png_write_info(png_ptr, info_ptr);

	row = (png_bytep) malloc(3 * width * sizeof(png_byte));

	int x, y;
	for (y=0 ; y<height ; y++) {
		for (x=0 ; x<width ; x++) {
			setRGB(&(row[x*3]), buffer[y*width + x]);
		}
		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, NULL);

abort:
	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) free(row);

	return retval;
}

