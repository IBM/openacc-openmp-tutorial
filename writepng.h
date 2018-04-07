
struct image {
	struct rgb* buffer;
	int width, height;
};

struct rgb {
	float r,g,b;
};

struct hsv {
	float h,s,v;
};

struct rgb hsv2rgb(struct hsv in);
int writeImage(const char* filename, const struct image* img, const char* title);
