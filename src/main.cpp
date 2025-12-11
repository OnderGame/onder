#include <cstddef>
#include <onder/filesystem.hpp>
#include <onder/graphics.hpp>
#include <onder/world.hpp>
#include <iostream>

int main(int argc, char **argv)
{
	using namespace onder::collections;
	using namespace onder::graphics;
	using namespace onder::world;
	using namespace onder::filesystem;

	const size_t W = 36;

	Window display("Hello framebuffer!", 64 * W, 640);
	World world(256);

	Array<Image, 4> tiles;

	FileMmap png("assets/tiles/stone.png");

	tiles[0] = Image::filled(64, 64, { 127, 127, 127, 127 });
	tiles[1] = Image::from_png(png.slice());
	tiles[2] = Image::filled(64, 64, { 255, 255, 0, 255 });
	tiles[3] = Image::filled(64, 64, { 255, 0, 0, 255 });

	for (uint32_t y = 0; y < 64; y++) {
		for (uint32_t x = 0; x < 64; x++) {
			world[0, x, y].id = 0;
		}
	}

	world[0, 0, 0].id = 1;
	world[0, 1, 0].id = 2;
	world[0, 1, 1].id = 3;
	world[0, 2, 2].id = 1;
	world[0, 3, 2].id = 1;
	world[0, 4, 2].id = 1;
	world[0, 34, 5].id = 1;

	for (uint32_t x = 0; x < W; x++) {
		for (uint32_t y = 0; y < 10; y++) {
			const auto &img = tiles[world[0, x, y].id];
			display.draw(x*64, y*64, img);
		}
	}

	while (display.is_open()) {
		display.update();
	}

	return 0;
}
