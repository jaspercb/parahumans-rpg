#pragma once

#include "entt/entt.hpp"
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
		auto x = (globalpos.x - viewcenter.x);
		auto y = (globalpos.y - viewcenter.y);
		return vec2<T>((x - y),
		             (x + y)/1.73);
	}
private:
	vec2f viewcenter;
};

struct RenderSystem : public System {
public:
	RenderSystem(SDL_Renderer* renderer, int screenwidth, int screenheight)
		: _renderer(renderer),
		  _screenwidth(screenwidth),
		  _screenheight(screenheight),
		  timeSinceStart(0.0) {};
	void renderEntity(Entity entity);

	template <typename T> vec2<T> inline renderCoordFromGlobal(vec2<T> globalpos) const {
		return _view.viewCoordFromGlobal(globalpos) + vec2<T>(_screenwidth, _screenheight)/2;
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
		return vec2i(std::floor(pos.x/gridwidth), std::floor(pos.y/gridwidth));
	}
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
