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

public:
	collections::List<uint8_t> buffer; // FIXME don't make this public!

	Client(const net::SocketAddr<net::Ip4> &addr, const net::SocketAddr<net::Ip4> &server_addr);

	const world::World &world() const;

	void poll();
	void send();
};

class Server {
	world::World m_world;
	net::Udp<net::Ip4> ip4;
	net::Poller poller;
	collections::List<uint8_t> buffer;

public:
	Server(const net::SocketAddr<net::Ip4> &addr);

	void poll();
	void run();
};

}
}
