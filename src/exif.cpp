#include "exif.h"
#include <sstream>
#include <QFile>

#include "ImageViewerApplication.h"

class QIODeviceExifStream : public TinyEXIF::EXIFStream{
	std::unique_ptr<QIODevice> dev;
	std::vector<std::uint8_t> internal_buffer;
public:
	QIODeviceExifStream(std::unique_ptr<QIODevice> &&dev): dev(std::move(dev)){
		this->dev->reset();
	}
	bool IsValid() const override{
		return true;
	}
	const uint8_t *GetBuffer(unsigned desired_length) override{
		this->internal_buffer.resize(desired_length);
		auto p = this->internal_buffer.data();
		auto read = this->dev->read((char *)p, desired_length);
		if (read < desired_length)
			memset(p + read, 0, desired_length - read);
		return p;
	}
	bool SkipBuffer(unsigned desired_length) override{
		auto position = this->dev->pos();
		auto length = this->dev->size();
		return this->dev->seek(position + desired_length);
	}
};

typedef std::optional<std::string> optstring;
typedef optstring (*deparser_function)(const TinyEXIF::EXIFInfo &info);

struct Deparser{
	deparser_function f;
	const char *key_name;
};

namespace{

template <typename T>
std::string custom_to_string(const T &x){
	std::stringstream stream;
	stream << x;
	return stream.str();
}

template <typename T>
optstring custom_to_string_zero_is_null(const T &x){
	if (!x)
		return {};
	return custom_to_string(x);
}

optstring get_width(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.ImageWidth);
}

optstring get_height(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.ImageHeight);
}

optstring get_related_width(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.RelatedImageWidth);
}

optstring get_related_height(const TinyEXIF::EXIFInfo &info){
	if (!info.RelatedImageHeight)
		return {};
	return custom_to_string_zero_is_null(info.RelatedImageHeight);
}

optstring get_description(const TinyEXIF::EXIFInfo &info){
	if (info.ImageDescription.empty())
		return {};
	return info.ImageDescription;
}

optstring get_make(const TinyEXIF::EXIFInfo &info){
	if (info.Make.empty())
		return {};
	return info.Make;
}

optstring get_model(const TinyEXIF::EXIFInfo &info){
	if (info.Model.empty())
		return {};
	return info.Model;
}

optstring get_serial_number(const TinyEXIF::EXIFInfo &info){
	if (info.SerialNumber.empty())
		return {};
	return info.SerialNumber;
}

optstring get_orientation(const TinyEXIF::EXIFInfo &info){
	switch (info.Orientation){
		case 1:
			return "normal";
		case 2:
			return "normal (mirrored)";
		case 3:
			return "camera inverted";
		case 4:
			return "camera inverted (mirrored)";
		case 5:
			return "camera rotated 90\xc2\xb0 to the left (mirrored)";
		case 6:
			return "camera rotated 90\xc2\xb0 to the right";
		case 7:
			return "camera rotated 90\xc2\xb0 to the right (mirrored)";
		case 8:
			return "camera rotated 90\xc2\xb0 to the left";
		default:
			return {};
	}
}

optstring resolution_value_to_string(double val, const TinyEXIF::EXIFInfo &info){
	if (!val)
		return {};

	std::stringstream stream;
	stream << val;
	switch (info.ResolutionUnit){
		case 2:
			stream << " DPI";
			break;
		case 3:
			stream << " px/cm";
			break;
		default:
			stream << " ???";
	}
	return stream.str();
}

optstring get_x_resolution(const TinyEXIF::EXIFInfo &info){
	return resolution_value_to_string(info.XResolution, info);
}

optstring get_y_resolution(const TinyEXIF::EXIFInfo &info){
	return resolution_value_to_string(info.YResolution, info);
}

optstring get_bits_per_sample(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.BitsPerSample);
}

optstring get_software(const TinyEXIF::EXIFInfo &info){
	if (info.Software.empty())
		return {};
	return info.Software;
}

optstring get_datetime(const TinyEXIF::EXIFInfo &info){
	if (info.DateTime.empty())
		return {};
	return info.DateTime;
}

optstring get_original_datetime(const TinyEXIF::EXIFInfo &info){
	if (info.DateTimeOriginal.empty())
		return {};
	auto ret = info.DateTimeOriginal;
	if (!info.SubSecTimeOriginal.empty()){
		ret += '.';
		ret += info.SubSecTimeOriginal;
	}
	return ret;
}

optstring get_digitized_datetime(const TinyEXIF::EXIFInfo &info){
	if (info.DateTimeDigitized.empty())
		return {};
	return info.DateTimeDigitized;
}

optstring get_copyright(const TinyEXIF::EXIFInfo &info){
	if (info.Copyright.empty())
		return {};
	return info.Copyright;
}

optstring get_exposure(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.ExposureTime);
}

optstring get_fstop(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.FNumber);
}

