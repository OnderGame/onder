#pragma once

#include <onder/collections/array.hpp>
#include <onder/collections/list.hpp>
#include <poll.h>
#include <iostream>

namespace onder {
namespace net {

struct Ip4 {
	uint32_t addr;

	Ip4() : addr(0) {}
	Ip4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
		: addr(((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d)
	{}
};

struct Ip6 {
	uint16_t addr[8];

	Ip6() : addr({}) {}
};

template<typename T>
struct SocketAddr {
	T addr;
	uint16_t port;

	SocketAddr() : port() {}
	SocketAddr(T addr, uint16_t port) : addr(addr), port(port) {}
};

class IPollable {
public:
	virtual int fd() const = 0;
};

template<typename A>
class Udp : public IPollable {
	int m_fd;

	Udp(const Udp<A> &) = delete;
	Udp<A> &operator=(const Udp<A> &) = delete;

public:
	Udp();
	Udp(const SocketAddr<A> &addr);
	~Udp();

	int recv(collections::List<uint8_t> &buffer, SocketAddr<A> &addr);
	int send(collections::Slice<const uint8_t> data, const SocketAddr<A> &addr);

	int fd() const override;
};

class Poller {
	collections::List<struct pollfd> fds;

public:
	Poller();
	void add(const IPollable &track);
	size_t poll(int timeout_ms);
};

// Why a template and not overloads?
//
// The main idea is to avoid unexpected implicit casting and cause
// a linker error if an unsupported type is passed (e.g. float).
template<typename T>
collections::Array<uint8_t, sizeof(T)> to_le_bytes(T);

template<typename T>
T from_le_bytes(const collections::Array<uint8_t, sizeof(T)>);

std::ostream &operator<<(std::ostream &out, const Ip4 &value);
std::ostream &operator<<(std::ostream &out, const Ip6 &value);
template<typename A>
std::ostream &operator<<(std::ostream &out, const SocketAddr<A> &value);

}
}
