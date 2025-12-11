#pragma once

#include <numeric>
#include <onder/collections/slice.hpp>

namespace onder {
namespace collections {

template<typename T>
class List {
	T *base;
	size_t len, capacity;

	void grow(size_t min_cap) {
		// FIXME overflow checking
		size_t total = std::max(capacity * 3 / 2, min_cap);
		// Do *not* use realloc, as C++ cannot deal with suddenly moving objects.
		T *newbase = new T[total];
		for (size_t i = 0; i < len; i++)
			newbase[i] = base[i];
		delete[] base;
		base = newbase;
		capacity = total;
	}

public:
	List() : base(nullptr), len(0), capacity(0) {}
	List(size_t fill, T value) : List() {
		reserve(fill);
		for (size_t i = 0; i < fill; i++)
			push(value);
	}

	void reserve(size_t additional) {
		size_t total = len + additional;
		if (total < additional)
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

	void clear() {
		len = 0;
	}

	operator Slice<const T>() const {
		return { base, len };
	}

	operator Slice<T>() {
		return { base, len };
	}

	const T &operator[](size_t index) const {
		return ((Slice<const T>)*this)[index];
	}

	T &operator[](size_t index) {
		return ((Slice<T>)*this)[index];
	}
};

}
}