optstring get_exposure_program(const TinyEXIF::EXIFInfo &info){
	switch (info.ExposureProgram){
		case 1:
			return "manual";
		case 2:
			return "normal";
		case 3:
			return "aperture priority";
		case 4:
			return "shutter priority";
		case 5:
			return "creative program";
		case 6:
			return "action program";
		case 7:
			return "portrait mode";
		case 8:
			return "landscape mode";
		default:
			return {};
	}
}

optstring get_iso_speed(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.ISOSpeedRatings);
}

optstring get_shutter_speed(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.ShutterSpeedValue);
}

optstring get_aperture(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.ApertureValue);
}

optstring get_brightness(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.BrightnessValue);
}

optstring get_exposure_bias(const TinyEXIF::EXIFInfo &info){
	return custom_to_string_zero_is_null(info.ExposureBiasValue);
}

optstring get_focal_plane(const TinyEXIF::EXIFInfo &info){
	if (!info.SubjectDistance)
		return {};
	return custom_to_string(info.SubjectDistance) + " m";
}

optstring get_focal_length(const TinyEXIF::EXIFInfo &info){
	if (!info.FocalLength)
		return {};
	return custom_to_string(info.FocalLength) + " mm";
}

optstring get_flash_setting(const TinyEXIF::EXIFInfo &info){
	std::string ret;
	ret += (info.Flash & 1) ? "flash used" : "no flash";
	ret += ", ";

	switch ((info.Flash >> 1) & 0b11){
		case 0:
			ret += "strobe not used";
			break;
		case 1:
			ret += "reserved";
			break;
		case 2:
			ret += "strobe return light not detected";
			break;
		case 3:
			ret += "strobe return light detected";
			break;
	}
	ret += ", ";

	switch ((info.Flash >> 3) & 0b11){
		case 0:
			ret += "unknown flash mode";
			break;
		case 1:
			ret += "flash forced on";
			break;
		case 2:
			ret += "flash forced off";
			break;
		case 3:
			ret += "auto flash";
			break;
	}
	ret += ", ";

	ret += ((info.Flash >> 5) & 1) ? "flash available" : "flash unavailable";
	ret += ", ";

	ret += ((info.Flash >> 6) & 1) ? "no red-eye mode" : "red-eye mode supported";

	return ret;
}

optstring get_metering_mode(const TinyEXIF::EXIFInfo &info){
	switch (info.MeteringMode){
		case 1:
			return "average";
		case 2:
			return "center weighted average";
		case 3:
			return "spot";
		case 4:
			return "multi-spot";
		case 5:
			return "pattern";
		case 6:
			return "partial";
		default:
			return {};
	}
}

optstring get_light_source(const TinyEXIF::EXIFInfo &info){
	switch (info.LightSource){
		case 1:
			return "daylight";
		case 2:
			return "fluorescent";
		case 3:
			return "tungsten (incandescent light)";
		case 4:
			return "flash";
		case 9:
			return "fine weather";
		case 10:
			return "cloudy weather";
		case 11:
			return "shade";
		case 12:
			return "daylight fluorescent (D 5700 - 7100K)";
		case 13:
			return "day white fluorescent (N 4600 - 5400K)";
		case 14:
			return "cool white fluorescent (W 3900 - 4500K)";
		case 15:
			return "white fluorescent (WW 3200 - 3700K)";
		case 17:
			return "standard light A";
		case 18:
			return "standard light B";
		case 19:
			return "standard light C";
		case 20:
			return "D55";
		case 21:
			return "D65";
		case 22:
			return "D75";
		case 23:
			return "D50";
		case 24:
			return "ISO studio tungsten";
		default:
			return {};
	}
}

optstring get_projection(const TinyEXIF::EXIFInfo &info){
	switch (info.ProjectionType){
		case 1:
			return "perspective projection";
		case 2:
			return "equirectangular/spherical projection";
		default:
			return {};
	}
}

optstring get_calibration(const TinyEXIF::EXIFInfo &info){
	if (!info.Calibration.FocalLength && !info.Calibration.OpticalCenterX && !info.Calibration.OpticalCenterY)
		return {};

	std::stringstream stream;
	stream <<
		"focal length: " << info.Calibration.FocalLength << " px; "
		"optical center: " << info.Calibration.OpticalCenterX << "x" << info.Calibration.OpticalCenterY << " px";
	return stream.str();
}

optstring get_geo_coords(const TinyEXIF::EXIFInfo &info){
	if (!info.GeoLocation.hasLatLon())
		return {};
	std::stringstream stream;
	stream << "lat: " << info.GeoLocation.Latitude << ", lon: " << info.GeoLocation.Longitude;
	return stream.str();
}

optstring get_geo_altitude(const TinyEXIF::EXIFInfo &info){
	if (!info.GeoLocation.hasAltitude())
		return {};
	return custom_to_string(info.GeoLocation.Altitude) + " m";
}

