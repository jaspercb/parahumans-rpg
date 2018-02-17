#pragma once

#include <unordered_set>
#include <list>
#include <vector>
#include <array>

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

/*
// TODO: Stat refactor. Some things are immune to some stats being modified - how to represent?
// Could break up into stats that everything has vs stats that only people have
// Could have a Stat monoclass with various flags
class InorganicStats {
	// stuff like resistance to damage, time-locked ness
};

class OrganicStats {
	// stuff like 
};
*/

class ConditionSystem;

Stat StatVulnerabilityTo(Damage::Type damagetype);

class Stats {
friend ConditionSystem;
private:
	std::array<float, Stat::SIZE> basestats, stats;
	bool dirty;
public:
	Stats(float speed, float accel) {
		basestats[Stat::SPEED]     = speed;
		basestats[Stat::ACCEL]     = accel;
		basestats[Stat::SUBJECTIVE_TIME_RATE] = 1.0;
		for (int damageInt = Damage::Type::Puncture;
		     damageInt != Damage::Type::SIZE; ++damageInt) {
			basestats[StatVulnerabilityTo(static_cast<Damage::Type>(damageInt))] = 1.0;
		}
		stats = basestats;
		dirty = false;
	}
	float& operator[](Stat stat) { return stats[stat]; }
	float  operator[](Stat stat) const { return stats[stat]; }
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
	 // How many collisions until this entity should be destroyed. -1 means "never delete"
	int collisionsUntilDestroyed;
	TimeDelta timeUntilCollidable;
	// Whether we want to add an entity to our ignore list after colliding
	bool ignoreRepeatCollisions;
	std::unordered_set<Entity> ignored;

	Collidable(Type type, float arg1, int collisionsUntilDestroyed=-1, TimeDelta timeUntilCollidable=0.0)
		: type(type), circle_radius(arg1), collisionsUntilDestroyed(collisionsUntilDestroyed), timeUntilCollidable(timeUntilCollidable) {}

	bool canCollide(Entity other) const {
		return (timeUntilCollidable <= 0.0
		     && collisionsUntilDestroyed != 0
		     && ignored.find(other) == ignored.end());
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
	std::vector<std::shared_ptr<Ability>> abilities;
};

struct OnCollision {
	/* 
	A component storing a list of callbacks, saying what to do upon colliding.
	*/
	std::list<std::shared_ptr<OnCollisionCallback>> callbacks;
};

struct CameraFocus {};

struct TimeOut {
	TimeDelta timeLeft;
	bool isExpired() const {
		return timeLeft <= 0;
	}
};
