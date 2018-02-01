#pragma once

#include <unordered_set>

#include "sdlTools.hpp"
#include "types.hpp"

struct SpatialData {
	SpatialData(vec2f position={0, 0}, vec2f velocity={0, 0}, float z=0, float orientation=0) :
		position(position),
		velocity(velocity),
		z(z),
		orientation(orientation),
		timeMoving(0.0) {};

	vec2f orientationToVec() const {
		return vec2f(cos(orientation), sin(orientation));
	};
	bool isMoving() const {
		return velocity.x || velocity.y;
	}

	vec2f position, velocity;
	float z, orientation;
	TimeDelta timeMoving;
};

struct Renderable {
	enum Type {
		Circle,
		Line,
		Person,
		Cube
	};

	Renderable(Type type=Circle, SDL_Color color=SDL_Colors::RED, float radius=10, int x=0, int y=0) :
		type(type),
		color(color),
		line_thickness(radius),
		line_displacement_x(x),
		line_displacement_y(y)
	{}

	Type type;
	SDL_Color color;
	union {
		// Circle
		struct {
			float circle_radius;
		};
		// Line
		struct {
			float line_thickness;
			float line_displacement_x;
			float line_displacement_y;
		};
		struct {
			float cube_x, cube_y, cube_z;
		};
	};
};

class ConditionSystem;

class Stats {
friend ConditionSystem;
private:
	struct {
		// float brawn, athletics, dexterity, wits, social, knowledge, guts;
		float speed, accel;
	} basestats, stats;
	bool dirty;
public:
	Stats(float speed, float accel) {
		for (auto i : {&basestats, &stats}) {
			i->speed = speed;
			i->accel = accel;
		}
		dirty = false;
	}
	float speed() const { return stats.speed; }
	float accel() const { return stats.accel; }
};

struct Conditions {
	std::list<Condition> list;
};

struct Collidable {
	enum Type {
		Circle
	};
	Type type;
	union {
		struct {
			float circle_radius;
		};
	};
	std::unordered_set<Entity> ignored;

	bool canCollide(Entity other) const {
		return ignored.find(other) != ignored.end();
	}
};

struct Destructible {
	Destructible(HPType maxHP, bool indestructible=false)
		: HP(maxHP, 0, maxHP), indestructible(indestructible)
		{}
	BoundedQuantity<HPType> HP;
	bool indestructible;
};

struct Controllable {
	Controllable() {}
};

struct OnCollision {
	/* 
	Stores behavioral data about what to do when we encounter a collision.
	*/
	OnCollision();
	DamageType damagetype;
	float damage;
	std::vector<Condition> conditions;
};
