#include <cerrno>
#include <csignal>
#include <cstddef>
#include <sys/prctl.h>
#include <onder/filesystem.hpp>
#include <onder/graphics.hpp>
#include <onder/net.hpp>
#include <onder/world.hpp>
#include <iostream>

using namespace onder::collections;
using namespace onder::graphics;
using namespace onder::world;
using namespace onder::filesystem;
using namespace onder::math;
using namespace onder::net;

template<typename T>
T read_raw(void *src) {
	T x;
	::memcpy((void *)&x, src, sizeof(x));
	return x;
}
template<typename T>
void append_raw(List<uint8_t> &buf, const T &x) {
	buf.append((uint8_t *)&x, sizeof(x));
}

void server() {
	World world(256);
	Poller poller;
	Udp<Ip4> server({ {}, 3333 });
	List<uint8_t> recvbuffer;

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

	std::cout << "server started" << std::endl;
	poller.add(server);
	recvbuffer.reserve(1 << 13);
	while (1) {
		for (size_t i = 0; i < poller.poll(1000); i++) {
			SocketAddr<Ip4> addr;
			if (server.recv(recvbuffer, addr) < 0)
				throw std::exception();
			std::cout << addr << " <-  " << recvbuffer.len() << std::endl;
			if (recvbuffer.len() < 2)
				continue; // ignore invalid requests
			uint16_t subsystem = read_raw<uint16_t>((void*)recvbuffer.ptr());
			std::cout << "     ~ " << subsystem << std::endl;
			switch (subsystem) {
			case 0: {
				if (recvbuffer.len() < 12)
					continue;
				uint32_t x = read_raw<uint32_t>((void*)(recvbuffer.ptr() + 4));
				uint32_t y = read_raw<uint32_t>((void*)(recvbuffer.ptr() + 8));
				const Chunk &chunk = world.chunk(0, x, y);
				recvbuffer.clear();
				append_raw(recvbuffer, subsystem);
				append_raw(recvbuffer, (uint16_t)0);
				append_raw(recvbuffer, x);
				append_raw(recvbuffer, y);
				recvbuffer.append((uint8_t *)&chunk, sizeof(chunk));
				std::cout <<  "  -> " << recvbuffer.len() << std::endl;
				server.send(recvbuffer, addr);
				break;
			}
			default: continue;
			}
		}
	}
}

void client() {
	World world(256);
	const Vec2 DIM(36, 12);
	Array<Image, 4> tiles;
	FileMmap png("assets/tiles/stone.png");
	Window display("Hello framebuffer!", DIM * 64);
	InputListener inputs;
	Poller poller;
	Udp<Ip4> client({ { 127, 0, 0, 1 }, 0 });
	List<uint8_t> recvbuffer;
	SocketAddr<Ip4> server_addr({ 127, 0, 0, 1 }, 3333);

	display.set_listener(inputs);

	poller.add(client);

	auto no_tile = Image::filled({ 64, 64 }, {});
	tiles[0] = Image::filled({ 64, 64 }, { 127, 127, 127, 127 });
	tiles[1] = Image::from_png(png.slice());
	tiles[2] = Image::filled({ 64, 64 }, { 255, 255, 0, 255 });
	tiles[3] = Image::filled({ 64, 64 }, { 255, 0, 0, 255 });

	uint32_t x = 64*2, y = 64*2;

	recvbuffer.reserve(1 << 13);

	for(int i=0;i<12;i++)recvbuffer.push(0);
	client.send(recvbuffer, server_addr);
	recvbuffer[4] = 1;
	client.send(recvbuffer, server_addr);

	std::cout << "client started" << std::endl;
	while (display.is_open()) {
		auto display_rect = Rect<int32_t>::from_pos_size({}, display.dim());
		int32_t ox = x / 64, oy = y / 64;
		int32_t hx = x % 64, hy = y % 64;
		for (int32_t dy = 0; dy <= DIM.y; dy++) {
			for (int32_t dx = 0; dx <= DIM.x; dx++) {
				const auto &tile = world[0, ox + dx, oy + dy];
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
		for (size_t i = 0; i < poller.poll(0); i++) {
			SocketAddr<Ip4> addr_stub;
			if (client.recv(recvbuffer, addr_stub) < 0)
				throw std::exception();
			std::cout << addr_stub << " <-  " << recvbuffer.len() << std::endl;
			if (recvbuffer.len() < 2)
				continue; // ignore invalid requests
			uint16_t subsystem = read_raw<uint16_t>((void*)recvbuffer.ptr());
			std::cout << " " << subsystem << std::endl;
			switch (subsystem) {
			case 0: {
				if (recvbuffer.len() < 12 + sizeof(Chunk))
					continue;
				uint32_t x = read_raw<uint32_t>((void*)(recvbuffer.ptr() + 4));
				uint32_t y = read_raw<uint32_t>((void*)(recvbuffer.ptr() + 8));
				Chunk &chunk = world.chunk(0, x, y);
				::memcpy((void *)&chunk, (void*)&recvbuffer[12], sizeof(chunk));
				break;
			}
			default: continue;
			}
		}
	}
}

void adhoc() {
	pid_t child = fork();
	if (child < 0)
		throw std::exception();
	if (child == 0) {
		// FIXME potential race condition here... bah
		if (::prctl(PR_SET_PDEATHSIG, SIGINT) < 0)
			std::cerr << "prctrl(PR_SET_PDEATHSIG, SIGINT) failed: " << ::strerror(errno) << std::endl;
		server();
	}
	else
		client();
}

int main(int argc, char **argv) {
	adhoc();
	return 0;
}
