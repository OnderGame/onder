#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <PixelToaster.h>

namespace onder {
namespace graphics {

using Pixel = PixelToaster::TrueColorPixel;

class Window {
	PixelToaster::Display display;

public:
	Window(const char *title, uint16_t width, uint16_t height)
		: display(title, width, height, PixelToaster::Output::Default, PixelToaster::Mode::TrueColor)
	{}

	bool is_open() const {
		return display.open();
	}

	void update(Pixel *pixels) {
		display.update(pixels);
	}
};

class ImageRgb8 {
	uint16_t width, height;
	uint8_t *data;

public:
	ImageRgb8() : data(nullptr), width(0), height(0) {}
	ImageRgb8(uint16_t width, uint16_t height) : data(nullptr), width(width), height(height) {
		size_t len = (size_t)width * height;
		if (height != 0 && len / height != width)
			throw new std::exception(); // TODO specific error
		data = new uint8_t[len];
		memset(data, 0, len);
	}
	ImageRgb8(uint8_t *data, uint16_t width, uint16_t height) : data(data), width(width), height(height) {}
	~ImageRgb8() {
		delete data;
	}
};

class Render {
public:
};

}
}
