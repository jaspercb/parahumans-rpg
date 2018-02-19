#include <set>

#include "types.hpp"

// TODO: AOE effects should be able to give owner immunity

/* Your standard Composite design pattern. */

class World;

struct AbilityData {
	World* world;
	Entity owner;
	TimeDelta timeSinceUsed;
	std::set<Entity> ignored;
};

class Effect {
public:
	Effect(AbilityData& data) : mData(data) {};
	virtual void operator()(World* world, Entity e) {
		if (canEffect(e)) {
			__apply(world, e);
		}
	};
	virtual void __apply(World* world, Entity e) = 0;
protected:
	bool canEffect(Entity e) { return mData.ignored.find(e) == mData.ignored.end(); }
	AbilityData& mData;
};

/*
// TODO: Persistent Single Effects
class PersistentSingleEffect : public Effect {
// Effects like "I am stronger" (condition) or "things near me catch on fire" (attached aura)
public:
	PersistentSingleEffect(Entity owner) {};
	virtual void apply(World* world, Entity e)  = 0;
	virtual void remove(World* world, Entity e) = 0;
};
*/

class InstantSingleEffect : public Effect, public OnCollisionCallback {
/* Effects like "take 5 poison damage" or "catch on fire" */
public:
	InstantSingleEffect(AbilityData& data) : Effect(data) {};
	void operator()(World* world, Entity e) {
		if (canEffect(e)) {
			__apply(world, e);
		}
	};
};

class InstantSingleDamage : public InstantSingleEffect {
public:
	InstantSingleDamage(Damage damage, AbilityData& data)
		: mDamage(damage), InstantSingleEffect(data) {};
	virtual void __apply(World* world, Entity e) override;
private:
	Damage mDamage;
};

class InstantSingleCondition : public InstantSingleEffect {
public:
	InstantSingleCondition(const Condition& condition, AbilityData& data)
		: mCondition(condition), InstantSingleEffect(data) {};
	virtual void __apply(World* world, Entity e) override;
private:
	Condition mCondition;
};

class InstantAreaEffect : public Effect {
public:
	InstantAreaEffect(AbilityData& data) : Effect(data) {};
	virtual void operator()(World* world, vec2f pos) = 0;
	//virtual void operator()(World* world, Entity e) {};
};

class InstantAreaExplosion : public InstantAreaEffect {
public:
	InstantAreaExplosion(
		std::shared_ptr<InstantSingleEffect> effect, float radius,
		AbilityData& data)
	: InstantAreaEffect(data), mInstantSingleEffect(effect),
	  mExplosionRadius(radius) {};
	virtual void operator()(World* world, vec2f pos) override;
	void __apply(World* world, Entity entity) {};
private:
	std::shared_ptr<InstantSingleEffect> mInstantSingleEffect;
	float mExplosionRadius;
};
