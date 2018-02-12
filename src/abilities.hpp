#pragma once
#include <memory>
#include <optional>
#include "types.hpp"
#include "effects.hpp"

class Ability {
public:
	Ability(World* world, Entity owner)
		: mWorld(world), mOwner(owner) {}
	virtual void onKeyDown(vec2f target) {};
	virtual void onKeyUp(vec2f target) {};
	virtual bool isUsable() { return true; };
	void update(TimeDelta dt) {
		mTimeSinceUsed += dt;
	}
protected:
	World* mWorld;
	Entity mOwner;
	TimeDelta mTimeSinceUsed;
};

/*
class ToggleAbility : public Ability {
public:
	ToggleAbility(World* world, Entity owner)
		: Ability(world, owner) {};
	void onKeyDown(vec2f target) override;
};
*/

class AreaEffectTargetAbility : public Ability {
public:
	AreaEffectTargetAbility(World* world, Entity owner, std::shared_ptr<InstantAreaEffect> instantAreaEffect)
		: Ability(world, owner), mInstantAreaEffect(instantAreaEffect) {};
	void onKeyDown(vec2f target) override;
private:
	std::shared_ptr<InstantAreaEffect> mInstantAreaEffect;
};

class BasicProjectileAbility : public Ability {
public:
	BasicProjectileAbility(World* world, Entity owner, std::shared_ptr<InstantSingleEffect> instantSingleEffect,
	                       float projectileSpeed, float projectileRadius)
		: Ability(world, owner), mInstantSingleEffect(instantSingleEffect),
		  mProjectileSpeed(projectileSpeed), mProjectileRadius(projectileRadius) {};

	void onKeyDown(vec2f target) override;
protected:
	Entity makeProjectile(vec2f position, vec2f target);
private:
	std::shared_ptr<InstantSingleEffect> mInstantSingleEffect;
	float mProjectileSpeed;
	float mProjectileRadius;
};

class HeartseekerAbility : public BasicProjectileAbility {
public:
	HeartseekerAbility(World* world, Entity owner, std::shared_ptr<InstantSingleEffect> instantSingleEffect,
	                       float projectileSpeed, float projectileRadius)
		: BasicProjectileAbility(world, owner, instantSingleEffect, projectileSpeed, projectileRadius) {};
	void onKeyDown(vec2f target) override;
	void onKeyUp(vec2f target) override;
private:
	std::optional<Entity> mActiveProjectile;
};
class SelfInstantEffectAbility : public Ability {
public:
	SelfInstantEffectAbility(World* world, Entity owner, std::shared_ptr<InstantSingleEffect> instantSingleEffect)
		: Ability(world, owner), mInstantSingleEffect(instantSingleEffect) {};
	void onKeyDown(vec2f target) override;
private:
	std::shared_ptr<InstantSingleEffect> mInstantSingleEffect;
};

class PuckAbility : public Ability {
	/* Fire projectile which passes through foes. Use power again to swap location
	   with projectile, with non-damage, effect-only explosions at both points.
	*/
public:
	PuckAbility(World* world, Entity owner, std::shared_ptr<InstantAreaEffect> instantAreaEffect,
	            float projectileSpeed, float projectileDuration)
		: Ability(world, owner), mInstantAreaEffect(instantAreaEffect), mProjectileSpeed(projectileSpeed),
		  mProjectileDuration(projectileDuration) {};
	void onKeyDown(vec2f target) override;
private:
	std::shared_ptr<InstantAreaEffect> mInstantAreaEffect;
	std::optional<Entity> mProjectile;
	float mProjectileSpeed;
	float mProjectileDuration;
};

class FlashAbility : public Ability {
public:
	FlashAbility(World* world, Entity owner, float range)
		: Ability(world, owner), mRange(range) {};
	void onKeyDown(vec2f target) override;
private:
	float mRange;
	// float mStaminaCost;
};

std::shared_ptr<Ability> TestProjectileAbility(World* world, Entity owner);
std::shared_ptr<Ability> TestBuffAbility(World* world, Entity owner, Condition condition);
std::shared_ptr<Ability> TestAreaEffectTargetAbility(World* world, Entity owner);
std::shared_ptr<Ability> TestPuckAbility(World* world, Entity owner);
std::shared_ptr<Ability> TestHeartseekerAbility(World* world, Entity owner);
