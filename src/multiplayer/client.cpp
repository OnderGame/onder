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

Client::Client(const net::SocketAddr<net::Ip4> &addr, const net::SocketAddr<net::Ip4> &server_addr)
	: m_world({ 256, 16 })
	, ip4(addr)
	, server_addr(server_addr)
{
	poller.add(ip4);
}

void Client::poll() {
	buffer.clear();
	buffer.reserve(1 << 13);
	for (size_t i = 0; i < poller.poll(0); i++) {
		net::SocketAddr<net::Ip4> addr_stub;
		if (ip4.recv(buffer, addr_stub) < 0)
			throw std::exception();
		std::cout << addr_stub << " <-  " << buffer.len() << std::endl;
		if (buffer.len() < 2)
			continue; // ignore invalid requests
		uint16_t subsystem = read_raw<uint16_t>((void*)buffer.ptr());
		std::cout << " " << subsystem << std::endl;
		switch (subsystem) {
		case 0: {
			if (buffer.len() < 12 + sizeof(world::Chunk))
				continue;
			uint32_t x = read_raw<uint32_t>((void*)(buffer.ptr() + 4));
			uint32_t y = read_raw<uint32_t>((void*)(buffer.ptr() + 8));
			world::Chunk &chunk = m_world.chunk(0, x, y);
			::memcpy((void *)&chunk, (void*)&buffer[12], sizeof(chunk));
			break;
		}
		default: continue;
		}
	}
}

void Client::send() {
	ip4.send(buffer, server_addr);
}

const world::World &Client::world() const {
	return m_world;
}

}
}