optstring get_geo_relative_altitude(const TinyEXIF::EXIFInfo &info){
	if (!info.GeoLocation.hasRelativeAltitude())
		return {};
	return custom_to_string(info.GeoLocation.RelativeAltitude) + " m";
}

optstring get_geo_3dof(const TinyEXIF::EXIFInfo &info){
	if (!info.GeoLocation.hasOrientation())
		return {};
	std::stringstream stream;
	stream << "pitch: " << info.GeoLocation.PitchDegree << "\xc2\xb0, roll: " << info.GeoLocation.RollDegree << "\xc2\xb0, yaw: " << info.GeoLocation.YawDegree << "\xc2\xb0";
	return stream.str();
}

optstring get_geo_speed(const TinyEXIF::EXIFInfo &info){
	if (!info.GeoLocation.hasSpeed())
		return {};
	std::stringstream stream;
	stream << info.GeoLocation.SpeedX << "," << info.GeoLocation.SpeedY << "," << info.GeoLocation.SpeedZ << " m/s";
	return stream.str();
}

optstring get_gps_date(const TinyEXIF::EXIFInfo &info){
	if (info.GeoLocation.GPSDateStamp.empty())
		return {};
	return info.GeoLocation.GPSDateStamp;
}

optstring get_gps_time(const TinyEXIF::EXIFInfo &info){
	if (info.GeoLocation.GPSTimeStamp.empty())
		return {};
	return info.GeoLocation.GPSTimeStamp;
}

}

#define DEFINE_GETTER(x) { get_##x, #x }

static const Deparser deparsers[] = {
	DEFINE_GETTER(width),
	DEFINE_GETTER(height),
	DEFINE_GETTER(related_width),
	DEFINE_GETTER(related_height),
	DEFINE_GETTER(description),
	DEFINE_GETTER(make),
	DEFINE_GETTER(model),
	DEFINE_GETTER(serial_number),
	DEFINE_GETTER(orientation),
	DEFINE_GETTER(x_resolution),
	DEFINE_GETTER(y_resolution),
	DEFINE_GETTER(bits_per_sample),
	DEFINE_GETTER(software),
	DEFINE_GETTER(datetime),
	DEFINE_GETTER(original_datetime),
	DEFINE_GETTER(digitized_datetime),
	DEFINE_GETTER(copyright),
	DEFINE_GETTER(exposure),
	DEFINE_GETTER(fstop),
	DEFINE_GETTER(exposure_program),
	DEFINE_GETTER(iso_speed),
	DEFINE_GETTER(shutter_speed),
	DEFINE_GETTER(aperture),
	DEFINE_GETTER(brightness),
	DEFINE_GETTER(exposure_bias),
	DEFINE_GETTER(focal_plane),
	DEFINE_GETTER(focal_length),
	DEFINE_GETTER(flash_setting),
	DEFINE_GETTER(metering_mode),
	DEFINE_GETTER(light_source),
	DEFINE_GETTER(projection),
	DEFINE_GETTER(calibration),
	DEFINE_GETTER(geo_coords),
	DEFINE_GETTER(geo_altitude),
	DEFINE_GETTER(geo_relative_altitude),
	DEFINE_GETTER(geo_3dof),
	DEFINE_GETTER(geo_speed),
	DEFINE_GETTER(gps_date),
	DEFINE_GETTER(gps_time),
};

std::vector<std::pair<std::string, std::string>> set_exif(TinyEXIF::EXIFInfo &dst, std::unique_ptr<QIODevice> &&dev){
	std::vector<std::pair<std::string, std::string>> ret;
	QIODeviceExifStream stream(std::move(dev));
	auto result = dst.parseFrom(stream);
	if (result != TinyEXIF::PARSE_SUCCESS)
		return ret;

	for (auto [function, key] : deparsers){
		auto s = function(dst);
		if (!s)
			continue;
		ret.emplace_back(key, std::move(*s));
	}

	return ret;
}

ImageMetadata::ImageMetadata(QImage &image, const QString &path){
	auto dev = std::make_unique<QFile>(path);
	dev->open(QFile::ReadOnly);
	this->name = QString::fromStdString(dev->filesystemFileName().filename().u8string());
	this->path = path;
	this->size = dev->size();
	this->dimensions = std::make_pair(image.size(), 1);
	this->human_metadata = set_exif(this->machine_metadata, std::move(dev));
}

ImageMetadata::ImageMetadata(std::unique_ptr<QIODevice> &&dev){
	this->human_metadata = set_exif(this->machine_metadata, std::move(dev));
}

std::pair<int, bool> ImageMetadata::get_orientation() const{
	auto o = this->machine_metadata.Orientation;
	if (o < 2 || o > 8)
		return { 0, false };

	static const signed char values[] = {
		-1, // 2
		 3, // 3
		-3, // 4
		-2, // 5
		 2, // 6
		-4, // 7
		 4, // 8
	};
	auto value = values[o - 2];
	bool flipped = value < 0;
	if (flipped)
		value = (signed char)-value;
	value--;
	return { value, flipped };
}
