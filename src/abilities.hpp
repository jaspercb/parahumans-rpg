#pragma once
#include <memory>
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
private:
	std::shared_ptr<InstantSingleEffect> mInstantSingleEffect;
	float mProjectileSpeed;
	float mProjectileRadius;
};

class SelfInstantEffectAbility : public Ability {
public:
	SelfInstantEffectAbility(World* world, Entity owner, std::shared_ptr<InstantSingleEffect> instantSingleEffect)
		: Ability(world, owner), mInstantSingleEffect(instantSingleEffect) {};
	void onKeyDown(vec2f target) override;
private:
	std::shared_ptr<InstantSingleEffect> mInstantSingleEffect;
};

std::shared_ptr<Ability> TestProjectileAbility(World* world, Entity owner);
std::shared_ptr<Ability> TestBuffAbility(World* world, Entity owner, Condition condition);
std::shared_ptr<Ability> TestAreaEffectTargetAbility(World* world, Entity owner);
