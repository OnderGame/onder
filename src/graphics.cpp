#include <onder/graphics.hpp>
#include <lodepng.h>
#include <cstring>

using namespace onder::math;

namespace onder {
namespace graphics {

Image::Image(math::Vec2<uint16_t> dim) : m_data(nullptr), m_dim(dim) {
	m_data = new Pixel[area()];
}

Image::~Image() {
	delete[] m_data;
}

Image::Image(Image &&src) : m_data(src.m_data), m_dim(src.m_dim) {
	src.m_dim = {};
	src.m_data = nullptr;
}
Image &Image::operator=(Image &&src) {
	this->~Image();
	m_dim = src.m_dim;
	m_data = src.m_data;
	src.m_dim = {};
	src.m_data = nullptr;
	return *this;
}

const Pixel *Image::row(uint16_t y) const {
	return m_data + (m_dim.x * y);
}

Pixel *Image::row(uint16_t y) {
	return m_data + (m_dim.x * y);
}

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
	// LodePNG uses malloc/free/realloc internally, which isn't compatible with new/delete
	// We can easily substitute malloc/free, but replacing realloc requires knowing the old size,
	// which we don't have.
	// The easiest fix is to simply copy. What can you do?
	Pixel *newout = new Pixel[(size_t)w * h];
	::memcpy((void *)newout, (void *)out, (size_t)w * h * 4);
	free(out);
	return { newout, { w, h } };
}

void Image::copy_from(const Image &src, Rect<uint16_t> from, Vec2<uint16_t> to) {
	if (src.dim().x < from.high().x || src.dim().y < from.high().y)
		throw std::exception();
	Vec2<uint32_t> from_dim = from.dim();
	if (dim().x < (uint32_t)to.x + from_dim.x || dim().y < (uint32_t)to.y + from_dim.y)
		throw std::exception();

	for (uint16_t y = 0; y < from.dim().y; y++) {
		Pixel *rd = row(to.y + y);
		const Pixel *rs = src.row(from.low().y + y);
		::memcpy((void *)(rd + to.x), (void *)(rs + from.low().x), from.dim().x * 4ULL);
	}
}

void Image::fill(Pixel value) {
	for (size_t i = 0; i < area(); i++)
		m_data[i] = value;
}

Vec2<uint16_t> Image::dim() const {
	return m_dim;
}

const Pixel *Image::data() const {
	return m_data;
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
