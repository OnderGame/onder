#include "PixelToaster.h"

int main(int argc, char **argv)
{
	PixelToaster::Display display("Hello framebuffer!", 480, 320);

	PixelToaster::Pixel pixels[320 * 480];

	for (size_t y = 0; y < 320; y++) {
		for (size_t x = 0; x < 480; x++) {
			auto &p = pixels[y * 480 + x];
			p.r = x / 480.0f;
			p.g = y / 320.0f;
			p.b = (2 - p.r - p.g) / 2;
			p.a = 1.0f;
		}
	}

	while (display.open()) {
		display.update(pixels);
	}

	return 0;
}
