#include "borderless.h"

#define USE_ITERATOR

B::Image entry_point(B::Application &app, B::Image img){
	auto t0 = borderless_clock();
	
#ifdef USE_ITERATOR
	B::ImageIterator it(img);
	u8 *pixel;
	while (it.next(pixel)){
		pixel[0] ^= 255;
		pixel[1] ^= 255;
		pixel[2] ^= 255;
	}
#else
	int w, h, stride, pitch;
	unsigned char *pixels;
	img.get_pixel_data(w, h, stride, pitch, pixels);
	int n = w * h;
	for (int i = 0; i < n; i++){
		pixels[0] ^= 255;
		pixels[1] ^= 255;
		pixels[2] ^= 255;
		pixels += 4;
	}
#endif

	auto t1 = borderless_clock();
	B::Stream() << "Filter took " << t1 - t0 << " s." << B::msgbox;
	return img;
}
