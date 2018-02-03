#pragma once

#include "types.hpp"
#include "events.hpp"
#include "components.hpp"
#include <map>
#include <set>
#include <utility>
#include <list>

class SDL_Renderer;

struct World;

struct System {
public:
	System() : world(nullptr) {};
	virtual void init() {};
	virtual void update(TimeDelta dt) {};
	World *world;
};

struct MovementSystem : public System {
	void update(TimeDelta dt) override;
};

struct View {
public:
	template <typename T> vec2<T> inline viewCoordFromGlobal(vec2<T> globalpos) const {
		T x = (globalpos.x - viewcenter.x) * scale;
		T y = (globalpos.y - viewcenter.y) * scale;
		return {(x - y), (x + y)/1.73};
	}
	float scale; // larger is bigger
	vec2f viewcenter;
};

struct RenderSystem : public System {
public:
	RenderSystem(SDL_Renderer* renderer, int screenwidth, int screenheight)
		: _renderer(renderer),
		  timeSinceStart(0.0),
		  _screenwidth(screenwidth),
		  _screenheight(screenheight) {
		  	_view.scale = 1;
		  };
	void renderEntity(Entity entity);

	template <typename T> vec2<T> inline renderCoordFromGlobal(vec2<T> globalpos) const {
		return _view.viewCoordFromGlobal(globalpos) + vec2<T>{_screenwidth, _screenheight}/2;
	}

	void update(TimeDelta dt) override;
private:
	SDL_Renderer* _renderer;
	View _view;
	TimeDelta timeSinceStart;
	int _screenwidth, _screenheight;
};

struct CollisionSystem : public System {
public:
	CollisionSystem(int gridwidth);
	bool collides(Entity one, Entity two);
	void receive(const MovedEvent &e);
private:
	const int gridwidth;
	std::map<vec2i, std::set<Entity>> spatial_hash;
	const vec2i getGridCoords(vec2f pos) const {
		return {std::floor(pos.x/gridwidth), std::floor(pos.y/gridwidth)};
	}
	static bool _collides(const SpatialData &spatial1, const Collidable &collide1,
                          const SpatialData &spatial2, const Collidable &collide2);
};

struct DestructibleSystem : public System {
public:
	DestructibleSystem();
	void receive(const DamagedEvent &e);
	void update(TimeDelta dt) override;
};

struct InputSystem : public System {
	InputSystem();
	void receive(const SDL_Event& e);
	void update(TimeDelta dt) override;
};

class ConditionSystem : public System {
public:
	ConditionSystem() {};
	void receive(const ConditionEvent& e);
	void update(TimeDelta dt) override;
private:
	void tickCondition(Entity entity, Condition& condition, TimeDelta dt);
	static void recalculateStats(Stats &stats, Conditions &conditions);
};

class CollisionHandlerSystem : public System {
public:
	CollisionHandlerSystem() {};
	void receive(const CollidedEvent &e);
private:
	void handle(Entity one, Entity two);
};

struct ControlSystem : public System {
	ControlSystem() {};
	void receive(const Control_MoveAccelEvent& e);
	void receive(const Control_UseAbilityEvent& e);
};
