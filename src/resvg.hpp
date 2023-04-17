#pragma once

#include "config.hpp"

#ifdef ENABLE_SVG

#include <tuple>
#include <optional>
#include <vector>
#include <cstdint>

struct resvg_options;
struct resvg_render_tree;

class ReSvgOptions{
	friend class ReSvgRenderTree;
	resvg_options *options = nullptr;
public:
	enum class ShapeRenderingMode{
		OptimizeSpeed,
		CrispEdges,
		GeometricPrecision,
	};
	enum class TextRenderingMode{
		OptimizeSpeed,
		OptimizeLegibility,
		GeometricPrecision,
	};
	enum class ImageRenderingMode{
		OptimizeQuality,
		OptimizeSpeed,
	};
	
	ReSvgOptions();
	ReSvgOptions(const ReSvgOptions &) = delete;
	ReSvgOptions &operator=(const ReSvgOptions &) = delete;
	ReSvgOptions(ReSvgOptions &&other) noexcept{
		*this = std::move(other);
	}
	ReSvgOptions &operator=(ReSvgOptions &&other) noexcept{
		this->options = other.options;
		other.options = nullptr;
		return *this;
	}
	~ReSvgOptions();
	void set_resources_dir(const char *path);
	void set_dpi(double dpi);
	void set_font_family(const char *family);
	void set_font_size(double size);
	void set_serif_family(const char *family);
	void set_sans_serif_family(const char *family);
	void set_cursive_family(const char *family);
	void set_fantasy_family(const char *family);
	void set_monospace_family(const char *family);
	void set_languages(const char *languages);
	void set_shape_rendering_mode(ShapeRenderingMode);
	void set_text_rendering_mode(TextRenderingMode);
	void set_image_rendering_mode(ImageRenderingMode);
	void load_font_data(const char *data, uintptr_t len);
	std::int32_t load_font_file(const char *file_path);
	void load_system_fonts();
};

class ReSvgRenderTree{
public:
	enum class Error{
		NoError,
		NotUtf8,
		FileOpenFailed,
		MalformedGzip,
		ElementsLimitExceeded,
		InvalidDimensions,
		ParsingError,
	};
private:
	resvg_render_tree *tree = nullptr;
	
	static std::pair<Error, ReSvgRenderTree> check_errors(std::int32_t, ReSvgRenderTree &&);
public:
	ReSvgRenderTree() = default;
	static std::pair<Error, ReSvgRenderTree> create_from_path(const char *path, const ReSvgOptions &options);
	static std::pair<Error, ReSvgRenderTree> create_from_data(const void *data, size_t size, const ReSvgOptions &options);
	ReSvgRenderTree(const ReSvgRenderTree &) = delete;
	ReSvgRenderTree &operator=(const ReSvgRenderTree &) = delete;
	ReSvgRenderTree(ReSvgRenderTree &&other) noexcept{
		*this = std::move(other);
	}
	ReSvgRenderTree &operator=(ReSvgRenderTree &&other) noexcept{
		this->tree = other.tree;
		other.tree = nullptr;
		return *this;
	}
	~ReSvgRenderTree();
	operator bool() const{
		return !!this->tree;
	}
	bool is_empty() const;
	std::pair<double, double> get_size() const;
	std::pair<int, int> get_size_int() const;
	std::tuple<double, double, double, double> get_viewbox() const;
	std::optional<std::tuple<double, double, double, double>> get_bounding_box() const;
	std::tuple<int, int, std::vector<std::uint8_t>> render() const;
	//dst MUST point to a portion of writeable memory >= w * h * 4, where [w, h] = get_size_int().
	void render(void *dst) const;
};

#endif
