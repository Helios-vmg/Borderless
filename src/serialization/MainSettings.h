public:
MainSettings();
DEFINE_INLINE_SETTER_GETTER(clamp_strength)
DEFINE_INLINE_SETTER_GETTER(clamp_to_edges)
DEFINE_INLINE_SETTER_GETTER(use_checkerboard_pattern)
DEFINE_INLINE_SETTER_GETTER(center_when_displayed)
DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, zoom_mode_for_new_windows)
DEFINE_ENUM_INLINE_SETTER_GETTER(ZoomMode, fullscreen_zoom_mode_for_new_windows)
DEFINE_INLINE_SETTER_GETTER(keep_application_in_background)
bool operator==(const MainSettings &other) const;
bool operator!=(const MainSettings &other) const{
	return !(*this == other);
}
