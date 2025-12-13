#pragma once

#include <new>
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

		// we can simplify things by creating a new List,
		// then moving objects from here to there.
		// Then all we need to do is move the new list over ourselves.

		// FIXME overflow check
		T *newbase = reinterpret_cast<T *>(::operator new (sizeof(T) * total, (std::align_val_t)alignof(T)));
		for (size_t i = 0; i < m_len; i++)
			new (&newbase[i]) T(std::move(base[i]));

		List<T> newlist;
		newlist.base = newbase;
		newlist.m_len = m_len;
		newlist.m_capacity = total;
	
		*this = std::move(newlist);
	}

	// Make copy constructor private to avoid accidental copying
	List(const List &) = delete;
	List &operator=(const List &) = delete;

public:
	List() : base(nullptr), m_len(0), m_capacity(0) {}
	List(size_t fill, T value) : List() {
		reserve(fill);
		for (size_t i = 0; i < fill; i++)
			push(value);
	}
	~List() noexcept {
		clear();
		::operator delete (base, sizeof(T) * m_capacity, (std::align_val_t)alignof(T));
	}
	List(List &&old) : base(old.base), m_len(old.m_len), m_capacity(old.m_capacity) {
		old.base = nullptr;
		old.m_len = 0;
		old.m_capacity = 0;
	}
	List &operator=(List &&old) {
		this->~List();
		base = old.base;
		m_len = old.m_len;
		m_capacity = old.m_capacity;
		old.base = nullptr;
		old.m_len = 0;
		old.m_capacity = 0;
		return *this;
	}

	void reserve(size_t additional) {
		size_t total = m_len + additional;
		if (total < additional)
			throw std::exception(); // TODO proper exception
		if (total > m_capacity)
			grow(total);
	}

	void push(const T &value) {
		reserve(1);
		new (&base[m_len++]) T(value);
	}

	void push(T &&value) {
		reserve(1);
		new (&base[m_len++]) T(std::move(value));
	}

	void append(const T *values, size_t num) {
		reserve(num);
		const T *end = values + num;
		while (values < end)
			push(*values++);
	}

	void append(const Slice<T> slice) {
		append(slice.ptr(), slice.len());
	}

	T pop() {
		if (m_len == 0)
			throw std::exception();
		T value = std::move(base[--m_len]);
		base[m_len].~T();
		return value;
	}

	void clear() {
		for (size_t i = 0; i < m_len; i++)
			base[i].~T();
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

	Slice<const T> slice(size_t from, size_t until) const {
		return ((Slice<const T>)*this).slice(from, until);
	}

	Slice<T> slice(size_t from, size_t until) {
		return ((Slice<T>)*this).slice(from, until);
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
