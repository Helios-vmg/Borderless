/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "capi.h"
#include "ImageStore.h"
#include "PluginCoreState.h"
#include <QtWidgets/QMessageBox>
#include <ctime>
#include <cmath>
#include <sstream>
#include <random>
#ifdef WIN32
#include <Windows.h>
#undef max
#undef min
#endif

const double pi = 3.1415926535897932384626433832795;
const double tau = pi * 2;
const float pif = 3.1415926535897932384626433832795f;
const float tauf = pif * 2;

EXPORT_C Image *load_image(PluginCoreState *state, const char *path){
	return state->get_store().load_image(path);
}

EXPORT_C Image *allocate_image(PluginCoreState *state, int w, int h){
	return state->get_store().allocate_image(w, h);
}

EXPORT_C Image *clone_image(Image *image){
	int w, h;
	unsigned stride, pitch, s, p;
	image->get_dimensions(w, h);
	auto src = image->get_pixels_pointer(stride, pitch);
	auto ret = image->get_owner()->allocate_image(w, h);
	auto dst = ret->get_pixels_pointer(s, p);
	memcpy(dst, src, h * pitch);
	return ret;
}

EXPORT_C Image *clone_image_without_data(Image *image){
	int w, h;
	image->get_dimensions(w, h);
	return image->get_owner()->allocate_image(w, h);
}

EXPORT_C void unload_image(Image *image){
	auto store = image->get_owner();
	store->unload(image);
}

EXPORT_C void get_image_dimensions(Image *image, int *w, int *h){
	image->get_dimensions(*w, *h);
}

EXPORT_C u8 *get_image_pixel_data(Image *image, int *stride, int *pitch){
	unsigned temp1, temp2;
	auto ret = (u8 *)image->get_pixels_pointer(temp1, temp2);
	*stride = temp1;
	*pitch = temp2;
	return ret;
}


EXPORT_C Image *get_displayed_image(PluginCoreState *state){
	auto handle = state->get_caller_image_handle();
	return state->get_store().get_image(handle).get();
}

EXPORT_C void display_in_current_window(PluginCoreState *state, Image *image){
	state->display_in_caller(image);
}

template <typename T>
T max(const T &a, const T &b, const T &c){
	return std::max(a, std::max(b, c));
}

template <typename T>
T min(const T &a, const T &b, const T &c){
	return std::min(a, std::min(b, c));
}

template <typename T>
T fmod2(T x, T y){
	if (x < 0)
		return fmod(y - fmod(x, y), y);
	return fmod(x, y);
}

EXPORT_C void rgb_to_hsv(hsv_quad *dst, u8_quad src){
	auto r = src.data[0] * float(1.f / 255.f);
	auto g = src.data[1] * float(1.f / 255.f);
	auto b = src.data[2] * float(1.f / 255.f);
	auto a = src.data[3] * float(1.f / 255.f);
	auto &h = dst->hue;
	auto &s = dst->saturation;
	auto &v = dst->value;
	auto max = ::max(r, g, b);
	auto min = ::min(r, g, b);
	auto chroma = max - min;
	if (!chroma)
		h = 0;
	else if (max == r)
		h = (g - b) / chroma;
	else if (max == g)
		h = (b - r) / chroma + 2;
	else
		h = (r - g) / chroma + 4;
	h = fmod2<float>(h, 6) * (tauf / 6.f);
	v = max;
	s = !dst->value ? 0 : chroma / dst->value;
	dst->alpha = a;
}

static void set_rgb(u8 &r, u8 &g, u8 &b, float x, float y, float z){
	r = (u8)(x * 255);
	g = (u8)(y * 255);
	b = (u8)(z * 255);
}

EXPORT_C void hsv_to_rgb(u8_quad *dst, const hsv_quad *src){
	auto h = src->hue;
	auto s = src->saturation;
	auto v = src->value;
	auto &r = dst->data[0];
	auto &g = dst->data[1];
	auto &b = dst->data[2];
	dst->data[3] = (unsigned char)(src->alpha * 255.f);
	auto h2 = h * (6.f / tauf);
	auto chroma = s * v;
	auto x = chroma * (1 - fabs(fmod2<float>(h2, 2) - 1));

	typedef void (*f)(u8 &r, u8 &g, u8 &b, float c, float x);
	static const f functions[] = {
		[](u8 &r, u8 &g, u8 &b, float c, float x){ set_rgb(r, g, b, c, x, 0); },
		[](u8 &r, u8 &g, u8 &b, float c, float x){ set_rgb(r, g, b, x, c, 0); },
		[](u8 &r, u8 &g, u8 &b, float c, float x){ set_rgb(r, g, b, 0, c, x); },
		[](u8 &r, u8 &g, u8 &b, float c, float x){ set_rgb(r, g, b, 0, x, c); },
		[](u8 &r, u8 &g, u8 &b, float c, float x){ set_rgb(r, g, b, x, 0, c); },
		[](u8 &r, u8 &g, u8 &b, float c, float x){ set_rgb(r, g, b, c, 0, x); },
	};

	functions[(int)h](r, g, b, chroma, x);
}

#ifdef WIN32
EXPORT_C void debug_print(const char *string){
	auto w = QString::fromUtf8(string).toStdWString();
	OutputDebugStringW(w.c_str());
#else
EXPORT_C void debug_print(const char *){
#endif
}

EXPORT_C void show_message_box(const char *string){
	QMessageBox msgbox;
	msgbox.setText(QString::fromUtf8(string));
	msgbox.exec();
}

EXPORT_C void random_seed(uint32_t *dst){
	std::random_device dev;
	for (int i = 4; i--;)
		dst[i] = dev();
}

EXPORT_C int save_image(Image *image, const char *path){
	image->save(QString::fromUtf8(path), SaveOptions());
	return false;
}

EXPORT_C double borderless_clock(){
	return clock() / (double)CLOCKS_PER_SEC;
}

EXPORT_C char *double_to_string(double x){
	std::stringstream stream;
	stream << x;
	auto s = stream.str();
	auto ret = new char[s.size() + 1];
	ret[s.size()] = 0;
	memcpy(ret, &s[0], s.size());
	return ret;
}

EXPORT_C void release_double_to_string(char *s){
	delete[] s;
}
