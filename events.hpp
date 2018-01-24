#pragma once

#include "entityx/entityx.h"

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
	MovedEvent(entityx::Entity, vec2f oldPos, vec2f newPos) : entity(entity), oldPos(oldPos), newPos(newPos) {}
	entityx::Entity entity;
	vec2f oldPos, newPos;
};

struct CollidedEvent {
	CollidedEvent(entityx::Entity one, entityx::Entity two) : one(one), two(two) {}
	entityx::Entity one, two;
};
