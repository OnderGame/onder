#include <onder/collections/array.hpp>
#include <onder/multiplayer.hpp>
#include <cstring>

namespace onder {
namespace multiplayer {

using namespace onder::collections;

Server::Server(const net::SocketAddr<net::Ip4> &addr) : ip4(addr) {
}

void Server::run() {
	poller.add(ip4);
	while (1) {
		poll();
	}
}

void Server::poll() {
	recv_buffer.clear();
	recv_buffer.reserve(1 << 13);

	for (size_t i = 0; i < poller.poll(1000); i++) {
		net::SocketAddr<net::Ip4> addr;
		if (ip4.recv(recv_buffer, addr) < 0)
			throw std::exception();
		std::cout << addr << " <-  " << recv_buffer.len() << std::endl;
		if (recv_buffer.len() < 2)
			continue; // ignore invalid requests
		auto subsystem = net::from_le_bytes<uint16_t>(*(Array<uint8_t, 2> *)&recv_buffer[0]);
		std::cout << "     ~ " << subsystem << std::endl;
		if (subsystem >= m_subsystems.len())
			continue; // just ignore
		m_subsystems[subsystem]->handle_packet(addr, recv_buffer.slice(2, recv_buffer.len()));
	}
}

collections::List<uint8_t> &Server::send_begin(uint16_t subsystem) {
	send_buffer.clear();
	send_buffer.append(net::to_le_bytes<uint16_t>(subsystem));
	return send_buffer;
}

void Server::send_end(const net::SocketAddr<net::Ip4> &address) {
	ip4.send(send_buffer, address);
}

void Server::add_subsystem(IServerSubSystem &subsystem) {
	m_subsystems.push(&subsystem);
}

ServerChunkManager::ServerChunkManager(world::World &world, Server &server)
	: m_world(world)
	, m_server(server)
{}

void ServerChunkManager::handle_packet(const net::SocketAddr<net::Ip4> &addr, collections::Slice<const uint8_t> data) {
	if (data.len() < 10)
		return;
	auto y = net::from_le_bytes<uint32_t>(*(Array<uint8_t, 4> *)&data[6]);
	auto x = net::from_le_bytes<uint32_t>(*(Array<uint8_t, 4> *)&data[2]);
	auto d = net::from_le_bytes<uint16_t>(*(Array<uint8_t, 2> *)&data[0]);
	const world::Chunk &chunk = m_world.chunk(d, x, y);
	auto &buffer = m_server.send_begin(0); // FIXME don't hardcode subsystem
	buffer.reserve(2 + 4*2 + 32*32*4);
	buffer.append(net::to_le_bytes<uint16_t>(0));
	buffer.append(net::to_le_bytes<uint32_t>(x));
	buffer.append(net::to_le_bytes<uint32_t>(y));
	buffer.append((uint8_t *)&chunk, sizeof(chunk));
	m_server.send_end(addr);
}

}
}
