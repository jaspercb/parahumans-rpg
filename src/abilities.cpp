#include "abilities.hpp"
#include "World.hpp"

void BasicProjectileAbility::onKeyDown(vec2f target) {
	// fire in direction of enemy
	auto &sdata = mWorld->registry.get<SpatialData>(mOwner);
	makeProjectile(sdata.position, target);
}

Entity BasicProjectileAbility::makeProjectile(vec2f position, vec2f target) {
	auto velocity = target - position;
	velocity.truncate(mProjectileSpeed);
	Entity projectile = mWorld->registry.create();
	mWorld->registry.assign<SpatialData>(projectile, position, velocity, 20 /* z */);
	auto &renderable = mWorld->registry.assign<Renderable>(projectile);
	renderable.circle_radius = mProjectileRadius;
	auto &collidable = mWorld->registry.assign<Collidable>(projectile, CircleCollidable(mProjectileRadius, 1 /* collisions until destroyed */));
	collidable.addIgnored(mOwner);
	auto &oncollision = mWorld->registry.assign<OnCollision>(projectile);
	oncollision.callbacks.push_back(mInstantSingleEffect);
	return projectile;
}

void HeartseekerAbility::onKeyDown(vec2f target) {
	auto &sdata = mWorld->registry.get<SpatialData>(mOwner);
	mActiveProjectile = makeProjectile(sdata.position, target);
	assert(mActiveProjectile);
}

void HeartseekerAbility::onKeyUp(vec2f target) {
	if (mActiveProjectile && mWorld->registry.valid(mActiveProjectile.value())) {
		auto &sdata = mWorld->registry.get<SpatialData>(mActiveProjectile.value());
		vec2f newVelocity = target - sdata.position;
		newVelocity.truncate(sdata.velocity.length());
		sdata.velocity = newVelocity;
		mActiveProjectile.reset();
	}
}

void SelfInstantEffectAbility::onKeyDown(vec2f target) {
	(*mInstantSingleEffect)(mWorld, mOwner);
}

void AreaEffectTargetAbility::onKeyDown(vec2f target) {
	(*mInstantAreaEffect)(mWorld, target);
}

void PuckAbility::onKeyDown(vec2f target) {
	auto &sdata = mWorld->registry.get<SpatialData>(mOwner);
	if (mProjectile && mWorld->registry.valid(mProjectile.value())) {
		// swap location
		auto &projectileSdata = mWorld->registry.get<SpatialData>(mProjectile.value());
		std::swap(sdata.position, projectileSdata.position);
		mWorld->bus.publish<MovedEvent>(mOwner, projectileSdata.position, sdata.position);
		// make explosion
		(*mInstantAreaEffect)(mWorld, sdata.position);
		(*mInstantAreaEffect)(mWorld, projectileSdata.position);
		mWorld->destroy(mProjectile.value());
		mProjectile.reset();
	} else {
		// fire in direction of enemy
		auto velocity = target - sdata.position;
		velocity.truncate(mProjectileSpeed);
		Entity projectile = mWorld->registry.create();
		mWorld->registry.assign<SpatialData>(projectile, sdata.position, velocity, 20 /* z */);
		auto &renderable = mWorld->registry.assign<Renderable>(projectile);
		renderable.circle_radius = 15;
		mWorld->registry.assign<TimeOut>(projectile, mProjectileDuration);
		mProjectile = projectile;
	}
}

void FlashAbility::onKeyDown(vec2f target) {
	auto &sdata = mWorld->registry.get<SpatialData>(mOwner);
	vec2f offset = target-sdata.position;
	if (offset.length() > mRange) {
		offset.truncate(mRange);
	}
	sdata.position += offset;
}

std::shared_ptr<Ability> TestProjectileAbility(World* world, Entity owner) {
	return std::make_shared<BasicProjectileAbility>(world, owner, std::make_shared<InstantSingleDamage>(Damage{Damage::Type::Impact, 10}), 500, 10);
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

std::shared_ptr<Ability> TestPuckAbility(World* world, Entity owner) {
	return std::make_shared<PuckAbility>(
		world, owner,
		std::make_shared<InstantAreaExplosion>(
			std::make_shared<InstantSingleCondition>(
				Condition{Condition::Type::BURN, 3.0, 3.0 /* seconds */}),
			150),
		400, 2
		);
}

std::shared_ptr<Ability> TestHeartseekerAbility(World* world, Entity owner) {
	return std::make_shared<HeartseekerAbility>(world, owner, std::make_shared<InstantSingleDamage>(Damage{Damage::Type::Impact, 10}), 700, 5);
}
