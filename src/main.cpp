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
	using namespace onder::math;

	const Vec2 DIM(36, 12);

	Window display("Hello framebuffer!", DIM * 64);
	World world(256);

	Array<Image, 4> tiles;

	FileMmap png("assets/tiles/stone.png");

	InputListener inputs;
	display.set_listener(inputs);

	tiles[0] = Image::filled({ 64, 64 }, { 127, 127, 127, 127 });
	tiles[1] = Image::from_png(png.slice());
	tiles[2] = Image::filled({ 64, 64 }, { 255, 255, 0, 255 });
	tiles[3] = Image::filled({ 64, 64 }, { 255, 0, 0, 255 });

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
	world[0, 34, 7].id = 1;

	int x = 64*2, y = 64*2;

	while (display.is_open()) {
		display.clear({ 0, 0, 0, 0 });
		for (uint32_t dy = 0; dy < DIM.y; dy++) {
			for (uint32_t dx = 0; dx < DIM.x; dx++) {
				int ox = x / 64, oy = y / 64;
				const auto &img = tiles[world[0, ox + dx, oy + dy].id];
				Rect<uint16_t> to = Rect<uint16_t>::from_pos_size({ dx*64, dy*64 }, { 64, 64 });
				//to = to.intersection(display.rect())
				Rect<uint16_t> from({}, { 64, 64 });
				display.draw(img, from, to.low());
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
