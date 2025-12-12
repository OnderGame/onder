#pragma once

#include <onder/net.hpp>
#include <onder/world.hpp>

namespace onder {
namespace multiplayer {

class Client {
	world::World m_world;
	net::Udp<net::Ip4> ip4;
	net::Poller poller;
	net::SocketAddr<net::Ip4> server_addr;
	collections::List<uint8_t> buffer; // FIXME don't make this public!

	Client(const Client &) = delete;
	Client &operator=(const Client &) = delete;

public:
	Client(const net::SocketAddr<net::Ip4> &addr, const net::SocketAddr<net::Ip4> &server_addr);

	const world::World &world() const;

	void poll();

	collections::List<uint8_t> &send_begin(uint16_t subsystem);
	void send_end();
};

class Server {
	world::World m_world;
	net::Udp<net::Ip4> ip4;
	net::Poller poller;
	collections::List<uint8_t> buffer;

	Server(const Server &) = delete;
	Server &operator=(const Server &) = delete;

public:
	Server(const net::SocketAddr<net::Ip4> &addr);

	void poll();
	void run();
};

}
}
