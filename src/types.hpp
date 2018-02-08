#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <vector>

typedef float TimeDelta;
typedef float HPType;
typedef std::uint32_t Entity;

template <class T>
class vec2 {
public:
	T x, y;

	vec2 operator+(const vec2& v) const {
		return {x + v.x, y + v.y};
	}
	vec2 operator-(const vec2& v) const {
		return {x - v.x, y - v.y};
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
	vec2 operator+(const double s) const {
		return {x + s, y + s};
	}
	vec2 operator-(const double s) const {
		return {x - s, y - s};
	}
	vec2 operator*(const double s) const {
		return {x * s, y * s};
	}
	vec2 operator/(const double s) const {
		return {x / s, y / s};
	}
	vec2& operator+=(const double s) {
		x += s;
		y += s;
		return *this;
	}
	vec2& operator-=(const double s) {
		x -= s;
		y -= s;
		return *this;
	}
	vec2& operator*=(const double s) {
		x *= s;
		y *= s;
		return *this;
	}
	vec2& operator/=(const double s) {
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
		const double c = cos(rad);
		const double s = sin(rad);
		const double tx = x * c - y * s;
		const double ty = x * s + y * c;
		x = tx;
		y = ty;
	}
	vec2& normalize() {
		if (length() == 0) return *this;
		*this *= (1.0 / length());
		return *this;
	}
	float dist(vec2 v) const {
		vec2 d= {v.x - x, v.y - y};
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
		return {y, -x};
	}
	static float dot(vec2 v1, vec2 v2) {
		return v1.x * v2.x + v1.y * v2.y;
	}
	static float cross(vec2 v1, vec2 v2) {
		return (v1.x * v2.y) - (v1.y * v2.x);
	}
	template<typename P> operator vec2<P>() const {
		return {P(x), P(y)};
	}
};


template<typename T> vec2<T> operator*(const double s, const vec2<T> vec) {
	return {vec.x * s, vec.y * s};
}

typedef vec2<int> vec2i;
typedef vec2<float> vec2f;
typedef vec2<double> vec2d;

template<typename T> class BoundedQuantity {
public:
	BoundedQuantity(T val, T min, T max)
		: value(val), min(min), max(max)
		{}
	BoundedQuantity& operator+=(const T& other) {
		value = std::clamp(value + other, min, max);
		return *this;
	}
	BoundedQuantity& operator-=(const T& other) {
		value = std::clamp(value - other, min, max);
		return *this;
	}
	BoundedQuantity& operator=(const T& other) {
		value = std::clamp(other, min, max);
		return *this;
	}
	operator T() const {
		return value;
	}
	T value, min, max;
};

struct Damage {
	enum Type {
		Puncture,
		Slash,
		Impact,
		Heat,
		Cold,
		Electricity,
		Toxin
	};

	Type type;
	float amount;
};

struct Condition {
	enum Priority {
		Adder      = 0,
		Multiplier = 1,
		None       = 2
	};
	enum Type {
		BURN,
		// FREEZE, // TODO
		BLEED,
		POISON,
		// STUN,   // TODO
		REGEN,
		MOD_SPEED,
		MOD_ACCEL
	};

	Priority priority;
	Type type;
	/* Condition strength means different things for different conditions.
	   BURN/BLEED/POISON:    damage in HP/second
	   REGEN:                heal in HP/second
	   MOD_SPEED, MOD_ACCEL: amount to add/multiply by based on priority */
	float strength;
	TimeDelta timeLeft;
	// Contagious?

	bool isExpired() const {
		return timeLeft <= 0;
	}
	bool isBeneficial() const {
		switch(type) {
			case Type::BURN:
			// case Type::FREEZE:
			case Type::BLEED:
			case Type::POISON:
			return false;
			case Type::REGEN:
			return true;
			case Type::MOD_SPEED:
			case Type::MOD_ACCEL:
			return (priority == Priority::Multiplier) ? strength > 1 : strength > 0;
		}
	}
	bool operator<(const Condition& other) const {return priority < other.priority;}
};

class Collidable;
struct OnCollision;
struct Renderable;
struct TimeOut;

struct ProjectileTemplate {
	float projectile_speed;
	Collidable* collidable;
	OnCollision* oncollision;
	Renderable* renderable;
	TimeOut* timeout;
};

struct Ability {
	enum Type {
		FireProjectile,
		// ExplosionOnTarget, // TODO
		// TeleportToTarget,  // TODO
		SelfCondition
	};
	Type type;
	union {
		ProjectileTemplate projectile_template;
		Condition condition;
	};
	float timeSinceUsed;
	float cooldown;

	bool isOffCooldown() const { return timeSinceUsed >= cooldown; }
};

// TODO: Damage utility struct, bundling type and amount