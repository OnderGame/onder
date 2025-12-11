#pragma once

#include <iostream>
#include <onder/math/vec2.hpp>

namespace onder {
namespace math {

template<typename T>
class Rect {
	Vec2<T> lo, hi;

public:
	Rect() {}
	Rect(Vec2<T> a, Vec2<T> b) : lo(a.min(b)), hi(a.max(b)) {}

	static Rect from_pos_size(Vec2<T> pos, Vec2<T> size) {
		return { pos, pos + size };
	}

	const Vec2<T> &low() const {
		return lo;
	}

	const Vec2<T> &high() const {
		return hi;
	}

	const Vec2<T> dim() {
		return hi - lo;
	}

	T area() const {
		return (hi - lo).element_product();
	}

	template<typename U>
	operator Rect<U>() {
		return { lo, hi };
	}

	Rect<T> operator&(const Rect<T> &other) {
		auto l = lo.max(other.lo), h = hi.min(other.hi);
		return { l.min(h), h };
	}
	Rect<T> &operator&=(const Rect<T> &other) {
		return *this = *this & other;
	}

	Rect<T> operator-(const Vec2<T> &translation) {
		return { lo - translation, hi - translation };
	}
	Rect<T> operator+(const Vec2<T> &translation) {
		return { lo + translation, hi + translation };
	}
};

template<typename T>
std::ostream &operator<<(std::ostream &out, const Rect<T> &v) {
	return out << "{" << v.low() << "," << v.high() << "}";
}

}
}
