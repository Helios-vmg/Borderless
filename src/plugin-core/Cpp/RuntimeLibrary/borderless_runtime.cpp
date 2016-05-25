B::Image Application::get_displayed_image(){
	auto handle = ::get_displayed_image(this->state);
	auto it = this->handles.find((uintptr_t)handle);
	if (it != this->handles.end())
		return Image(it->second);
	shared_ptr ret(handle);
	this->handles[(uintptr_t)handle] = ret;
	return Image(ret);
}

void Application::display_in_current_window(const Image &img){
	if (img)
		::display_in_current_window(this->state, img.get_handle());
}

void Application::debug_print(const std::string &s){
	::debug_print(s.c_str());
}

void Application::show_message_box(const std::string &s){
	::show_message_box(s.c_str());
}

Image::Image(const char *path){
	auto p = load_image(g_application->get_state(), path);
	if (p)
		this->handle.reset(p);
}

Image::Image(int w, int h){
	auto p = allocate_image(g_application->get_state(), w, h);
	if (p)
		this->handle.reset(p);
}

Image::~Image(){
	if (this->handle.unique())
		::unload_image(this->get_handle());
}

Image Image::clone() const{
	return Image(clone_image(this->get_handle()));
}

Image Image::clone_without_data() const{
	return Image(clone_image_without_data(this->get_handle()));
}

void Image::get_dimensions(int &w, int &h){
	if (!this->dims_initialized){
		get_image_dimensions(this->get_handle(), &this->w, &this->h);
		this->dims_initialized = true;
	}
	w = this->w;
	h = this->h;
}

void Image::get_pixel_data(int &w, int &h, int &stride, int &pitch, unsigned char *&pixels){
	this->get_dimensions(w, h);
	if (!this->props_initialized){
		this->pixels = get_image_pixel_data(this->get_handle(), &this->stride, &this->pitch);
		this->props_initialized = true;
	}
	stride = this->stride;
	pitch = this->pitch;
	pixels = this->pixels;
}

bool Image::save(const char *path){
	return !!save_image(this->get_handle(), path);
}
