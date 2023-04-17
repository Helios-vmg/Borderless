#include "resvg.hpp"
#ifdef ENABLE_SVG
#include <resvg.h>

#if defined _MSC_VER && (defined WIN32 || defined WIN64)
#ifdef _DEBUG
#pragma comment(lib, "resvgd.lib")
#else
#pragma comment(lib, "resvg.lib")
#endif
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Userenv.lib")
#endif

ReSvgOptions::ReSvgOptions(){
	this->options = resvg_options_create();
}

ReSvgOptions::~ReSvgOptions(){
	if (this->options)
		resvg_options_destroy(this->options);
}

#define ENSURE_VALID_OPTIONS if (!this->options) throw std::exception()

void ReSvgOptions::set_resources_dir(const char *path){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_resources_dir(this->options, path);
}

void ReSvgOptions::set_dpi(double dpi){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_dpi(this->options, dpi);
}

void ReSvgOptions::set_font_family(const char *family){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_font_family(this->options, family);
}

void ReSvgOptions::set_font_size(double size){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_font_size(this->options, size);
}

void ReSvgOptions::set_serif_family(const char *family){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_serif_family(this->options, family);
}

void ReSvgOptions::set_sans_serif_family(const char *family){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_sans_serif_family(this->options, family);
}

void ReSvgOptions::set_cursive_family(const char *family){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_cursive_family(this->options, family);
}

void ReSvgOptions::set_fantasy_family(const char *family){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_fantasy_family(this->options, family);
}

void ReSvgOptions::set_monospace_family(const char *family){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_monospace_family(this->options, family);
}

void ReSvgOptions::set_languages(const char *languages){
	ENSURE_VALID_OPTIONS;
	resvg_options_set_languages(this->options, languages);
}

void ReSvgOptions::set_shape_rendering_mode(ShapeRenderingMode mode){
	ENSURE_VALID_OPTIONS;
	resvg_shape_rendering mode2;
	switch (mode){
		case ShapeRenderingMode::OptimizeSpeed:
			mode2 = RESVG_SHAPE_RENDERING_OPTIMIZE_SPEED;
			break;
		case ShapeRenderingMode::CrispEdges:
			mode2 = RESVG_SHAPE_RENDERING_CRISP_EDGES;
			break;
		case ShapeRenderingMode::GeometricPrecision:
			mode2 = RESVG_SHAPE_RENDERING_GEOMETRIC_PRECISION;
			break;
		default:
			throw std::exception();
	}
	resvg_options_set_shape_rendering_mode(this->options, mode2);
}

void ReSvgOptions::set_text_rendering_mode(TextRenderingMode mode){
	ENSURE_VALID_OPTIONS;
	resvg_text_rendering mode2;
	switch (mode){
		case TextRenderingMode::OptimizeSpeed:
			mode2 = RESVG_TEXT_RENDERING_OPTIMIZE_SPEED;
			break;
		case TextRenderingMode::OptimizeLegibility:
			mode2 = RESVG_TEXT_RENDERING_OPTIMIZE_LEGIBILITY;
			break;
		case TextRenderingMode::GeometricPrecision:
			mode2 = RESVG_TEXT_RENDERING_GEOMETRIC_PRECISION;
			break;
		default:
			throw std::exception();
	}
	resvg_options_set_text_rendering_mode(this->options, mode2);
}

void ReSvgOptions::set_image_rendering_mode(ImageRenderingMode mode){
	ENSURE_VALID_OPTIONS;
	resvg_image_rendering mode2;
	switch (mode){
		case ImageRenderingMode::OptimizeQuality:
			mode2 = RESVG_IMAGE_RENDERING_OPTIMIZE_QUALITY;
			break;
		case ImageRenderingMode::OptimizeSpeed:
			mode2 = RESVG_IMAGE_RENDERING_OPTIMIZE_SPEED;
			break;
		default:
			throw std::exception();
	}
	resvg_options_set_image_rendering_mode(this->options, mode2);
}

void ReSvgOptions::load_font_data(const char *data, uintptr_t len){
	ENSURE_VALID_OPTIONS;
	resvg_options_load_font_data(this->options, data, len);
}

