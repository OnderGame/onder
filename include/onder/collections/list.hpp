#pragma once

#include <numeric>
#include <onder/collections/slice.hpp>

namespace onder {
namespace collections {

template<typename T>
class List {
	T *base;
	size_t m_len, m_capacity;

	void grow(size_t min_cap) {
		// FIXME overflow checking
		size_t total = std::max(m_capacity * 3 / 2, min_cap);
		// Do *not* use realloc, as C++ cannot deal with suddenly moving objects.
		T *newbase = new T[total];
		for (size_t i = 0; i < m_len; i++)
			newbase[i] = base[i];
		delete[] base;
		base = newbase;
		m_capacity = total;
	}

public:
	List() : base(nullptr), m_len(0), m_capacity(0) {}
	List(size_t fill, T value) : List() {
		reserve(fill);
		for (size_t i = 0; i < fill; i++)
			push(value);
	}

	void reserve(size_t additional) {
		size_t total = m_len + additional;
		if (total < additional)
			throw std::exception(); // TODO proper exception
		if (total > m_capacity)
			grow(total);
	}

	void push(T value) {
		reserve(1);
		base[m_len] = value;
		++m_len;
	}

	T pop() {
		if (m_len == 0)
			throw std::exception();
		return base[--m_len];
	}

	void clear() {
		m_len = 0;
	}

	const T *ptr() const {
		return base;
	}

	T *ptr() {
		return base;
	}

	size_t len() const {
		return m_len;
	}

	void set_len(size_t newlen) {
		m_len = newlen;
	}

	size_t capacity() const {
		return m_capacity;
	}

	operator Slice<const T>() const {
		return { base, m_len };
	}

	operator Slice<T>() {
		return { base, m_len };
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
