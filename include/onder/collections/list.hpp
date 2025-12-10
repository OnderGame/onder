#pragma once

#include <numeric>
#include <stdckdint.h>

namespace onder {
namespace collections {

template<T>
class List {
	T *base;
	size_t len, capacity;

	void grow(size_t min_cap) {
		size_t total = std::max(std::mul_sat(capacity, 2), min_cap);
		// Do *not* use realloc, as C++ cannot deal with suddenly moving objects.
		T *newbase = new T[total];
		for (size_t i = 0; i < len; i++)
			newbase[i] = base[i];
		delete[] base;
		base = newbase;
		capacity = total;
	}

	void check_bounds(size_t index) const {
		if (index >= len)
			throw std::exception(); // TODO
	}

public:
	List() : base(nullptr), len(0), capacity(0) {}

	void reserve(size_t additional) {
		// TODO overflow check
		size_t total;
		if (chk_add(&total, len, capacity))
			throw std::exception(); // TODO proper exception
		if (total > capacity)
			grow(total);
	}

	void push(T value) {
		reserve(1);
		base[len] = value;
		++len;
	}

	T pop() {
		if (len == 0)
			throw std::exception();
		return base[--len];
	}

	const T &operator[](size_t index) const {
		check_bounds(index);
		return base[index];
	}

	T &operator[](size_t index) {
		check_bounds(index);
		return base[index];
	}
};

}
}
