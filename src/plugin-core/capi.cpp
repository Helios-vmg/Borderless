/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "capi.h"
#include "ImageStore.h"

EXPORT_C Image *load_image_c(PluginCoreState *state, const char *path){
	return nullptr;
}

EXPORT_C Image *allocate_image_c(PluginCoreState *state, int w, int h){
	return nullptr;
}

EXPORT_C void unload_image_c(Image *image){

}

EXPORT_C void get_image_dimensions_c(int *w, int *h, Image *image){

}

EXPORT_C u8 *get_image_pixel_data_c(int *stride, int *pitch, Image *image){
	return nullptr;
}


EXPORT_C Image *get_displayed_image_c(PluginCoreState *state){
	return nullptr;
}

EXPORT_C void display_in_current_window_c(Image *image){

}

EXPORT_C ImageTraversalIterator *new_traversal_iterator_c(Image *image){
	auto it = image->get_iterator();
	if (it.is_null())
		return nullptr;
	return new decltype(it)(it);
}

EXPORT_C void free_traversal_iterator_c(ImageTraversalIterator *p){
	delete p;
}

EXPORT_C int traversal_iterator_next_c(ImageTraversalIterator *p){
	return p->next();
}

EXPORT_C void traversal_iterator_get_c(position_info *info, ImageTraversalIterator *p){
	*info = p->get();
}

EXPORT_C void traversal_iterator_set_quad_c(ImageTraversalIterator *p, u8_quad rgba){
	p->set(rgba.data);
}

EXPORT_C void traversal_iterator_set_c(ImageTraversalIterator *p, u8 r, u8 g, u8 b, u8 a){
	p->set(r, g, b, a);
}

EXPORT_C void traversal_iterator_reset_c(ImageTraversalIterator *p){
	p->reset();
}

EXPORT_C void rgb_to_hsv_c(u8_quad *hsv, u8_quad rgb){

}

EXPORT_C void hsv_to_rgb_c(u8_quad *rgb, u8_quad hsv){

}

EXPORT_C void debug_print_c(const char *string){

}

EXPORT_C void show_message_box_c(const char *string){

}

EXPORT_C int save_image_c(Image *image, const char *path){
	return false;
}
