// This example applies a Floyd-Steinberg dither with 2 colors on the selected
// image. It runs over 5 times faster than the Lua implementation, excluding
// compilation time.

#include "borderless.h"
#include <cmath>

typedef float pixel_t;

pixel_t f(int x, const std::vector<int> &colors){
	float pos = x / 255.f * (colors.size() - 1);
	int which = floor(pos + 0.5f);
	return (pixel_t)colors[which];
}

std::vector<int> construct_colors(int count){
	std::vector<int> ret(count);
	for (int i = 0; i < count; i++)
		ret[i] = 255 * i / (count - 1);
	return ret;
}

pixel_t luma(int r, int g, int b){
	return (r * 2126 + g * 7152 + b * 722) / (pixel_t)10000;
}

std::vector<pixel_t> construct_pixels(B::Image &img){
	int w, h;
	img.get_dimensions(w, h);
	
	std::vector<pixel_t> pixels(w * h);

	B::ImageIterator it(img);
	u8 *pixel;
	int i = 0;
	while (it.next(pixel))
		pixels[i++] = luma(pixel[0], pixel[1], pixel[2]);
	
	return pixels;
}

void compute_dither(B::Image &img, std::vector<pixel_t> &pixels, const std::vector<int> &colors){
	int w, h;
	img.get_dimensions(w, h);
	
	static const int offsets[][2] = {
		{  1, 0 },
		{ -1, 1 },
		{  0, 1 },
		{  1, 1 },
	};
	static const float weights[][4] = {
		{        0,        0,        0,        0 },
		{        0,        1,        0,        0 },
		{ 7.f/16.f,        0,        0,        0 },
		{ 8.f/16.f,        0, 6.f/16.f, 2.f/16.f },
		{        0,        0,        0,        0 },
		{        0, 7.f/16.f, 9.f/16.f,        0 },
		{ 7.f/16.f,        0,        0,        0 },
		{ 7.f/16.f, 3.f/16.f, 5.f/16.f, 1.f/16.f }
	};
	
	bool errored = false;
	
	for (int y = 0; y < h; y++){
		int start = 0;
		int finish = w - 1;
		bool invert = y % 2 != 0;
		int step = invert ? -1 : 1;
		if (invert){
			start = w - 1;
			finish = -1;
		}
		for (int x = start; x != finish; x += step){
			pixel_t oldpixel = pixels[x + y * w];
			if (oldpixel > 255)
				oldpixel = 255;
			else if (oldpixel < 0)
				oldpixel = 0;
			pixel_t newpixel = f(oldpixel, colors);
			pixels[x + y * w] = newpixel;
			pixel_t qe = oldpixel - newpixel;
			int weight_chosen;
			if (!invert)
				weight_chosen = (!!x) * 4 + (x != w - 1) * 2 + (y != h - 1);
			else
				weight_chosen = (!!x) * 2 + (x != w - 1) * 4 + (y != h - 1);
			
			for (int i = 0; i < 4; i++){
				float weight = weights[weight_chosen][i];
				if (weight <= 0)
					continue;
				int x2 = x + offsets[i][0] * (!invert * 2 - 1);
				int y2 = y + offsets[i][1];
				int j = x2 + y2 * w;
				if (((!invert && y2 == y && x2 < x) || (invert && y2 == y && x2 > x)) && !errored){
					B::Stream() << "Internal error detected!" << B::msgbox;
					errored = true;
				}
				pixels[j] += qe * weight;
			}
		}
	}
}

void copy_back(B::Image &img, const std::vector<float> &pixels){
	B::ImageIterator it(img);
	u8 *pixel;
	int i = 0;
	while (it.next(pixel)){
		auto c = (int)floor(pixels[i++]);
		pixel[0] = c;
		pixel[1] = c;
		pixel[2] = c;
	}
}

void copy_back(B::Image &img, const std::vector<int> &pixels){
	B::ImageIterator it(img);
	u8 *pixel;
	int i = 0;
	while (it.next(pixel)){
		auto c = pixels[i++];
		pixel[0] = c;
		pixel[1] = c;
		pixel[2] = c;
	}
}

void apply_floyd_steinberg(B::Image &img, int color_count){
	auto colors = construct_colors(color_count);
	auto pixels = construct_pixels(img);
	compute_dither(img, pixels, colors);
	copy_back(img, pixels);
}

B::Image entry_point(B::Application &app, B::Image img){
	auto t0 = borderless_clock();
	
	apply_floyd_steinberg(img, 2);

	auto t1 = borderless_clock();
	B::Stream() << "Filter took " << t1 - t0 << " s." << B::msgbox;
	return img;
}
