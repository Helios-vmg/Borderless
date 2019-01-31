/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "quantization.h"
#include <cmath>
#include <utility>
#include <algorithm>

template <typename ColorSpace>
struct Point{
	ColorSpace vector;
	int count = 1;
};

template <typename ColorSpace>
struct Cluster{
	ColorSpace center;
	std::vector<Point<ColorSpace> *> points;
	int count;
};

template <typename ColorSpace>
std::vector<Point<ColorSpace>> get_points(B::Image &img){
	std::vector<Point<ColorSpace>> ret;
	
	typedef RgbFloat<typename ColorSpace::number_type> Rgb;

	std::vector<std::pair<int, ColorSpace>> colors;
	std::vector<int> indices(0x1000000, -1);

	B::ImageIterator it(img);
	u8 *pixel;
	while (it.next(pixel)){
		if (!pixel[3])
			continue;
		RgbInt rgb(pixel[0], pixel[1], pixel[2]);
		auto index = rgb.get_int();
		auto &i = indices[index];
		if (i >= 0)
			std::get<0>(colors[i])++;
		else{
			i = colors.size();
			colors.push_back(std::make_pair(1, (ColorSpace)rgb));
		}
	}

	for (auto &pair : colors){
		Point<ColorSpace> p;
		p.count = pair.first;
		//p.color = pair.second;
		p.vector = pair.second;
		ret.push_back(p);
	}

	return ret;
}

template <typename ColorSpace>
std::list<Cluster<ColorSpace>> initialize_clusters(std::vector<Point<ColorSpace>> &points, int colors, B::XorShift128 &rng){
	std::vector<Point<ColorSpace>> points2;
	points2.reserve(points.size());
	for (auto &point : points)
		points2.push_back(point);
	std::vector<size_t> indices(points2.size());
	for (size_t i = 0; i < points2.size(); i++)
		indices[i] = i;
	B::random_shuffle(indices.begin(), indices.end(), rng);
	std::sort(indices.begin(), indices.end(), [&points2](size_t a, size_t b){ return points2[a].vector.brightness() < points2[b].vector.brightness(); });
	std::list<Cluster<ColorSpace>> ret;
	for (int i = colors; i--;){
		Cluster<ColorSpace> c;
		c.center = points2[indices[i * (indices.size() - 1) / (colors - 1)]].vector;
		ret.push_back(c);
	}
	return ret;
}

template <typename ColorSpace>
void fill_clusters(std::list<Cluster<ColorSpace>> &ret, std::vector<Point<ColorSpace>> &points){
	typedef typename ColorSpace::number_type Float;
	for (auto &c : ret)
		c.points.clear();
	for (size_t a = 0; a < points.size(); a++){
		Float min_d = 0;
		Cluster<ColorSpace> *min_it = nullptr;
		for (auto &i : ret){
			auto d = points[a].vector.distance_sq(i.center);
			if (!min_it || d < min_d){
				min_d = d;
				min_it = &i;
			}
		}
		min_it->points.push_back(&points[a]);
	}
}

template <typename ColorSpace>
void kmeans(std::list<Cluster<ColorSpace>> &clusters, std::vector<Point<ColorSpace>> &points, double threshold = 0.5 / 255.0){
	typedef typename ColorSpace::number_type Float;
	const Float fthreshold = (Float)threshold * (Float)threshold;
	Float max_distance = 0;
	do{
		fill_clusters(clusters, points);
		std::vector<ColorSpace> new_vectors(clusters.size());
		size_t a = 0;
		for (auto &c : clusters){
			Float count = 0;
			for (auto &v : c.points){
				new_vectors[a] += v->vector * v->count;
				count += v->count;
			}
			if (count)
				new_vectors[a] *= 1 / count;
			a++;
		}
		max_distance = 0;
		a = 0;
		for (auto &c : clusters){
			if (c.points.size()){
				double d = c.center.distance_sq(new_vectors[a]);
				if (d > max_distance)
					max_distance = d;
			}
			a++;
		}
		a = 0;
		for (auto &c : clusters)
			c.center = new_vectors[a++];
	}while (max_distance < fthreshold);
}

template <typename ColorSpace>
std::vector<ColorSpace> cluster_image(B::Image &img, int colors, B::XorShift128 &rng){
	auto points = get_points<ColorSpace>(img);
	auto clusters = initialize_clusters(points, colors, rng);
	kmeans(clusters, points);
	std::vector<ColorSpace> quantized_colors;
	quantized_colors.reserve(clusters.size());
	for (auto &cluster : clusters)
		quantized_colors.push_back(cluster.center);
	return quantized_colors;
}

template <typename ColorSpace>
ColorSpace pick_color(const ColorSpace &color, const std::vector<ColorSpace> &clusters){
	typedef typename ColorSpace::number_type Float;
	Float min = 0;
	const ColorSpace *selected = nullptr;
	for (auto &cluster : clusters){
		auto d = color.distance_sq(cluster, 5);
		if (!selected || d < min){
			min = d;
			selected = &cluster;
		}
	}
	return *selected;
}

template <typename ColorSpace>
B::Image quantize_image(B::Image &img, int steps, const std::vector<ColorSpace> &palette){
	auto ret = img.clone();
	
	B::ImageIterator it(img);
	u8 *pixel;
	while (it.next(pixel)){
		if (!pixel[3])
			continue;
		auto color = (ColorSpace)RgbInt(pixel[0], pixel[1], pixel[2]);
		auto s = (RgbInt)pick_color(color, palette);
		pixel[0] = s.r;
		pixel[1] = s.g;
		pixel[2] = s.b;
	}
	return ret;
}
template <typename ColorSpace>
std::vector<ColorSpace> interpolate_palette(const ColorSpace &first_color, const ColorSpace &second_color, int colors){
	typedef typename ColorSpace::number_type Float;
	std::vector<ColorSpace> ret;
	ret.reserve(colors);
	auto v = (second_color - first_color) * (Float)(1.0 / (colors - 1));
	for (int i = 0; i < colors; i++)
		ret.push_back(first_color + v * (Float)i);
	return ret;
}
