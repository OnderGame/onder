#include <cerrno>
#include <csignal>
#include <cstddef>
#include <sys/prctl.h>
#include <onder/filesystem.hpp>
#include <onder/graphics.hpp>
#include <onder/net.hpp>
#include <onder/world.hpp>
#include <onder/multiplayer.hpp>
#include <iostream>

using namespace onder::collections;
using namespace onder::graphics;
using namespace onder::world;
using namespace onder::filesystem;
using namespace onder::math;
using namespace onder::net;
using namespace onder::multiplayer;

template<typename T>
static T read_raw(void *src) {
	T x;
	::memcpy((void *)&x, src, sizeof(x));
	return x;
}
template<typename T>
static void append_raw(List<uint8_t> &buf, const T &x) {
	buf.append((uint8_t *)&x, sizeof(x));
}

void server(const SocketAddr<Ip4> &address) {
	Server server(address);
	std::cout << "server started" << std::endl;
	server.run();
}

void client(const Ip4 &client_addr, const SocketAddr<Ip4> &server_addr) {
	Client client({ client_addr, 0 }, server_addr);
	const Vec2 DIM(36, 20);
	Array<Image, 4> tiles;
	FileMmap png("assets/tiles/stone.png");
	Window display("Hello framebuffer!", DIM * 64);
	InputListener inputs;

	display.set_listener(inputs);

	auto no_tile = Image::filled({ 64, 64 }, {});
	tiles[0] = Image::filled({ 64, 64 }, { 127, 127, 127, 127 });
	tiles[1] = Image::from_png(png.slice());
	tiles[2] = Image::filled({ 64, 64 }, { 255, 255, 0, 255 });
	tiles[3] = Image::filled({ 64, 64 }, { 255, 0, 0, 255 });

	uint32_t x = 64*2, y = 64*2;

	{
		auto &send = client.send_begin(0);
		for(int i=0;i<10;i++) send.push(0);
		client.send_end();
	}
	{
		auto &send = client.send_begin(0);
		for(int i=0;i<10;i++) send.push(0);
		send[4] = 1;
		client.send_end();
	}

	std::cout << "client started" << std::endl;
	while (display.is_open()) {
		auto display_rect = Rect<int32_t>::from_pos_size({}, display.dim());
		int32_t ox = x / 64, oy = y / 64;
		int32_t hx = x % 64, hy = y % 64;
		for (int32_t dy = 0; dy <= DIM.y; dy++) {
			for (int32_t dx = 0; dx <= DIM.x; dx++) {
				const auto &tile = client.world()[0, ox + dx, oy + dy];
				const auto &img = tile.is_valid() ? tiles[tile.id] : no_tile;
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
		client.poll();
	}
}

void adhoc() {
	const Ip4 localhost(127, 0, 0, 1);
	pid_t child = fork();
	if (child < 0)
		throw std::exception();
	if (child == 0) {
		// FIXME potential race condition here... bah
		if (::prctl(PR_SET_PDEATHSIG, SIGINT) < 0)
			std::cerr << "prctrl(PR_SET_PDEATHSIG, SIGINT) failed: " << ::strerror(errno) << std::endl;
		server({ localhost, 3333 });
	}
	else
		client(localhost, { localhost, 3333 });
}

int main(int argc, char **argv) {
	adhoc();
	return 0;
}
