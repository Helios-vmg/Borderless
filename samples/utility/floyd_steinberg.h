/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#pragma once

#include "quantization.h"

/*
 * Quantizes the image using the specified palette and applies Floyd-Steinberg
 * dithering. This variant uses zigzag scanning and the original Floyd-Steinberg
 * error diffusion kernel (7-3-5-1).
 */
template <typename ColorSpace>
B::Image floyd_steinberg(B::Image &img, const std::vector<ColorSpace> &palette);

/*
 * Same as the previous function, but also generates the palette.
 * Valid types for ColorSpace are RgbFloat<T>, Grayscale<T>, Cielab<T>, where T
 * is any floating point type. float generally performs better than double, at
 * no cost in quality.
 * RgbFloat<T> uses the RGB space to generate the palette and perform the
 * quantization. This generates an image with fairly good color fidelity, but
 * the dithering is rather noticeable.
 * Grayscale<T> generates a grayscale image. RGB values are mapped to grayscale
 * using luma (v = r * 0.2126 + g * 0.7152 + b * .0722), which should closely
 * approximate human perception.
 * Cielab<T> uses the CIELAB color space to generate the palette and quantize
 * the image. This variant tipically results in the subjectively best-looking
 * images, in that the dithering becomes almost unnoticeable. This comes at the
 * cost of color fidelity. The overall hue of the image will typically change
 * somewhat, for example towards red or purple. This is usually only noticeable
 * when placed side-by-side with the original.
 */
template <typename ColorSpace>
B::Image floyd_steinberg(B::Image &img, int steps, B::XorShift128 &rng){
	return floyd_steinberg(img, cluster_image<ColorSpace>(img, steps, rng));
}

template <typename ColorSpace>
B::Image floyd_steinberg(B::Image &img, int steps){
	B::XorShift128 rng;
	return floyd_steinberg<ColorSpace>(img, steps, rng);
}

/*
 * Same as floyd_steinberg(), but uses the Jarvis-Judice-Ninke error diffusion
 * kernel.
 */
template <typename ColorSpace>
B::Image floyd_steinberg_bell(B::Image &img, const std::vector<ColorSpace> &palette);

#include "floyd_steinberg.cpp"
