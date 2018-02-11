#include "World.hpp"

void InstantSingleCondition::operator()(World* world, Entity e) {
	world->bus.publish<ConditionEvent>(mCondition, e, e);
}

void InstantSingleDamage::operator()(World* world, Entity e) {
	world->bus.publish<DamagedEvent>(e /* TODO: should be owner */, e, mDamage);
}
