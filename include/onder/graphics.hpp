#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <PixelToaster.h>
#include <onder/collections/slice.hpp>

namespace onder {
namespace graphics {

using Pixel = PixelToaster::TrueColorPixel;

class Image {
	uint16_t m_width, m_height;
	Pixel *m_data;

	const Pixel *row(uint16_t y) const {
		return m_data + (m_width * y);
	}

	Pixel *row(uint16_t y) {
		return m_data + (m_width * y);
	}

	Image(const Image &) = delete;
	Image &operator=(const Image &) = delete;

public:
	Image() : m_data(nullptr), m_width(0), m_height(0) {}
	Image(uint16_t width, uint16_t height) : m_data(nullptr), m_width(width), m_height(height) {
		size_t len = (size_t)width * height;
		if (height != 0 && len / height != width)
			throw new std::exception(); // TODO specific error
		m_data = new Pixel[len];
	}
	Image(Pixel *data, uint16_t width, uint16_t height) : m_data(data), m_width(width), m_height(height) {}
	~Image() {
		delete m_data;
	}

	Image(Image &&src) : m_data(src.m_data), m_width(src.m_width), m_height(src.m_height) {
		src.m_width = 0;
		src.m_height = 0;
		src.m_data = nullptr;
	}
	Image &operator=(Image &&src) {
		m_width = src.m_width;
		m_height = src.m_height;
		m_data = src.m_data;
		src.m_width = 0;
		src.m_height = 0;
		src.m_data = nullptr;
		return *this;
	}

	static Image filled(uint16_t width, uint16_t height, Pixel value);
	static Image from_png(collections::Slice<const uint8_t> data);

	uint16_t width() const {
		return m_width;
	}

	uint16_t height() const {
		return m_height;
	}

	const Pixel *data() const {
		return m_data;
	}

	void copy_from(uint16_t x, uint16_t y, const Image &src);

	void fill(Pixel value) {
		for (size_t i = 0; i < (size_t)width() * height(); i++)
			m_data[i] = value;
	}
};

class Window {
	PixelToaster::Display display;
	Image buffer;

	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;

public:
	Window(const char *title, uint16_t width, uint16_t height)
		: display(title, width, height, PixelToaster::Output::Default, PixelToaster::Mode::TrueColor)
		, buffer(width, height)
	{}

	bool is_open() const {
		return display.open();
	}

	void update() {
		display.update((Pixel *)buffer.data());
	}

	void draw(uint16_t x, uint16_t y, const Image &img);
};

class Render {
public:
};

}
}
