/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "capi.h"
#include "ImageStore.h"
#include "PluginCoreState.h"
#include <QtWidgets/QMessageBox>
#ifdef WIN32
#include <Windows.h>
#endif

EXPORT_C Image *load_image(PluginCoreState *state, const char *path){
	return state->get_store().load_image(path);
}

EXPORT_C Image *allocate_image(PluginCoreState *state, int w, int h){
	return state->get_store().allocate_image(w, h);
}

EXPORT_C Image *clone_image(Image *image){
	return nullptr;
}

EXPORT_C Image *clone_image_without_data(Image *image){
	return nullptr;
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
#ifdef WIN32
	auto w = QString::fromUtf8(string).toStdWString();
	OutputDebugStringW(w.c_str());
#endif
}

EXPORT_C void show_message_box(const char *string){
	QMessageBox msgbox;
	msgbox.setText(QString::fromUtf8(string));
	msgbox.exec();
}

EXPORT_C int save_image(Image *image, const char *path){
	image->save(QString::fromUtf8(path), SaveOptions());
	return false;
}
