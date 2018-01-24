#pragma once

#include <cmath>
#include "entt/entt.hpp"

template <class T>
class vec2 {
public:
	T x, y;
	
	vec2() :x(0), y(0) {}
	vec2(T x, T y) : x(x), y(y) {}
	vec2(const vec2& v) : x(v.x), y(v.y) {}
	
	vec2& operator=(const vec2& v) {
		x = v.x;
		y = v.y;
		return *this;
	}
	
	vec2 operator+(const vec2& v) {
		return vec2(x + v.x, y + v.y);
	}
	vec2 operator-(const vec2& v) {
		return vec2(x - v.x, y - v.y);
	}
	
	vec2& operator+=(const vec2& v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	vec2& operator-=(const vec2& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	
	vec2 operator+(double s) {
		return vec2(x + s, y + s);
	}
	vec2 operator-(double s) {
		return vec2(x - s, y - s);
	}
	vec2 operator*(double s) {
		return vec2(x * s, y * s);
	}
	vec2 operator/(double s) {
		return vec2(x / s, y / s);
	}
	
	
	vec2& operator+=(double s) {
		x += s;
		y += s;
		return *this;
	}
	vec2& operator-=(double s) {
		x -= s;
		y -= s;
		return *this;
	}
	vec2& operator*=(double s) {
		x *= s;
		y *= s;
		return *this;
	}
	vec2& operator/=(double s) {
		x /= s;
		y /= s;
		return *this;
	}

	bool operator<(const vec2& other) const {
		if (x == other.x) return y < other.y;
		return x < other.x;
	}

	bool operator==(const vec2& other) const {
		return (x == other.x) && (y == other.y);
	}
	
	bool operator!=(const vec2& other) const {
		return !(*this == other);
	}
	
	void set(T x, T y) {
		this->x = x;
		this->y = y;
	}
	
	void rotate(double rad) {
		double c = cos(rad);
		double s = sin(rad);
		double tx = x * c - y * s;
		double ty = x * s + y * c;
		x = tx;
		y = ty;
	}
	
	vec2& normalize() {
		if (length() == 0) return *this;
		*this *= (1.0 / length());
		return *this;
	}
	
	float dist(vec2 v) const {
		vec2 d(v.x - x, v.y - y);
		return d.length();
	}
	float length() const {
		return std::sqrt(x * x + y * y);
	}
	void truncate(double length) {
		double angle = atan2f(y, x);
		x = length * cos(angle);
		y = length * sin(angle);
	}
	
	vec2 ortho() const {
		return vec2(y, -x);
	}
	
	static float dot(vec2 v1, vec2 v2) {
		return v1.x * v2.x + v1.y * v2.y;
	}
	static float cross(vec2 v1, vec2 v2) {
		return (v1.x * v2.y) - (v1.y * v2.x);
	}
	
};

typedef vec2<int> vec2i;
typedef vec2<float> vec2f;
typedef vec2<double> vec2d;
typedef float TimeDelta;
typedef std::uint32_t Entity;
