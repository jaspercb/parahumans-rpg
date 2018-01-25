#pragma once

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
	MovedEvent(Entity, vec2f oldPos, vec2f newPos) : entity(entity), oldPos(oldPos), newPos(newPos) {}
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
