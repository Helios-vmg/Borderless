/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef CAPI_H
#define CAPI_H

#ifndef __cplusplus
#define EXTERN_C
#else
#define EXTERN_C extern "C"
#endif

#include <stdint.h>

#if defined WIN32 && defined BUILDING_BORDERLESS
#define EXPORT_C EXTERN_C __declspec(dllexport)
#else
#define EXPORT_C EXTERN_C
#endif

#if defined BUILDING_BORDERLESS || defined __cplusplus
class PluginCoreState;
class Image;
#else
typedef void PluginCoreState;
typedef void Image;
#endif

typedef unsigned char u8;

struct u8_quad{
	u8 data[4];
};

typedef struct u8_quad u8_quad;

/* Note: Paths must be UTF-8 strings.*/

/* Image constructors. */

EXPORT_C Image *load_image(PluginCoreState *state, const char *path);
EXPORT_C Image *allocate_image(PluginCoreState *state, int w, int h);
EXPORT_C Image *clone_image(Image *image);
EXPORT_C Image *clone_image_without_data(Image *image);
EXPORT_C void unload_image(Image *image);


/* Image property accessors. */

EXPORT_C void get_image_dimensions(Image *image, int *w, int *h);
EXPORT_C u8 *get_image_pixel_data(Image *image, int *stride, int *pitch);


/* Image display functions. */
EXPORT_C Image *get_displayed_image(PluginCoreState *state);
EXPORT_C void display_in_current_window(PluginCoreState *state, Image *image);


/* Utility functions. */

EXPORT_C void rgb_to_hsv(u8_quad *hsv, u8_quad rgb);
EXPORT_C void hsv_to_rgb(u8_quad *rgb, u8_quad hsv);
EXPORT_C void debug_print(const char *string);
EXPORT_C void show_message_box(const char *string);
EXPORT_C void random_seed(uint32_t *);


/* Miscellaneous functions. */

EXPORT_C int save_image(Image *image, const char *path);
EXPORT_C double borderless_clock();
EXPORT_C char *double_to_string(double);
EXPORT_C void release_double_to_string(char *);

#endif
