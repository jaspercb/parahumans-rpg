#pragma once

#include "entityx/entityx.h"
#include "types.hpp"
#include <map>
#include <utility>
#include <list>

class SDL_Renderer;

struct MovementSystem : public entityx::System<MovementSystem> {
	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
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

struct RenderSystem : public entityx::System<RenderSystem> {
public:
	RenderSystem(SDL_Renderer* renderer) : _renderer(renderer), timeSinceStart(0.0) {};
	void renderEntity(entityx::Entity entity);
	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
private:
	SDL_Renderer* _renderer;
	View _view;
	entityx::TimeDelta timeSinceStart;
};

struct CollisionSystem : public entityx::System<CollisionSystem> {
public:
	CollisionSystem() {};
	bool collides(entityx::Entity one, entityx::Entity two);
	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
private:
	std::map<std::pair<int, int>, std::list<entityx::Entity>> spatial_hash;
};
