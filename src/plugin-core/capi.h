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

#ifdef WIN32
#define EXPORT_C EXTERN_C __declspec(dllexport)
#else
#define EXPORT_C EXTERN_C
#endif

#if defined BUILDING_BORDERLESS || defined __cplusplus
class PluginCoreState;
class Image;
class ImageTraversalIterator;
#else
typedef void PluginCoreState;
typedef void Image;
typedef void ImageTraversalIterator;
#endif

typedef unsigned char u8;

struct u8_quad{
	u8 data[4];
};

typedef struct u8_quad u8_quad;

struct position_info{
	u8_quad rgba;
	int x;
	int y;
};

typedef struct position_info position_info;

/* Note: Paths must be UTF-8 strings.*/

/* Image constructors. */

EXPORT_C Image *load_image_c(PluginCoreState *state, const char *path);
EXPORT_C Image *allocate_image_c(PluginCoreState *state, int w, int h);
EXPORT_C void unload_image_c(Image *image);


/* Image property accessors. */

EXPORT_C void get_image_dimensions_c(int *w, int *h, Image *image);
EXPORT_C u8 *get_image_pixel_data_c(int *stride, int *pitch, Image *image);


/* Image display functions. */
EXPORT_C Image *get_displayed_image_c(PluginCoreState *state);
EXPORT_C void display_in_current_window_c(Image *image);


/* Image traversal iterator. */

EXPORT_C ImageTraversalIterator *new_traversal_iterator_c(Image *image);
EXPORT_C void free_traversal_iterator_c(ImageTraversalIterator *p);
EXPORT_C int traversal_iterator_next_c(ImageTraversalIterator *p);
EXPORT_C void traversal_iterator_get_c(position_info *info, ImageTraversalIterator *p);
EXPORT_C void traversal_iterator_set_quad_c(ImageTraversalIterator *p, u8_quad rgba);
EXPORT_C void traversal_iterator_set_c(ImageTraversalIterator *p, u8 r, u8 g, u8 b, u8 a);
EXPORT_C void traversal_iterator_reset_c(ImageTraversalIterator *p);


/* Utility functions. */

EXPORT_C void rgb_to_hsv_c(u8_quad *hsv, u8_quad rgb);
EXPORT_C void hsv_to_rgb_c(u8_quad *rgb, u8_quad hsv);
EXPORT_C void debug_print_c(const char *string);
EXPORT_C void show_message_box_c(const char *string);


/* Miscellaneous functions. */

EXPORT_C int save_image_c(Image *image, const char *path);

#endif
