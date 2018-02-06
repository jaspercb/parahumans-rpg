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

struct CollidableCreatedEvent {
	Entity entity;
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
	DamagedEvent(Entity source, Entity target, Damage damage)
		: source(source), target(target), damage(damage) {}
	Entity source, target;
	Damage damage;
};

struct HealedEvent {
	HealedEvent(Entity source, Entity target, float amount)
		: source(source), target(target), amount(amount) {}
	Entity source, target;
	float amount;
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

/* Control events represents an intent one step above keyboard input - move
   here, use this ability, use this item. The Input System translates keyboard
   events into these events.*/

struct ControlEvent {
	Entity entity;
};

struct Control_MoveAccelEvent : public ControlEvent {
	Control_MoveAccelEvent(Entity entity, vec2f accel) 
	: ControlEvent{entity}, accel(accel) {};
	vec2f accel;
};

struct Control_UseAbilityEvent : public ControlEvent {
	Control_UseAbilityEvent(Entity entity, vec2f target, Ability* ability)
	: ControlEvent{entity}, target(target), ability(ability) {}
	vec2f target;
	Ability* ability;
};
