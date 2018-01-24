#pragma once

struct World {
	entt::Registry<Entity> registry;
	std::list<std::unique_ptr<System>> systems;

	void addSystem(std::unique_ptr<System> system) {
		system->world = this;
		systems.push_back(std::move(system));
	}

	void update_all(TimeDelta dt) {
		for (auto &system : systems) {
			system->update(dt);
		}
	}
};
