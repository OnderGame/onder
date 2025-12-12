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

class ClientChunkManager : public IClientSubSystem {
	World &m_world;

public:
	ClientChunkManager(World &world) : m_world(world) {}

	void handle_packet(const SocketAddr<Ip4> &address, Slice<const uint8_t> data);
};

class ServerChunkManager : public IServerSubSystem {
	World &m_world;
	Server &m_server;

public:
	ServerChunkManager(World &world, Server &server) : m_world(world), m_server(server) {}

	void handle_packet(const SocketAddr<Ip4> &address, Slice<const uint8_t> data);
};

void ClientChunkManager::handle_packet(const SocketAddr<Ip4> &addr, Slice<const uint8_t> data) {
	if (data.len() < 10 + sizeof(Chunk))
		return;
	uint16_t d = read_raw<uint16_t>((void*)(data.ptr() + 0));
	uint32_t x = read_raw<uint32_t>((void*)(data.ptr() + 2));
	uint32_t y = read_raw<uint32_t>((void*)(data.ptr() + 6));
	Chunk &chunk = m_world.chunk(d, x, y);
	::memcpy((void *)&chunk, (void*)&data[10], sizeof(chunk));
}

void ServerChunkManager::handle_packet(const SocketAddr<Ip4> &addr, Slice<const uint8_t> data) {
	if (data.len() < 10)
		return;
	uint16_t d = read_raw<uint16_t>((void*)(data.ptr() + 0));
	uint32_t x = read_raw<uint32_t>((void*)(data.ptr() + 2));
	uint32_t y = read_raw<uint32_t>((void*)(data.ptr() + 6));
	const Chunk &chunk = m_world.chunk(d, x, y);
	auto &buffer = m_server.send_begin(0); // FIXME don't hardcode subsystem
	append_raw(buffer, (uint16_t)0);
	append_raw(buffer, x);
	append_raw(buffer, y);
	buffer.append((uint8_t *)&chunk, sizeof(chunk));
	m_server.send_end(addr);
}

void server(const SocketAddr<Ip4> &address) {
	Server server(address);
	World world(256, 16);
	ServerChunkManager chunker(world, server);

	server.add_subsystem(chunker);

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
	server.run();
}

void client(const Ip4 &client_addr, const SocketAddr<Ip4> &server_addr) {
	Client client({ client_addr, 0 }, server_addr);
	const Vec2 DIM(36, 20);
	Array<Image, 4> tiles;
	FileMmap png("assets/tiles/stone.png");
	Window display("Hello framebuffer!", DIM * 64);
	InputListener inputs;
	World world(256, 16);
	ClientChunkManager chunker(world);

	display.set_listener(inputs);
	client.add_subsystem(chunker);

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
