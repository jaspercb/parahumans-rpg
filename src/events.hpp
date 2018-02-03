#pragma once

#include "types.hpp"

struct ControlSignal {
	enum Type {
		Move,
		Action
	};
	union {
		struct {
			float move_theta;
		};
		struct {
			int action_type;
		};
	};
};

struct MovedEvent {
	MovedEvent(Entity entity, vec2f oldPos, vec2f newPos) : entity(entity), oldPos(oldPos), newPos(newPos) {}
	Entity entity;
	vec2f oldPos, newPos;
};

struct CollidedEvent {
	CollidedEvent(Entity one, Entity two) : one(one), two(two) {}
	Entity one, two;
};

struct DamagedEvent {
	DamagedEvent(Entity source, Entity damaged, DamageType type, float damage)
		: source(source), damaged(damaged), type(type), damage(damage) {}
	Entity source, damaged;
	DamageType type;
	float damage;
};

struct WindowExitEvent {
	WindowExitEvent() {}
};

struct ConditionEvent {
	ConditionEvent(Condition condition, Entity source, Entity receiver)
		: condition(condition), source(source), receiver(receiver) {}
	Condition condition;
	Entity source, receiver;
};

struct ControlEvent {
	/* Represents an intent one step above keyboard input - move here, use this
	   ability, use this item. The Input System translates keyboard events into
	   these events.*/
	ControlEvent() {};
	enum Type {
		MoveAccel // Accelerate as hard as possible in a direction
	};
	union {
		struct {
			vec2f moveaccel_accel;
		};
		struct {
			int ability_id;
			vec2f ability_target;
		};
	};
};
