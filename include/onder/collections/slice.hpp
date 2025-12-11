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
	// https://stackoverflow.com/a/38103394
	class iterator {
		T *cur;

		friend class Slice;

		iterator(T *cur) : cur(cur) {}

	public:
		iterator& operator++() {
			++cur;
			return *this;
		}
        iterator operator++(int) {
			iterator ret = *this;
			++(*this);
			return ret;
		}
        bool operator==(iterator other) const {
			return cur == other.cur;
		}
        bool operator!=(iterator other) const {
			return !(*this == other);
		}
		T &operator*() {
			return *cur;
		}
        // iterator traits
        using value_type = T;
        using pointer = T *;
        using reference = T &;
	};

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

	iterator begin() const {
		return { base };
	}

	iterator end() const {
		return { base + m_len };
	}
};

}
}
