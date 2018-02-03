#pragma once
#include <SDL2/SDL.h>

#include "events.hpp"
#include "entt/entt.hpp"
#include "systems.hpp"

struct World {
	entt::Registry<Entity> registry;
	entt::ManagedBus<MovedEvent, CollidedEvent, DamagedEvent, WindowExitEvent,
	                 ConditionEvent, SDL_Event, Control_UseAbilityEvent,
	                 Control_MoveAccelEvent, CollidableCreatedEvent> bus;
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
};
