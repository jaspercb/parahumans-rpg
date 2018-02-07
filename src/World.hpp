#pragma once
#include <SDL2/SDL.h>

#include "events.hpp"
#include "entt/entt.hpp"
#include "systems.hpp"

struct World {
	entt::Registry<Entity> registry;
	entt::ManagedBus<MovedEvent,
	                 CollidedEvent,
	                 DamagedEvent,
	                 HealedEvent,
	                 WindowExitEvent,
	                 ConditionEvent,
	                 SDL_Event,
	                 Control_UseAbilityEvent,
	                 Control_MoveAccelEvent,
	                 EntityDestroyedEvent> bus;
	std::list<std::shared_ptr<System>> systems;

	template<typename SystemType> void addSystem(std::shared_ptr<SystemType> system) {
		system->world = this;
		system->init();
		bus.reg(system);
		systems.push_back(std::move(system));
	}

	void update_all(TimeDelta dt) {
		for (auto &system : systems) {
			system->update(dt);
		}
	}

	void destroy(Entity e) {
		bus.publish<EntityDestroyedEvent>(e);
		registry.destroy(e);
	}
};
