#pragma once

#include <unordered_set>
#include <list>
#include <vector>

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
		return {cos(orientation), sin(orientation)};
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

struct Conditions : public std::list<Condition> {};

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
	int collisionsUntilDestroyed; // -1 means "never delete this"
	std::unordered_set<Entity> ignored;

	Collidable(Type type, float arg1, int collisionsUntilDestroyed=-1)
		: type(type), circle_radius(arg1), collisionsUntilDestroyed(collisionsUntilDestroyed) {}

	bool canCollide(Entity other) const {
		return collisionsUntilDestroyed != 0 && ignored.find(other) == ignored.end();
	}

	void addIgnored(Entity other) {
		ignored.insert(other);
	}
};

struct Destructible {
	Destructible(HPType maxHP, bool indestructible=false, bool healable=true)
		: HP(maxHP, 0, maxHP), indestructible(indestructible), healable(healable)
		{}
	BoundedQuantity<HPType> HP;
	bool indestructible, healable;
};

struct Controllable {
	Controllable() {}
};

struct AbilityData {
	std::vector<Ability> abilities;
};

struct OnCollision {
	/* 
	A component storing behavioral data about what to do upon colliding.
	*/
	OnCollision() {};
	Damage damage;
	std::vector<Condition> conditions;
};

struct CameraFocus {};

struct TimeOut {
	TimeDelta timeLeft;
	bool isExpired() const {
		return timeLeft <= 0;
	}
};
