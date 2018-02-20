#include <stdlib.h>

#include "abilities.hpp"
#include "World.hpp"

void BasicProjectileAbility::onKeyDown(vec2f target) {
	// fire in direction of enemy
	auto &sdata = mData.world->registry.get<SpatialData>(mData.owner);
	makeProjectile(sdata.position, target);
}

Entity BasicProjectileAbility::makeProjectile(vec2f position, vec2f target) {
	auto velocity = target - position;
	velocity.truncate(mProjectileSpeed);
	Entity projectile = mData.world->registry.create();
	mData.world->registry.assign<SpatialData>(projectile, position, velocity, 20 /* z */);
	auto &renderable = mData.world->registry.assign<Renderable>(projectile);
	renderable.circle_radius = mProjectileRadius;
	auto &collidable = mData.world->registry.assign<Collidable>(projectile, CircleCollidable(mProjectileRadius, 1 /* collisions until destroyed */));
	collidable.ignored = mData.ignored;
	auto &oncollision = mData.world->registry.assign<OnCollision>(projectile);
	for (auto &effect : mInstantSingleEffects)
		oncollision.callbacks.push_back(effect);
	return projectile;
}

void SprayAbility::onKeyDown(vec2f target) {
	firing = true;
	mData.timeSinceUsed = 0.0;
}

void SprayAbility::onKeyUp(vec2f target) {
	firing = false;
}

void SprayAbility::onMouseMove(vec2f target) {
	focus = target;
}

void SprayAbility::update(TimeDelta dt) {
	if (firing) {
		auto &sdata = mData.world->registry.get<SpatialData>(mData.owner);
		auto direction = focus - sdata.position;
		float angle = (rand() % 10000 - 5000.0)/10000;
		direction.rotate(angle);
		makeProjectile(sdata.position, sdata.position + direction);
	} else {
		mData.timeSinceUsed += dt;
	}
}

void HeartseekerAbility::onKeyDown(vec2f target) {
	auto &sdata = mData.world->registry.get<SpatialData>(mData.owner);
	mActiveProjectile = makeProjectile(sdata.position, target);
}

void HeartseekerAbility::onKeyUp(vec2f target) {
	if (mActiveProjectile && mData.world->registry.valid(mActiveProjectile.value())) {
		auto &sdata = mData.world->registry.get<SpatialData>(mActiveProjectile.value());
		vec2f newVelocity = target - sdata.position;
		newVelocity.truncate(sdata.velocity.length());
		sdata.velocity = newVelocity;
		mActiveProjectile.reset();
	}
}

void SelfInstantEffectAbility::onKeyDown(vec2f target) {
	for (auto &effect : mInstantSingleEffects)
		(*effect)(mData.world, mData.owner);
}

void AreaEffectTargetAbility::onKeyDown(vec2f target) {
	for (auto &effect : mInstantAreaEffects)
		(*effect)(mData.world, target);
}

void PuckAbility::onKeyDown(vec2f target) {
	auto &sdata = mData.world->registry.get<SpatialData>(mData.owner);
	if (mProjectile && mData.world->registry.valid(mProjectile.value())) {
		// swap location
		auto &projectileSdata = mData.world->registry.get<SpatialData>(mProjectile.value());
		std::swap(sdata.position, projectileSdata.position);
		mData.world->bus.publish<MovedEvent>(mData.owner, projectileSdata.position, sdata.position);
		// make explosion
		for (auto &effect : mInstantAreaEffects) {
			(*effect)(mData.world, sdata.position);
			(*effect)(mData.world, projectileSdata.position);
		}
		mData.world->destroy(mProjectile.value());
		mProjectile.reset();
	} else {
		// fire in direction of enemy
		auto velocity = target - sdata.position;
		velocity.truncate(mProjectileSpeed);
		Entity projectile = mData.world->registry.create();
		mData.world->registry.assign<SpatialData>(projectile, sdata.position, velocity, 20 /* z */);
		auto &renderable = mData.world->registry.assign<Renderable>(projectile);
		renderable.circle_radius = 15;
		mData.world->registry.assign<TimeOut>(projectile, mProjectileDuration);
		mProjectile = projectile;
	}
}

void FlashAbility::onKeyDown(vec2f target) {
	auto &sdata = mData.world->registry.get<SpatialData>(mData.owner);
	vec2f offset = target-sdata.position;
	if (offset.length() > mRange) {
		offset.truncate(mRange);
	}
	sdata.position += offset;
}

std::shared_ptr<Ability> AbilityFactory::TestProjectileAbility(World* world, Entity owner) {
	auto ability = std::make_shared<BasicProjectileAbility>(world, owner, 500, 10);
	ability->addEffect(std::make_shared<InstantSingleDamage>(Damage{Damage::Type::Puncture, 10}, ability->mData));
	ability->addIgnored(owner);
	return ability;
}

std::shared_ptr<Ability> AbilityFactory::TestBuffAbility(World* world, Entity owner, Condition condition) {
	auto ability = std::make_shared<SelfInstantEffectAbility>(world, owner);
	ability->addEffect(std::make_shared<InstantSingleCondition>(condition, ability->mData));
	return ability;
}

std::shared_ptr<Ability> AbilityFactory::TestAreaEffectTargetAbility(World* world, Entity owner) {
	auto ability = std::make_shared<AreaEffectTargetAbility>(world, owner);
	ability->addEffect(
		std::make_shared<InstantAreaExplosion>(
			std::make_shared<InstantSingleDamage>(
				Damage{Damage::Type::Impact, 10}, ability->mData),
			250, ability->mData)
		);
	ability->addIgnored(owner);
	return ability;
}

std::shared_ptr<Ability> AbilityFactory::TestPuckAbility(World* world, Entity owner) {
	auto ability = std::make_shared<PuckAbility>(world, owner, 400, 30);
	ability->addEffect(
		std::make_shared<InstantAreaExplosion>(
			std::make_shared<InstantSingleCondition>(
				Condition{Condition::Type::BURN, 3.0, 3.0 /* seconds */}, ability->mData),
			150, ability->mData)
		);
	ability->addIgnored(owner);
	return ability;
}

std::shared_ptr<Ability> AbilityFactory::TestHeartseekerAbility(World* world, Entity owner) {
	auto ability = std::make_shared<HeartseekerAbility>(world, owner, 500, 10);
	ability->addEffect(
		std::make_shared<InstantSingleDamage>(
			Damage{Damage::Type::Impact, 10},
			ability->mData));
	ability->addIgnored(owner);
	return ability;
}

std::shared_ptr<Ability> AbilityFactory::TestSprayAbility(World* world, Entity owner) {
	auto ability = std::make_shared<SprayAbility>(world, owner, 500, 2);
	ability->addEffect(
		std::make_shared<InstantSingleDamage>(
			Damage{Damage::Type::Impact, 10},
			ability->mData));
	ability->addIgnored(owner);
	return ability;
}
