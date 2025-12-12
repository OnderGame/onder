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

Server::Server(const net::SocketAddr<net::Ip4> &addr)
	: m_world({ 256, 16 })
	, ip4(addr)
{
	for (uint32_t y = 0; y < 64; y++) {
		for (uint32_t x = 0; x < 64; x++) {
			m_world[0, x, y].id = 0;
		}
	}

	m_world[0, 2, 2].id = 1;
	m_world[0, 3, 2].id = 2;
	m_world[0, 3, 3].id = 3;
	m_world[0, 4, 4].id = 1;
	m_world[0, 5, 4].id = 1;
	m_world[0, 6, 4].id = 1;
	m_world[0, 34, 7].id = 1;
}

void Server::run() {
	poller.add(ip4);
	while (1) {
		poll();
	}
}

void Server::poll() {
	buffer.clear();
	buffer.reserve(1 << 13);

	for (size_t i = 0; i < poller.poll(1000); i++) {
		net::SocketAddr<net::Ip4> addr;
		if (ip4.recv(buffer, addr) < 0)
			throw std::exception();
		std::cout << addr << " <-  " << buffer.len() << std::endl;
		if (buffer.len() < 2)
			continue; // ignore invalid requests
		uint16_t subsystem = read_raw<uint16_t>((void*)buffer.ptr());
		std::cout << "     ~ " << subsystem << std::endl;
		switch (subsystem) {
		case 0: {
			if (buffer.len() < 12)
				continue;
			uint32_t x = read_raw<uint32_t>((void*)(buffer.ptr() + 4));
			uint32_t y = read_raw<uint32_t>((void*)(buffer.ptr() + 8));
			const world::Chunk &chunk = m_world.chunk(0, x, y);
			buffer.clear();
			append_raw(buffer, subsystem);
			append_raw(buffer, (uint16_t)0);
			append_raw(buffer, x);
			append_raw(buffer, y);
			buffer.append((uint8_t *)&chunk, sizeof(chunk));
			std::cout <<  "  -> " << buffer.len() << std::endl;
			ip4.send(buffer, addr);
			break;
		}
		default: continue;
		}
	}
}

}
}
