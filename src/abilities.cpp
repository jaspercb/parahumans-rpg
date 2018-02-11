#include "abilities.hpp"
#include "World.hpp"

void BasicProjectileAbility::onKeyDown(vec2f target) {
	// fire in direction of enemy
	auto &sdata = mWorld->registry.get<SpatialData>(mOwner);
	auto velocity = target - sdata.position;
	velocity.truncate(mProjectileSpeed);
	Entity projectile = mWorld->registry.create();
	mWorld->registry.assign<SpatialData>(projectile, sdata.position, velocity, 20 /* z */);
	mWorld->registry.assign<Renderable>(projectile);
	auto &collidable = mWorld->registry.assign<Collidable>(projectile, Collidable::Circle, mProjectileRadius, 1 /* collisions until destroyed */);
	collidable.addIgnored(mOwner);
	auto &oncollision = mWorld->registry.assign<OnCollision>(projectile);
	oncollision.callbacks.push_back(mInstantSingleEffect);
}

void SelfInstantEffectAbility::onKeyDown(vec2f target) {
	(*mInstantSingleEffect)(mWorld, mOwner);
}

void AreaEffectTargetAbility::onKeyDown(vec2f target) {
	(*mInstantAreaEffect)(mWorld, target);
}

std::shared_ptr<Ability> TestProjectileAbility(World* world, Entity owner) {
	return std::make_shared<BasicProjectileAbility>(world, owner, std::make_shared<InstantSingleDamage>(Damage{Damage::Type::Impact, 10}), 500, 50);
}

std::shared_ptr<Ability> TestBuffAbility(World* world, Entity owner, Condition condition) {
	return std::make_shared<SelfInstantEffectAbility>(world, owner, std::make_shared<InstantSingleCondition>(condition));
}

std::shared_ptr<Ability> TestAreaEffectTargetAbility(World* world, Entity owner) {
	return std::make_shared<AreaEffectTargetAbility>(
		world, owner,
		std::make_shared<InstantAreaExplosion>(
			std::make_shared<InstantSingleDamage>(
				Damage{Damage::Type::Impact, 10}),
			250)
		);
}
