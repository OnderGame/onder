#pragma once

#include <onder/math/vec2.hpp>
#include <onder/net.hpp>
#include <onder/world.hpp>

namespace onder {
namespace multiplayer {

class ISubSystem {
public:
	virtual void handle_packet(const net::SocketAddr<net::Ip4> &address, collections::Slice<const uint8_t> data) = 0;
};

class IClientSubSystem : public ISubSystem {};
class IServerSubSystem : public ISubSystem {};

class Client {
	net::Udp<net::Ip4> ip4;
	net::Poller poller;
	net::SocketAddr<net::Ip4> server_addr;
	collections::List<uint8_t> buffer;
	collections::List<IClientSubSystem *> m_subsystems;

	Client(const Client &) = delete;
	Client &operator=(const Client &) = delete;

public:
	Client(const net::SocketAddr<net::Ip4> &addr, const net::SocketAddr<net::Ip4> &server_addr);

	void poll();

	collections::List<uint8_t> &send_begin(uint16_t subsystem);
	void send_end();

	void add_subsystem(IClientSubSystem &subsystem);
};

class Server {
	net::Udp<net::Ip4> ip4;
	net::Poller poller;
	// use two separate buffers to reduce the potential for mistakes.
	collections::List<uint8_t> recv_buffer, send_buffer;
	collections::List<IServerSubSystem *> m_subsystems;

	Server(const Server &) = delete;
	Server &operator=(const Server &) = delete;

public:
	Server(const net::SocketAddr<net::Ip4> &addr);

	void poll();
	void run();

	collections::List<uint8_t> &send_begin(uint16_t subsystem);
	void send_end(const net::SocketAddr<net::Ip4> &address);

	void add_subsystem(IServerSubSystem &subsystem);
};

class ClientChunkManager : public IClientSubSystem {
	world::World &m_world;
	Client &m_client;

public:
	ClientChunkManager(world::World &world, Client &client);

	void handle_packet(const net::SocketAddr<net::Ip4> &address, collections::Slice<const uint8_t> data);

	void request_chunk(uint16_t depth, math::Vec2<uint32_t> chunk_pos);
	void request_tile(uint16_t depth, math::Vec2<uint32_t> tile_pos);
};

class ServerChunkManager : public IServerSubSystem {
	world::World &m_world;
	Server &m_server;

public:
	ServerChunkManager(world::World &world, Server &server);

	void handle_packet(const net::SocketAddr<net::Ip4> &address, collections::Slice<const uint8_t> data);
};

}
}
