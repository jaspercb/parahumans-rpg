#include "World.hpp"

void InstantSingleCondition::__apply(World* world, Entity e) {
	world->bus.publish<ConditionEvent>(mCondition, e, e);
}

void InstantSingleDamage::__apply(World* world, Entity e) {
	world->bus.publish<DamagedEvent>(mData.owner, e, mDamage);
}

void InstantAreaExplosion::operator()(World* world, vec2f position) {
	Entity explosion = world->registry.create();
	auto &sdata       = world->registry.assign<SpatialData>(explosion, position);
	auto &collidable  = world->registry.assign<Collidable>(explosion, CircleCollidable(mExplosionRadius));
	collidable.ignoreRepeatCollisions = true;
	collidable.ignored = mData.ignored;
	auto &timeout     = world->registry.assign<TimeOut>(explosion, 0.1);
	auto &oncollision = world->registry.assign<OnCollision>(explosion);
	oncollision.callbacks.push_back(mInstantSingleEffect);
	auto &renderable  = world->registry.assign<Renderable>(explosion, Renderable::Type::Circle);
	renderable.circle_radius = mExplosionRadius;
}
