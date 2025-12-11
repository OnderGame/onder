#include <cstddef>
#include <onder/filesystem.hpp>
#include <onder/graphics.hpp>
#include <onder/net.hpp>
#include <onder/world.hpp>
#include <iostream>

int main(int argc, char **argv)
{
	using namespace onder::collections;
	using namespace onder::graphics;
	using namespace onder::world;
	using namespace onder::filesystem;
	using namespace onder::math;
	using namespace onder::net;

	const Vec2 DIM(36, 12);

	Window display("Hello framebuffer!", DIM * 64);
	World world(256);

	Array<Image, 4> tiles;

	FileMmap png("assets/tiles/stone.png");

	InputListener inputs;
	display.set_listener(inputs);

	Udp<Ip4> server({ {}, 3333 });
	List<uint8_t> recvbuffer;

	Poller poller;
	poller.add(server);

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

	int32_t x = 64*2, y = 64*2;

	while (display.is_open()) {
		display.clear({ 0, 0, 0, 0 });
		auto display_rect = Rect<int32_t>::from_pos_size({}, display.dim());
		int32_t ox = x / 64, oy = y / 64;
		int32_t hx = x % 64, hy = y % 64;
		for (int32_t dy = 0; dy <= DIM.y; dy++) {
			for (int32_t dx = 0; dx <= DIM.x; dx++) {
				const auto &img = tiles[world[0, ox + dx, oy + dy].id];
				Vec2<int32_t> pos(dx * 64 - hx, dy * 64 - hy);
				auto to = Rect<int32_t>::from_pos_size(pos, { 64, 64 });
				to &= display_rect;
				auto from = to - pos;
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
		recvbuffer.reserve(1024);
		SocketAddr<Ip4> addr;
		if (poller.poll(0) > 0) {
			if (server.recv(recvbuffer, addr) < 0)
				throw std::exception();
			(std::cout << addr << " -> ").write((char *)recvbuffer.ptr(), recvbuffer.len()) << std::endl;
		}
	}

	return 0;
}
