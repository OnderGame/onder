#pragma once

#include <onder/net.hpp>
#include <onder/world.hpp>

namespace onder {
namespace multiplayer {

class ISubSystem {
public:
	virtual void handle_packet(collections::Slice<const uint8_t> data) = 0;
};

class IClientSubSystem : public ISubSystem {};

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
