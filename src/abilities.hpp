#pragma once
#include <memory>
#include <optional>
#include "types.hpp"
#include "effects.hpp"

/*
 We differentiate between EFFECTS and ABILITIES.
 An EFFECT is an explosion, "take 5 damage", catching on fire, transforming into a frog.
 An ABILITY is a way of applying that effect. A bullet, a spray of bullets, a strike, a beam, etc.

 Example powers:
 	Throw fireballs which explode on impact
		ABILITY=BasicProjectileAbility
		EFFECT=InstantAreaExplosion
	Throw bolts which freeze a person's subjective passage of time for a while
		ABILITY=BasicProjectileABility
		EFFECT=InstantSingleCondition{Multiply subjective time rate by 0 for 2 real seconds}

Abilities have a list of ignored objects.
Effects have a reference to this list.
*/

class Ability {
friend class AbilityFactory;
public:
	Ability(World* world, Entity owner)
		: mData{world, owner, 99.9} {}
	virtual void onKeyDown(vec2f target) {};
	virtual void onKeyUp(vec2f target) {};
	virtual bool isUsable() { return true; };
	void update(TimeDelta dt) {
		mData.timeSinceUsed += dt;
	}
	void addIgnored(Entity e) {
		mData.ignored.insert(e);
	}
protected:
	// All effect children of this ability will have access to this block.
	AbilityData mData;
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
	AreaEffectTargetAbility(World* world, Entity owner)
		: Ability(world, owner) {};
	void onKeyDown(vec2f target) override;
	void addEffect(std::shared_ptr<InstantAreaEffect> effect) {
		mInstantAreaEffects.push_back(effect);
	}
private:
	std::vector<std::shared_ptr<InstantAreaEffect>> mInstantAreaEffects;
};

class BasicProjectileAbility : public Ability {
public:
	BasicProjectileAbility(World* world, Entity owner,
	                       float projectileSpeed, float projectileRadius)
		: Ability(world, owner), mProjectileSpeed(projectileSpeed),
		  mProjectileRadius(projectileRadius) {};

	void onKeyDown(vec2f target) override;
	void addEffect(std::shared_ptr<InstantSingleEffect> effect) {
		mInstantSingleEffects.push_back(effect);
	}
protected:
	Entity makeProjectile(vec2f position, vec2f target);
private:
	std::vector<std::shared_ptr<InstantSingleEffect>> mInstantSingleEffects;
	float mProjectileSpeed;
	float mProjectileRadius;
};

class HeartseekerAbility : public BasicProjectileAbility {
public:
	HeartseekerAbility(World* world, Entity owner, float projectileSpeed, float projectileRadius)
		: BasicProjectileAbility(world, owner, projectileSpeed, projectileRadius) {};
	void onKeyDown(vec2f target) override;
	void onKeyUp(vec2f target) override;
private:
	std::optional<Entity> mActiveProjectile;
};

class SelfInstantEffectAbility : public Ability {
public:
	SelfInstantEffectAbility(World* world, Entity owner)
		: Ability(world, owner) {};
	void onKeyDown(vec2f target) override;
	void addEffect(std::shared_ptr<InstantSingleEffect> effect) {
		mInstantSingleEffects.push_back(effect);
	}
private:
	std::vector<std::shared_ptr<InstantSingleEffect>> mInstantSingleEffects;
};

class PuckAbility : public Ability {
	/* Fire projectile which passes through foes. Use power again to swap location
	   with projectile, with non-damage, effect-only explosions at both points.
	*/
public:
	PuckAbility(World* world, Entity owner,
	            float projectileSpeed, float projectileDuration)
		: Ability(world, owner), mProjectileSpeed(projectileSpeed),
		  mProjectileDuration(projectileDuration) {};
	void onKeyDown(vec2f target) override;
	void addEffect(std::shared_ptr<InstantAreaEffect> effect) {
		mInstantAreaEffects.push_back(effect);
	}
private:
	std::vector<std::shared_ptr<InstantAreaEffect>> mInstantAreaEffects;
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

class AbilityFactory {
public:
	static std::shared_ptr<Ability> TestProjectileAbility(World* world, Entity owner);
	static std::shared_ptr<Ability> TestBuffAbility(World* world, Entity owner, Condition condition);
	static std::shared_ptr<Ability> TestAreaEffectTargetAbility(World* world, Entity owner);
	static std::shared_ptr<Ability> TestPuckAbility(World* world, Entity owner);
	static std::shared_ptr<Ability> TestHeartseekerAbility(World* world, Entity owner);
};
