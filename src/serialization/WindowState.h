public:
WindowState();
DEFINE_INLINE_SETTER_GETTER(pos)
DEFINE_INLINE_SETTER_GETTER(size)
DEFINE_INLINE_SETTER_GETTER(label_pos)
DEFINE_INLINE_SETTER_GETTER(using_checkerboard_pattern)
DEFINE_INLINE_SETTER_GETTER(current_directory)
DEFINE_INLINE_SETTER_GETTER(current_filename)
DEFINE_INLINE_SETTER_GETTER(zoom)
DEFINE_INLINE_SETTER_GETTER(fullscreen)
DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, zoom_mode)
DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, fullscreen_zoom_mode)
DEFINE_INLINE_SETTER_GETTER(movement_size)
DEFINE_INLINE_SETTER_GETTER(transform)
DEFINE_INLINE_SETTER_GETTER(border_size)
static const decltype(border_size) default_border_size = 50;
void reset_border_size(){
	this->border_size = default_border_size;
}
