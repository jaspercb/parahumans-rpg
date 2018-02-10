#pragma once
#include <memory>
#include "types.hpp"

class World;

class Effect {
	virtual void operator()(World* world, Entity e) = 0;
};

class PersistentSingleEffect : public Effect {
/* Effects like "I am stronger" or "things near me catch on fire" */
public:
	PersistentSingleEffect() {};
	virtual void apply(World* world, Entity e)  = 0;
	virtual void remove(World* world, Entity e) = 0;
};

class InstantSingleEffect : public Effect, public OnCollisionCallback {
/* Effects like "I am stronger" or "things near me catch on fire" */
public:
	InstantSingleEffect() {};
	virtual void operator()(World* world, Entity e) = 0;
};

class InstantSingleDamage : public InstantSingleEffect {
public:
	InstantSingleDamage(Damage damage)
		: mDamage(damage) {};
	virtual void operator()(World* world, Entity e) override;
private:
	Damage mDamage;
};

class InstantSingleCondition : public InstantSingleEffect {
public:
	InstantSingleCondition(const Condition& condition)
		: mCondition(condition) {};
	virtual void operator()(World* world, Entity e) override;
private:
	Condition mCondition;
};

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

class BasicProjectileAbility : public Ability {
public:
	BasicProjectileAbility(World* world, Entity owner, std::shared_ptr<InstantSingleEffect> instantEffect,
	                       float projectileSpeed, float projectileRadius)
		: Ability(world, owner), mInstantEffect(std::move(instantEffect)),
		  mProjectileSpeed(projectileSpeed), mProjectileRadius(projectileRadius) {};

	void onKeyDown(vec2f target) override;
private:
	std::shared_ptr<InstantSingleEffect> mInstantEffect;
	float mProjectileSpeed;
	float mProjectileRadius;
};

class SelfInstantEffectAbility : public Ability {
public:
	SelfInstantEffectAbility(World* world, Entity owner, std::shared_ptr<InstantSingleEffect> instantEffect)
		: Ability(world, owner), mInstantEffect(instantEffect) {};
	void onKeyDown(vec2f target) override;
private:
	std::shared_ptr<InstantSingleEffect> mInstantEffect;
};

std::shared_ptr<Ability> TestProjectileAbility(World* world, Entity owner);
std::shared_ptr<Ability> TestBuffAbility(World* world, Entity owner, Condition condition);
