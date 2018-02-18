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
	void renderRectangularPrism(
		const SpatialData& sdata, const Renderable& renderable,
		SDL_Color fillcolor, SDL_Color linecolor);
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
	bool collides(Entity one, Entity two) const;
	void receive(const MovedEvent &e);
	void receive(const EntityDestroyedEvent &e);
	void receive(const CollidedEvent &e);
	void update(TimeDelta dt) override;
private:
	const int gridwidth;
	std::map<vec2i, std::set<Entity>> mSpatialHash;
	std::map<Entity, vec2i> mGridCoords;
	const vec2i getGridCoords(vec2f pos) const {
		return {std::floor(pos.x/gridwidth), std::floor(pos.y/gridwidth)};
	}
	bool isWatching(Entity e) const {
		return mGridCoords.find(e) != mGridCoords.end();
	}
	static bool _collides(
		const SpatialData &spatial1, const Collidable &collide1,
		const SpatialData &spatial2, const Collidable &collide2);
	bool isCollidingWithTile(
		const SpatialData& spatial, const Collidable& collidable,
		TileLayout& layout);
	std::unordered_set<Entity> mToDestroy;
	std::unordered_set<Entity> mPotentiallyMoved;
};

class DestructibleSystem : public System {
public:
	DestructibleSystem() {};
	void receive(const DamagedEvent &e);
	void receive(const HealedEvent &e);
	void update(TimeDelta dt) override;
private:
	std::unordered_set<Entity> mToDestroy;
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

// Manages ability cooldowns.
struct AbilitySystem : public System {
	AbilitySystem() {};
	void update(TimeDelta dt) override;
};

struct TimeOutSystem : public System {
	void update(TimeDelta dt) override;
};
