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

template<typename T>
static void append_num(collections::List<uint8_t> &buf, T x) {
	for (int i = 0; i < sizeof(T); i++)
		buf.push((x >> 8*i) & 0xff);
}

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
		uint16_t subsystem = read_raw<uint16_t>((void*)buffer.ptr());
		std::cout << " " << subsystem << std::endl;
		if (subsystem >= m_subsystems.len())
			continue; // just ignore
		m_subsystems[subsystem]->handle_packet(addr, buffer.slice(2, buffer.len()));
	}
}

collections::List<uint8_t> &Client::send_begin(uint16_t subsystem) {
	buffer.clear();
	append_num(buffer, subsystem);
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
	if (data.len() < 10 + sizeof(world::Chunk))
		return;
	uint16_t d = read_raw<uint16_t>((void*)(data.ptr() + 0));
	uint32_t x = read_raw<uint32_t>((void*)(data.ptr() + 2));
	uint32_t y = read_raw<uint32_t>((void*)(data.ptr() + 6));
	world::Chunk &chunk = m_world.chunk(d, x, y);
	::memcpy((void *)&chunk, (void*)&data[10], sizeof(chunk));
}

void ClientChunkManager::request_chunk(uint16_t depth, math::Vec2<uint32_t> chunk_pos) {
	auto &send = m_client.send_begin(0); // FIXME don't hardcode
	append_num(send, depth);
	append_num(send, chunk_pos.x);
	append_num(send, chunk_pos.y);
	m_client.send_end();
}

void ClientChunkManager::request_tile(uint16_t depth, math::Vec2<uint32_t> chunk_pos) {
	request_chunk(depth, chunk_pos / world::CHUNK_DIM);
}

}
}
