#include <onder/multiplayer.hpp>
#include <cstring>

using namespace onder::collections;

namespace onder {
namespace multiplayer {

Client::Client(const net::SocketAddr<net::Ip4> &addr, const net::SocketAddr<net::Ip4> &server_addr)
	: ip4(addr)
	, server_addr(server_addr)
{
	poller.add(ip4);
}

void Client::poll() {
	buffer.clear();
	buffer.reserve(1 << 13);
	for (size_t i = 0; i < poller.poll(0); i++) {
		net::SocketAddr<net::Ip4> addr;
		if (ip4.recv(buffer, addr) < 0)
			throw std::exception();
		std::cout << addr << " <-  " << buffer.len() << std::endl;
		if (buffer.len() < 2)
			continue; // ignore invalid requests
		auto subsystem = net::from_le_bytes<uint16_t>(*(Array<uint8_t, 2> *)&buffer[0]);
		std::cout << " " << subsystem << std::endl;
		if (subsystem >= m_subsystems.len())
			continue; // just ignore
		m_subsystems[subsystem]->handle_packet(addr, buffer.slice(2, buffer.len()));
	}
}

collections::List<uint8_t> &Client::send_begin(uint16_t subsystem) {
	buffer.clear();
	buffer.append(net::to_le_bytes<uint16_t>(subsystem));
	return buffer;
}

void Client::send_end() {
	ip4.send(buffer, server_addr);
}

void Client::add_subsystem(IClientSubSystem &subsystem) {
	m_subsystems.push(&subsystem);
}

ClientChunkManager::ClientChunkManager(world::World &world, Client &client)
	: m_world(world)
	, m_client(client)
{}

void ClientChunkManager::handle_packet(const net::SocketAddr<net::Ip4> &addr, collections::Slice<const uint8_t> data) {
	(void)addr;
	if (data.len() < 10 + sizeof(world::Chunk))
		return;
	auto y = net::from_le_bytes<uint32_t>(*(Array<uint8_t, 4> *)&data[6]);
	auto x = net::from_le_bytes<uint32_t>(*(Array<uint8_t, 4> *)&data[2]);
	auto d = net::from_le_bytes<uint16_t>(*(Array<uint8_t, 2> *)&data[0]);
	world::Chunk &chunk = m_world.chunk(d, x, y);
	::memcpy((void *)&chunk, (void*)&data[10], sizeof(chunk));
}

void ClientChunkManager::request_chunk(uint16_t depth, math::Vec2<uint32_t> chunk_pos) {
	auto &send = m_client.send_begin(0); // FIXME don't hardcode
	send.append(net::to_le_bytes<uint16_t>(depth));
	send.append(net::to_le_bytes<uint32_t>(chunk_pos.x));
	send.append(net::to_le_bytes<uint32_t>(chunk_pos.y));
	m_client.send_end();
}

void ClientChunkManager::request_tile(uint16_t depth, math::Vec2<uint32_t> chunk_pos) {
	request_chunk(depth, chunk_pos / world::CHUNK_DIM);
}

}
}
