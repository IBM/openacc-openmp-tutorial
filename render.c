
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <limits.h>

#include "model.h"
#include "support.h"
#include "writepng.h"

#define M_PI 3.14159265358979323846

void drawPixel(struct image* img, float x, float y, struct rgb rgb) {
		int width=img->width;
		int height=img->height;
		struct rgb* buffer=img->buffer;
		int yPos=y+0.5;
		int xPos=x+0.5;
		if (xPos>=0&&xPos<width&&yPos>=0&&yPos<height) {
			buffer[yPos * width + xPos].r += rgb.r;
			buffer[yPos * width + xPos].g += rgb.g;
			buffer[yPos * width + xPos].b += rgb.b;
		}
}

void drawPoint(struct image* img, float x, float y, float z, struct rgb rgb, float scale, float ca, float sa) {
	float xx,yy,zz;
	float theta=-M_PI/8;
	float ct=cos(theta);
	float st=sin(theta);
	//xz angle (rout around y axis)
	xx=x*ca-z*sa; 
	zz=x*sa+z*ca;
	x=xx;
	z=zz;
	//yz angle, (rot around x axis)
	yy=y*ct-z*st;
	zz=y*st+z*ct;
	y=yy;
	z=zz;
	
	
	x=img->width*0.5+x*scale;
	y=img->height*0.5+y*scale;
	//z=center+z*scale;
	drawPixel(img,x,y,rgb);
}

void drawLine(struct image* img, float x1, float y1, float z1, float x2, float y2, float z2, struct rgb rgb, float scale, float ca, float sa) {
	int steps=100;
	double x,y,z;
	for (double j=0;j<1;j+=1.0/steps) {
		x=x1+j*(x2-x1);
		y=y1+j*(y2-y1);
		z=z1+j*(z2-z1);
		drawPoint(img,x,y,z,rgb,scale,ca,sa);
	}
}
void destroyImage(struct image* img) {
	free(img->buffer);
	free(img);
}

struct image* createImage(int width, int height, float angle, float max)
{
	struct image* img = (struct image*) malloc(sizeof(struct image));
	img->width=width;
	img->height=height;
	int bufsize=width * height * sizeof(struct rgb);
	img->buffer= (struct rgb *) malloc(bufsize);
	if (img->buffer == NULL) {
		fprintf(stderr, "Could not create image buffer\n");
		return NULL;
	}
	memset(img->buffer,0, bufsize);

	float sa=sin(angle);
	float ca=cos(angle);
	float scale=img->width*0.5/max;
	struct hsv hsv;
	struct rgb rgb;
	hsv.s=0;
	hsv.v=0.5;
	hsv.h=0;
	rgb=hsv2rgb(hsv);
	drawLine(img,-max,0,0,max,0,0,rgb,scale,ca,sa);
	drawLine(img,0,-max,0,0,max,0,rgb,scale,ca,sa);
	drawLine(img,0,0,-max,0,0,max,rgb,scale,ca,sa);

	for (int i=0;i<NumInsects;i++) {
		hsv.s=1;
		hsv.v=0.2;
		hsv.h=0;
		int leader_id=insects[i].leader_id;
		if (leader_id!=-1) {
			hsv.h=leaders[leader_id].hue;
		}
		rgb=hsv2rgb(hsv);
		struct insect_data *pi,*pj;
		pi=&insects[i];
		if (pi->parent>=0) {
			pj=&insects[pi->parent];
			drawLine(img,pi->x,pi->y,pi->z,pj->x,pj->y,pj->z,rgb,scale,ca,sa);
		}
	}
	return img;
}

void save_image(int i) 
{
	const char* title="";
	int s=section_start("image");
	int width = 1920;
	int height = 1080;
	float max = 80;
        char filename[1024];
        sprintf(filename,"%s/iteration.%04d.png",params.output_dir,i);
	float angle=2*M_PI*i/720;
	struct image* img = createImage(width,height,angle,max);
	//normalizeImage(a,a,buffer);
	int result = writeImage(filename, img, title);
	destroyImage(img);
	section_end(s);
}

