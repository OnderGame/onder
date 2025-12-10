#include <cstddef>
#include <onder/graphics.hpp>

int main(int argc, char **argv)
{
	onder::graphics::Window display("Hello framebuffer!", 480, 320);

	onder::graphics::Pixel pixels[320 * 480];

	for (size_t y = 0; y < 320; y++) {
		for (size_t x = 0; x < 480; x++) {
			auto &p = pixels[y * 480 + x];
			p.r = x * 255 / 480;
			p.g = y * 255 / 320;
			p.b = (511 - p.r - p.g) / 2;
			p.a = 255;
		}
	}

	while (display.is_open()) {
		display.update(pixels);
	}

	return 0;
}
