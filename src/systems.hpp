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

struct ViewTransform {
public:
	template <typename T> vec2<T> inline viewCoordFromGlobal(vec2<T> globalpos) const {
		float x = (globalpos.x - viewcenter.x) * scale;
		float y = (globalpos.y - viewcenter.y) * scale;
		return vec2<T>{(x - y),
		               (x + y)/1.73};
	}
	template <typename T> vec2<T> inline globalCoordFromView(vec2<T> viewpos) const {
		float x = (1.73*viewpos.y + viewpos.x)/2;
		float y = (1.73*viewpos.y - viewpos.x)/2;
		return vec2<T>{x/scale + viewcenter.x,
		               y/scale + viewcenter.y};
	}
	template <typename T> vec2<T> inline screenCoordFromView(vec2<T> viewpos) const {
		return {viewpos.x + screensize.x/2, viewpos.y + screensize.y/2};
	}
	template <typename T> vec2<T> inline viewCoordFromScreen(vec2<T> screenpos) const {
		return {screenpos.x - screensize.x/2, screenpos.y - screensize.y/2};
	}
	template <typename T> vec2<T> inline screenCoordFromGlobal(vec2<T> globalpos) const {
		return screenCoordFromView(viewCoordFromGlobal(globalpos));
	}
	template <typename T> vec2<T> inline globalCoordFromScreen(vec2<T> screenpos) const {
		return globalCoordFromView(viewCoordFromScreen(screenpos));
	}
	float scale; // larger is bigger
	vec2f viewcenter;
	vec2i screensize;
};

struct RenderSystem : public System {
public:
	RenderSystem(SDL_Renderer* renderer, const ViewTransform* viewxform)
		: _renderer(renderer),
		  timeSinceStart(0.0),
		  _viewxform(viewxform) {};
	void renderEntity(Entity entity);
	void update(TimeDelta dt) override;
private:
	SDL_Renderer* _renderer;
	TimeDelta timeSinceStart;
	const ViewTransform* _viewxform;
};

struct CameraTrackingSystem : public System {
// Responsible for making the camera track the player
// TODO: screen shake
public:
	CameraTrackingSystem(ViewTransform* viewxform)
		: _viewxform(viewxform) {}
	void update(TimeDelta dt) override;
private:
	ViewTransform* _viewxform;
};

struct CollisionSystem : public System {
public:
	CollisionSystem(int gridwidth);
	bool collides(Entity one, Entity two);
	void receive(const MovedEvent &e);
	void receive(const CollidableCreatedEvent& e);
private:
	const int gridwidth;
	std::map<vec2i, std::set<Entity>> spatial_hash;
	const vec2i getGridCoords(vec2f pos) const {
		return {std::floor(pos.x/gridwidth), std::floor(pos.y/gridwidth)};
	}
	static bool _collides(const SpatialData &spatial1, const Collidable &collide1,
                          const SpatialData &spatial2, const Collidable &collide2);
};

class DestructibleSystem : public System {
public:
	DestructibleSystem();
	void receive(const DamagedEvent &e);
	void receive(const HealedEvent &e);
	void update(TimeDelta dt) override;
};

class InputSystem : public System {
/* Translates keyboard events into Control events. */
public:
	InputSystem(const ViewTransform* viewxform)
	: _viewxform(viewxform) {};
	void receive(const SDL_Event& e);
	void update(TimeDelta dt) override;
private:
	vec2f getMouseGlobalCoords() const;
	const ViewTransform* _viewxform;
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

struct AbilitySystem : public System {
	AbilitySystem() {};
	void update(TimeDelta dt) override;
};
