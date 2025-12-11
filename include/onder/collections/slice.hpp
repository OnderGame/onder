#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>

namespace onder {
namespace collections {

template<typename T>
class Slice {
	T *base;
	size_t m_len;

	void check_bounds(size_t index) const {
		if (index >= m_len)
			throw std::exception(); // TODO
	}

public:
	Slice() : base(nullptr), m_len(0) {}
	Slice(T *base, size_t len) : base(base), m_len(len) {}

	const T &operator[](size_t index) const {
		check_bounds(index);
		return base[index];
	}

	T &operator[](size_t index) {
		check_bounds(index);
		return base[index];
	}

	const T *ptr() const {
		return base;
	}

	size_t len() const {
		return m_len;
	}
};

}
}
