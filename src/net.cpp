#include <onder/net.hpp>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iomanip>

namespace onder {
namespace net {

using namespace onder::collections;

template<>
Udp<Ip4>::Udp(const SocketAddr<Ip4> &address) : m_fd(socket(AF_INET, SOCK_DGRAM, 0)) {
	if (m_fd < 0)
		throw std::exception();
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::htonl(address.addr.addr);
	addr.sin_port = ::htons(address.port);
	if (bind(m_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		::close(m_fd);
		throw std::exception();
	}
}

template<>
Udp<Ip4>::~Udp() {
	::close(m_fd);
}

// we can't put this on top because
//
//   error: specialization of ‘onder::net::Udp<A>::Udp(const onder::net::SocketAddr<A>&) [with A = onder::net::Ip4]’ after instantiation
//
// ????
template<>
Udp<Ip4>::Udp() : Udp(SocketAddr<Ip4>()) {}

template<>
int Udp<Ip4>::fd() const {
	return m_fd;
}

template<>
int Udp<Ip4>::recv(collections::List<uint8_t> &buffer, SocketAddr<Ip4> &address) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	ssize_t res = ::recvfrom(m_fd, (void *)buffer.ptr(), buffer.capacity(), MSG_NOSIGNAL, (struct sockaddr *)&addr, &addrlen);
	if (res < 0)
		return -1;
	address.addr.addr = ntohl(addr.sin_addr.s_addr);
	address.port = ntohs(addr.sin_port);
	buffer.set_len(res);
	return 0;
}

template<>
int Udp<Ip4>::send(collections::Slice<const uint8_t> data, const SocketAddr<Ip4> &address) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::htonl(address.addr.addr);
	addr.sin_port = ::htons(address.port);
	ssize_t res = ::sendto(m_fd, (void *)data.ptr(), data.len(), MSG_NOSIGNAL, (struct sockaddr *)&addr, sizeof(addr));
	return res < 0 ? -1 : 0;
}

Poller::Poller() {}

void Poller::add(const IPollable &track) {
	struct pollfd fd;
	fd.fd = track.fd();
	fd.events = POLLIN;
	fd.revents = 0;
	fds.push(fd);
}

size_t Poller::poll(int timeout_ms) {
	return ::poll(fds.ptr(), (::nfds_t)fds.len(), timeout_ms);
}

template<typename T>
Array<uint8_t, sizeof(T)> to_le_bytes(T t) {
	Array<uint8_t, sizeof(T)> a;
	for (auto &x : a)
		x = (uint8_t)t, t >>= 8;
	return a;
}

template<typename T>
T from_le_bytes(const Array<uint8_t, sizeof(T)> &a) {
	T t(0);
	for (size_t i = sizeof(T); i > 0; )
		t <<= 8, --i, t |= (T)a[i];
	return t;
}

template Array<uint8_t, 1> to_le_bytes(int8_t);
template Array<uint8_t, 2> to_le_bytes(int16_t);
template Array<uint8_t, 4> to_le_bytes(int32_t);
template Array<uint8_t, 8> to_le_bytes(int64_t);
template Array<uint8_t, 1> to_le_bytes(uint8_t);
template Array<uint8_t, 2> to_le_bytes(uint16_t);
template Array<uint8_t, 4> to_le_bytes(uint32_t);
template Array<uint8_t, 8> to_le_bytes(uint64_t);

template int8_t from_le_bytes(const Array<uint8_t, 1> &);
template int16_t from_le_bytes(const Array<uint8_t, 2> &);
template int32_t from_le_bytes(const Array<uint8_t, 4> &);
template int64_t from_le_bytes(const Array<uint8_t, 8> &);
template uint8_t from_le_bytes(const Array<uint8_t, 1> &);
template uint16_t from_le_bytes(const Array<uint8_t, 2> &);
template uint32_t from_le_bytes(const Array<uint8_t, 4> &);
template uint64_t from_le_bytes(const Array<uint8_t, 8> &);

template<typename T>
T from_ne_bytes(const uint8_t *);

std::ostream &operator<<(std::ostream &out, const Ip4 &value) {
	auto f = [value](size_t i) { return (value.addr >> i) & 0xff; };
	return out << f(24) << "." << f(16) << "." << f(8) << "." << f(0);
}

std::ostream &operator<<(std::ostream &out, const Ip6 &value) {
	out << "[";
	for (size_t i = 0; i < sizeof(value.addr) / sizeof(*value.addr); i++) {
		if (i > 0)
			out << ":";
		out << std::hex << value.addr[i];
	}
	return out << "]";
}

template<>
std::ostream &operator<<(std::ostream &out, const SocketAddr<Ip4> &value) {
	return out << value.addr << ":" << value.port;
}

}
}
