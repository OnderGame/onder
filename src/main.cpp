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

	Window display("Hello framebuffer!", 64 * W, 768);
	World world(256);

	Array<Image, 4> tiles;

	FileMmap png("assets/tiles/stone.png");

	InputListener inputs;
	display.set_listener(inputs);

	tiles[0] = Image::filled(64, 64, { 127, 127, 127, 127 });
	tiles[1] = Image::from_png(png.slice());
	tiles[2] = Image::filled(64, 64, { 255, 255, 0, 255 });
	tiles[3] = Image::filled(64, 64, { 255, 0, 0, 255 });

	for (uint32_t y = 0; y < 64; y++) {
		for (uint32_t x = 0; x < 64; x++) {
			world[0, x, y].id = 0;
		}
	}

	world[0, 2, 2].id = 1;
	world[0, 3, 2].id = 2;
	world[0, 3, 3].id = 3;
	world[0, 4, 4].id = 1;
	world[0, 5, 4].id = 1;
	world[0, 6, 4].id = 1;
	world[0, 36, 7].id = 1;

	int x = 0, y = 0;

	while (display.is_open()) {
		display.clear({ 0, 0, 0, 0 });
		for (uint32_t dx = 2; dx < W - 2; dx++) {
			for (uint32_t dy = 2; dy < 10; dy++) {
				const auto &img = tiles[world[0, dx, dy].id];
				display.draw(x+dx*64, y+dy*64, img);
			}
		}
		display.update();
		for (const auto evt : inputs.events()) {
			switch (evt.key.code) {
			case 'Q': --x; break;
			case 'D': ++x; break;
			case 'Z': --y; break;
			case 'S': ++y; break;
			default: break;
			}
		}
		inputs.clear_events();
	}

	return 0;
}
