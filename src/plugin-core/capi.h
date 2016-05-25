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

#if defined WIN32 && defined BUILDING_BORDERLESS
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


/* Image traversal iterator. */

EXPORT_C ImageTraversalIterator *new_traversal_iterator(Image *image);
EXPORT_C void free_traversal_iterator(ImageTraversalIterator *p);
EXPORT_C int traversal_iterator_next(ImageTraversalIterator *p);
EXPORT_C void traversal_iterator_get(position_info *info, ImageTraversalIterator *p);
EXPORT_C void traversal_iterator_set_quad(ImageTraversalIterator *p, u8_quad rgba);
EXPORT_C void traversal_iterator_set(ImageTraversalIterator *p, u8 r, u8 g, u8 b, u8 a);
EXPORT_C void traversal_iterator_reset(ImageTraversalIterator *p);


/* Utility functions. */

EXPORT_C void rgb_to_hsv(u8_quad *hsv, u8_quad rgb);
EXPORT_C void hsv_to_rgb(u8_quad *rgb, u8_quad hsv);
EXPORT_C void debug_print(const char *string);
EXPORT_C void show_message_box(const char *string);


/* Miscellaneous functions. */

EXPORT_C int save_image(Image *image, const char *path);

#endif
