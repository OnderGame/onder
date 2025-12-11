#pragma once

#include <iostream>

namespace onder {
namespace math {

template<typename T>
struct Vec2 {
	T x, y;

	Vec2() : x({}), y({}) {}
	Vec2(T x, T y) : x(x), y(y) {}

	static Vec2<T> splat(T value) {
		return { value, value };
	}

	Vec2<T> operator+(const Vec2<T> &rhs) const {
		return { x + rhs.x, y + rhs.y };
	}
	Vec2<T> operator-(const Vec2<T> &rhs) const {
		return { x - rhs.x, y - rhs.y };
	}
	Vec2<T> operator*(const T &v) const {
		return { x * v, y * v };
	}
	Vec2<T> operator/(const T &v) const {
		return { x / v, y / v };
	}

	Vec2<T> min(const Vec2<T> &rhs) const {
		return { std::min(x, rhs.x), std::min(y, rhs.y) };
	}
	Vec2<T> max(const Vec2<T> &rhs) const {
		return { std::max(x, rhs.x), std::max(y, rhs.y) };
	}

	T element_sum() const {
		return x + y;
	}
	T element_product() const {
		return x * y;
	}

	template<typename U>
	operator Vec2<U>() {
		return { (U)x, (U)y };
	}
};

template<typename T>
std::ostream &operator<<(std::ostream &out, const Vec2<T> &v) {
	return out << "{" << v.x << "," << v.y << "}";
}

}
}
