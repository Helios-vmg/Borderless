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

ImageIterator::ImageIterator(Image &image): image(image){
	this->image.get_pixel_data(this->w, this->h, this->stride, this->pitch, this->pixels);
	this->reset();
}

bool ImageIterator::next(u8 *&pi){
	if (this->i >= this->n)
		return false;
	pi = this->pixels + this->i++ * 4;
	return true;
}

void ImageIterator::position(int &x, int &y) const{
	auto i = this->i - 1;
	x = i % this->w;
	y = i / this->w;
}

void ImageIterator::reset(){
	this->i = 0;
	this->n = this->w * this->h;
}

XorShift128::XorShift128(){
	random_seed(this->state.data());
}

std::uint32_t XorShift128::operator()(){
	auto x = this->state[3];
	x ^= x << 11;
	x ^= x >> 8;
	this->state[3] = this->state[2];
	this->state[2] = this->state[1];
	this->state[1] = this->state[0];
	x ^= this->state[0];
	x ^= this->state[0] >> 19;
	this->state[0] = x;
	return x;
}

void XorShift128::generate_block(void *buffer, size_t size){
	auto dst = (std::uint8_t *)buffer;
	for (size_t i = 0; i < size;){
		auto u = (*this)();
		for (int j = 4; j--; i++){
			dst[i] = u & 0xFF;
			u >>= 8;
		}
	}
}