std::int32_t ReSvgOptions::load_font_file(const char *file_path){
	ENSURE_VALID_OPTIONS;
	return resvg_options_load_font_file(this->options, file_path);
}

void ReSvgOptions::load_system_fonts(){
	ENSURE_VALID_OPTIONS;
	resvg_options_load_system_fonts(this->options);
}

ReSvgRenderTree::~ReSvgRenderTree(){
	if (this->tree)
		resvg_tree_destroy(this->tree);
}

std::pair<ReSvgRenderTree::Error, ReSvgRenderTree> ReSvgRenderTree::check_errors(std::int32_t result_int, ReSvgRenderTree &&ret){
	auto result = (resvg_error)result_int;
	if (result != RESVG_OK){
		ret.tree = nullptr;
		switch (result){
			case RESVG_ERROR_NOT_AN_UTF8_STR:
				return { Error::NotUtf8, std::move(ret) };
			case RESVG_ERROR_FILE_OPEN_FAILED:
				return { Error::FileOpenFailed, std::move(ret) };
			case RESVG_ERROR_MALFORMED_GZIP:
				return { Error::MalformedGzip, std::move(ret) };
			case RESVG_ERROR_ELEMENTS_LIMIT_REACHED:
				return { Error::ElementsLimitExceeded, std::move(ret) };
			case RESVG_ERROR_INVALID_SIZE:
				return { Error::InvalidDimensions, std::move(ret) };
			case RESVG_ERROR_PARSING_FAILED:
				return { Error::ParsingError, std::move(ret) };
			default:
				throw std::exception();
		}
	}
	return { Error::NoError, std::move(ret) };
}

std::pair<ReSvgRenderTree::Error, ReSvgRenderTree> ReSvgRenderTree::create_from_data(const void *data, size_t size, const ReSvgOptions &options){
	ReSvgRenderTree ret;
	auto result = (resvg_error)resvg_parse_tree_from_data((const char *)data, (uintptr_t)size, options.options, &ret.tree);
	return check_errors(result, std::move(ret));
}

std::pair<ReSvgRenderTree::Error, ReSvgRenderTree> ReSvgRenderTree::create_from_path(const char *path, const ReSvgOptions &options){
	ReSvgRenderTree ret;
	auto result = (resvg_error)resvg_parse_tree_from_file(path, options.options, &ret.tree);
	return check_errors(result, std::move(ret));
}

#define ENSURE_VALID_TREE if (!this->tree) throw std::exception()

bool ReSvgRenderTree::is_empty() const{
	ENSURE_VALID_TREE;
	return resvg_is_image_empty(this->tree);
}

std::pair<double, double> ReSvgRenderTree::get_size() const{
	ENSURE_VALID_TREE;
	auto [width, height] = resvg_get_image_size(this->tree);
	return { width, height };
}

std::pair<int, int> ReSvgRenderTree::get_size_int() const{
	ENSURE_VALID_TREE;
	auto [w, h] = resvg_get_image_size(this->tree);
	return {
		(int)ceil(w),
		(int)ceil(h),
	};
}

std::tuple<double, double, double, double> ReSvgRenderTree::get_viewbox() const{
	ENSURE_VALID_TREE;
	auto [x, y, w, h] = resvg_get_image_viewbox(this->tree);
	return { x, y, w, h };
}

std::optional<std::tuple<double, double, double, double>> ReSvgRenderTree::get_bounding_box() const{
	ENSURE_VALID_TREE;
	resvg_rect ret;
	if (!resvg_get_image_bbox(this->tree, &ret))
		return {};
	auto [x, y, w, h] = ret;
	return {{ x, y, w, h }};
}

std::tuple<int, int, std::vector<std::uint8_t>> ReSvgRenderTree::render() const{
	ENSURE_VALID_TREE;
	auto [w, h] = this->get_size_int();
	std::vector<std::uint8_t> ret(w * h * 4);
	this->render(ret.data());
	return { w, h, ret };
}

void ReSvgRenderTree::render(void *dst) const{
	auto [w, h] = this->get_size_int();
	resvg_render(this->tree, { RESVG_FIT_TO_TYPE_ORIGINAL, 1 }, resvg_transform_identity(), w, h, (char *)dst);
}

#endif
