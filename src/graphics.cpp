#include <onder/graphics.hpp>
#include <lodepng.h>
#include <iostream>
#include <cstring>

namespace onder {
namespace graphics {

Image Image::from_png(collections::Slice<const uint8_t> data) {
	uint8_t *out;
	unsigned int width, height;
	unsigned int res = lodepng_decode32(&out, &width, &height, data.ptr(), data.len());
	std::cerr << "from_png result: " << res << std::endl;
	if (res != 0)
		throw std::exception();
	// FIXME free() isn't compatible with delete[]
	return { (Pixel *)out, width, height };
}

void Image::copy_from(uint16_t x, uint16_t y, const Image &src) {
	for (uint16_t dy = 0; dy < src.height(); dy++) {
		::memcpy((void *)(row(y + dy) + x), (void *)src.row(dy), 4 * (size_t)src.width());
	}
}

void Window::draw(uint16_t x, uint16_t y, const Image &img) {
	buffer.copy_from(x, y, img);
}

}
}
