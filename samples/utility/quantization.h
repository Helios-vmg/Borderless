/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#pragma once

#include "color_space.h"

/*
 * Computes an optimal color palette of a given number of colors for the given
 * image. The algorithm used is k-means with semi-random initialization.
 * The returned palette is only guaranteed to contain *at-most* the specified
 * number of colors. The algorithm is allowed to converge to smaller palettes.
 * Valid types for ColorSpace are RgbFloat<T>, Grayscale<T>, Cielab<T>, where T
 * is any floating point type. float generally performs better than double, at
 * no cost in quality.
 */
template <typename ColorSpace>
std::vector<ColorSpace> cluster_image(B::Image &img, int colors, B::XorShift128 &rng);

template <typename ColorSpace>
std::vector<ColorSpace> cluster_image(B::Image &img, int colors){
	B::XorShift128 rng;
	return cluster_image(img, colors, rng);
}

/*
 * Generates a palette of uniformly distributed colors along a straight line in
 * the specified color space from first_color to second_color.
 */
template <typename ColorSpace>
std::vector<ColorSpace> interpolate_palette(
	const ColorSpace &first_color,
	const ColorSpace &second_color,
	int colors
);

/*
 * Quantizes the image using the specified palette. No dithering is performed.
 */
template <typename ColorSpace>
B::Image quantize_image(B::Image &img, const std::vector<ColorSpace> &palette);

#include "quantization.cpp"
