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
	buffer.push((subsystem >> 0) & 0xff);
	buffer.push((subsystem >> 8) & 0xff);
	return buffer;
}

void Client::send_end() {
	ip4.send(buffer, server_addr);
}

void Client::add_subsystem(IClientSubSystem &subsystem) {
	m_subsystems.push(&subsystem);
}

}
}
