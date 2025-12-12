#include <onder/multiplayer.hpp>
#include <cstring>

namespace onder {
namespace multiplayer {

template<typename T>
static T read_raw(void *src) {
	T x;
	::memcpy((void *)&x, src, sizeof(x));
	return x;
}
template<typename T>
static void append_raw(collections::List<uint8_t> &buf, const T &x) {
	buf.append((uint8_t *)&x, sizeof(x));
}

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
		uint16_t subsystem = read_raw<uint16_t>((void*)recv_buffer.ptr());
		std::cout << "     ~ " << subsystem << std::endl;
		if (subsystem >= m_subsystems.len())
			continue; // just ignore
		m_subsystems[subsystem]->handle_packet(addr, recv_buffer.slice(2, recv_buffer.len()));
	}
}

collections::List<uint8_t> &Server::send_begin(uint16_t subsystem) {
	send_buffer.clear();
	send_buffer.push((subsystem >> 0) & 0xff);
	send_buffer.push((subsystem >> 8) & 0xff);
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
	uint16_t d = read_raw<uint16_t>((void*)(data.ptr() + 0));
	uint32_t x = read_raw<uint32_t>((void*)(data.ptr() + 2));
	uint32_t y = read_raw<uint32_t>((void*)(data.ptr() + 6));
	const world::Chunk &chunk = m_world.chunk(d, x, y);
	auto &buffer = m_server.send_begin(0); // FIXME don't hardcode subsystem
	append_raw(buffer, (uint16_t)0);
	append_raw(buffer, x);
	append_raw(buffer, y);
	buffer.append((uint8_t *)&chunk, sizeof(chunk));
	m_server.send_end(addr);
}

}
}
