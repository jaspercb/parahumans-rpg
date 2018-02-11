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