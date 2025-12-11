#include <onder/graphics.hpp>
#include <lodepng.h>
#include <cstring>

using namespace onder::math;

namespace onder {
namespace graphics {

Image Image::filled(Vec2<uint16_t> dim, Pixel value) {
	Image img(dim);
	img.fill(value);
	return img;
}

Image Image::from_png(collections::Slice<const uint8_t> data) {
	uint8_t *out;
	unsigned int width, height;
	unsigned int res = lodepng_decode32(&out, &width, &height, data.ptr(), data.len());
	std::cerr << "from_png result: " << res << std::endl;
	if (res != 0)
		throw std::exception();
	auto w = (uint16_t)width, h = (uint16_t)height;
	if (width != w || height != h)
		throw std::exception();
	// FIXME free() isn't compatible with delete[]
	return { (Pixel *)out, { w, h } };
}

void Image::copy_from(const Image &src, Rect<uint16_t> from, Vec2<uint16_t> to) {
	if (src.dim().x < from.high().x || src.dim().y < from.high().y)
		throw std::exception();
	if (dim().x < (uint32_t)to.x + from.dim().x || dim().y < (uint32_t)to.y + from.dim().y)
		throw std::exception();

	for (size_t y = 0; y < from.dim().y; y++) {
		Pixel *rd = row(to.y + y);
		const Pixel *rs = src.row(from.low().y + y);
		::memcpy((void *)(rd + to.x), (void *)(rs + from.low().x), from.dim().x * 4ULL);
	}
}

Vec2<uint16_t> Image::dim() const {
	return m_dim;
}

uint32_t Image::area() const {
	return ((Vec2<uint32_t>)dim()).element_product();
}

void Window::clear(Pixel value) {
	buffer.fill(value);
}

void Window::draw(const Image &img, Rect<uint16_t> from, Vec2<uint16_t> to) {
	buffer.copy_from(img, from, to);
}

Vec2<uint16_t> Window::dim() const {
	return buffer.dim();
}

}
}
