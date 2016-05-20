/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "capi.h"
#include "ImageStore.h"

EXPORT_C Image *load_image(PluginCoreState *state, const char *path){
	return nullptr;
}

EXPORT_C Image *allocate_image(PluginCoreState *state, int w, int h){
	return nullptr;
}

EXPORT_C void unload_image(Image *image){

}

EXPORT_C void get_image_dimensions(int *w, int *h, Image *image){

}

EXPORT_C u8 *get_image_pixel_data(int *stride, int *pitch, Image *image){
	return nullptr;
}


EXPORT_C Image *get_displayed_image(PluginCoreState *state){
	return nullptr;
}

EXPORT_C void display_in_current_window(Image *image){

}

EXPORT_C ImageTraversalIterator *new_traversal_iterator(Image *image){
	auto it = image->get_iterator();
	if (it.is_null())
		return nullptr;
	return new decltype(it)(it);
}

EXPORT_C void free_traversal_iterator(ImageTraversalIterator *p){
	delete p;
}

EXPORT_C int traversal_iterator_next(ImageTraversalIterator *p){
	return p->next();
}

EXPORT_C void traversal_iterator_get(position_info *info, ImageTraversalIterator *p){
	*info = p->get();
}

EXPORT_C void traversal_iterator_set_quad(ImageTraversalIterator *p, u8_quad rgba){
	p->set(rgba.data);
}

EXPORT_C void traversal_iterator_set(ImageTraversalIterator *p, u8 r, u8 g, u8 b, u8 a){
	p->set(r, g, b, a);
}

EXPORT_C void traversal_iterator_reset(ImageTraversalIterator *p){
	p->reset();
}

EXPORT_C void rgb_to_hsv(u8_quad *hsv, u8_quad rgb){

}

EXPORT_C void hsv_to_rgb(u8_quad *rgb, u8_quad hsv){

}

EXPORT_C void debug_print(const char *string){

}

EXPORT_C void show_message_box(const char *string){

}

EXPORT_C int save_image(Image *image, const char *path){
	return false;
}
