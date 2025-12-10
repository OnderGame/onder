#include <cstddef>
#include <onder/filesystem.hpp>
#include <onder/graphics.hpp>
#include <onder/world.hpp>
#include <stdio.h>

int main(int argc, char **argv)
{
	onder::graphics::Window display("Hello framebuffer!", 480, 320);
	onder::world::World world;

	onder::filesystem::FileMmap png("assets/tiles/stone.png");
	auto img = onder::graphics::Image::from_png(png.slice());
	display.draw(0, 0, img);
	display.draw(64, 64, img);
	display.draw(128, 128, img);
	display.draw(128, 192, img);
	display.draw(192, 192, img);

	while (display.is_open()) {
		display.update();
	}

	return 0;
}
