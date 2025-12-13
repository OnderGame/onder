#pragma once

#include <cstdint>
#include <onder/collections/slice.hpp>

namespace onder {
namespace collections {

// TODO multi-dimensional arrays (figure out recursive templates)
template<typename T, size_t N>
class Array {
	T values[N];

	void check_bounds(size_t index) const {
		if (index >= N)
			throw std::exception(); // TODO
	}

public:
	using iterator = typename Slice<T>::iterator;

	Array() : values() {}

	const T &operator[](size_t index) const {
		check_bounds(index);
		return values[index];
	}

	T &operator[](size_t index) {
		check_bounds(index);
		return values[index];
	}

	operator Slice<T>() const {
		return { (T *)values, N };
	}

	iterator begin() const {
		return ((Slice<T>)*this).begin();
	}

	iterator end() const {
		return ((Slice<T>)*this).end();
	}
};

}
}
