/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "floyd_steinberg.h"
#include "misc.h"

template <typename ColorSpace, size_t N>
B::Image floyd_steinberg(
		B::Image &img,
		const std::vector<ColorSpace> &palette,
		const typename ColorSpace::number_type (&kernel_coefficients)[N],
		const std::pair<int, int> (&kernel_offsets)[N]){
	typedef typename ColorSpace::number_type Float;
	auto ret = img.clone();
	
	int w, h, stride, pitch;
	unsigned char *pixels;
	
	ret.get_pixel_data(w, h, stride, pitch, pixels);
	
	int min_x = 0,
		max_x = 0,
		max_y = 0;
		
	for (auto &offset : kernel_offsets){
		min_x = std::min(min_x, offset.first);
		max_x = std::max(max_x, offset.first);
		max_y = std::max(max_y, offset.second);
	}
	
	auto w0 = w + (max_x - min_x);
	auto h0 = h + max_y;
	
	int offsets[N];
	int offsets_inverted[N];
	for (size_t i = 0; i < N; i++){
		offsets[i] = kernel_offsets[i].first + kernel_offsets[i].second * w0;
		offsets_inverted[i] = -kernel_offsets[i].first + kernel_offsets[i].second * w0;
	}
	
	std::vector<ColorSpace> intermediate_pixels(w0 * h0);
	auto ip = &intermediate_pixels[-min_x];

	for (int y = 0; y < h; y++){
		auto row_pointer = pixels + pitch * y;
		for (int x = 0; x < w; x++){
			auto pixel = row_pointer + x * 4;
			ip[x + y * w0] = (ColorSpace)RgbInt(pixel[0], pixel[1], pixel[2]);
		}
	}

	for (int y = 0; y < h; y++){
		int start = 0;
		int finish = w;
		bool invert = y % 2 != 0;
		int step = 1;
		int *offsets_p = offsets;
		if (invert){
			step = -1;
			start = w - 1;
			finish = -1;
			offsets_p = offsets_inverted;
		}

		auto row = ip + y * w0;

		for (int x = start; x != finish; x += step){
			auto &pixel = row[x];
			auto old_color = pixel.saturate_channels();
			auto new_color = pick_color(pixel, palette);

			pixel = new_color;
			auto quantization_error = old_color - new_color;
			
			for (size_t i = 0; i < N; i++)
				row[x + offsets_p[i]] += quantization_error * kernel_coefficients[i];
		}
	}

	for (int y = 0; y < h; y++){
		auto row_pointer = pixels + pitch * y;
		auto row = ip + y * w0;
		for (int x = 0; x < w; x++){
			auto pixel = row_pointer + x * 4;
			auto s = (RgbInt)row[x];
			pixel[0] = s.r;
			pixel[1] = s.g;
			pixel[2] = s.b;
		}
	}
	
	return ret;
}

template <typename ColorSpace>
B::Image floyd_steinberg(B::Image &img, const std::vector<ColorSpace> &palette){
	typedef typename ColorSpace::number_type Float;
	const Float kernel_coefficients[] = {
		(Float)(7.0/16.0),
		(Float)(3.0/16.0),
		(Float)(5.0/16.0),
		(Float)(1.0/16.0),
	};
	const std::pair<int, int> kernel_offsets[] = {
		{  1, 0 },
		{ -1, 1 },
		{  0, 1 },
		{  1, 1 },
	};
	return floyd_steinberg(img, palette, kernel_coefficients, kernel_offsets);
}

template <typename ColorSpace>
B::Image floyd_steinberg_bell(B::Image &img, const std::vector<ColorSpace> &palette){
	typedef typename ColorSpace::number_type Float;
	const Float kernel_coefficients[] = {
		(Float)(7.0/48.0),
		(Float)(5.0/48.0),
		(Float)(3.0/48.0),
		(Float)(5.0/48.0),
		(Float)(7.0/48.0),
		(Float)(5.0/48.0),
		(Float)(3.0/48.0),
		(Float)(1.0/48.0),
		(Float)(3.0/48.0),
		(Float)(5.0/48.0),
		(Float)(3.0/48.0),
		(Float)(1.0/48.0),
	};
	const std::pair<int, int> kernel_offsets[] = {
		{  1, 0 },
		{  2, 0 },
		{ -2, 1 },
		{ -1, 1 },
		{  0, 1 },
		{  1, 1 },
		{  2, 1 },
		{ -2, 2 },
		{ -1, 2 },
		{  0, 2 },
		{  1, 2 },
		{  2, 2 },
	};
	return floyd_steinberg(img, palette, kernel_coefficients, kernel_offsets);
}
