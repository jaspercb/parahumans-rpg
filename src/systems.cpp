#include "systems.hpp"
#include "World.hpp"
#include "sdlTools.hpp"

#include <unistd.h>
#include <iostream>

#include <map>

void MovementSystem::update(TimeDelta dt) {
	world->registry.view<SpatialData>().each([dt, this](auto entity, auto &sdata) {
		auto oldpos = sdata.position;
		sdata.position.x += sdata.velocity.x * dt;
		sdata.position.y += sdata.velocity.y * dt;
		if (sdata.isMoving()) {
			sdata.timeMoving += dt;
			world->bus.publish<MovedEvent>(entity, oldpos, sdata.position);
		}
		else {
			sdata.timeMoving = 0;
		}
	});
};

void renderOvalOffset(SDL_Renderer* renderer, const ViewTransform* view, vec2i ipos, vec2i offset, float angle, float height, float rx, float ry, SDL_Color color) {
	const float scale = view->scale;
	offset.rotate(angle);

	vec2i screenpos = ipos + view->viewCoordFromGlobal(offset);
	filledEllipseColor(renderer, screenpos.x, screenpos.y - scale*height, int(rx*scale), int(ry*scale), SDL_ColortoUint32(color));
	ellipseColor(renderer, screenpos.x, screenpos.y - scale*height, int(rx*scale), int(ry*scale), SDL_ColortoUint32(SDL_Colors::BLACK));
}

void RenderSystem::renderEntity(Entity entity) {
	const auto &sdata = world->registry.get<SpatialData>(entity);
	const auto &renderable = world->registry.get<Renderable>(entity);
	vec2f fpos = _viewxform->screenCoordFromGlobal(sdata.position);
	vec2i ipos = {fpos.x, fpos.y};
	ipos.y -= sdata.z;
	switch(renderable.type) {
		case Renderable::Type::Circle: {
			filledCircleColor(_renderer, ipos.x, ipos.y, _viewxform->scale * renderable.circle_radius, SDL_ColortoUint32(renderable.color));
			break;
		}
		case Renderable::Type::Line: {
			vec2i offset = {renderable.line_displacement_x, renderable.line_displacement_y};
			vec2i start = ipos - offset/2.0;
			vec2i end   = ipos + offset/2.0;
			lineColor(_renderer, start.x, start.y, end.x, end.y, SDL_ColortoUint32(renderable.color));
			break;
		}
		case Renderable::Type::Person: {
			const auto& stats = world->registry.get<Stats>(entity);
			vec2f orientation= {0, 1};
			float walkspeed = stats.speed() * 0.065;
			orientation.rotate(sdata.orientation);
			float phase = sdata.timeMoving;
			float depthy = _viewxform->screenCoordFromGlobal(orientation).y;
			if (depthy > 0) {
				renderOvalOffset(_renderer, _viewxform, ipos, {0, -12}, sdata.orientation, 18, 5, 5, renderable.color); // arms
				renderOvalOffset(_renderer, _viewxform, ipos, {6*sin(phase*walkspeed), -6}, sdata.orientation, 0, 4, 4, renderable.color); // legs
			} else{
				renderOvalOffset(_renderer, _viewxform, ipos, {0,  12}, sdata.orientation, 18, 5, 5, renderable.color);
				renderOvalOffset(_renderer, _viewxform, ipos, {-6*sin(phase*walkspeed), 6}, sdata.orientation, 0, 4, 4, renderable.color);
			}
			// head, body
			renderOvalOffset(_renderer, _viewxform, ipos, {0, 0}, sdata.orientation, 16, 10, 15, renderable.color); // body
			renderOvalOffset(_renderer, _viewxform, ipos, {0, 0}, sdata.orientation, 43, 15, 15, renderable.color); // head
			if (depthy <= 0) {
				renderOvalOffset(_renderer, _viewxform, ipos, {0, -12}, sdata.orientation, 18, 5, 5, renderable.color); // arms
				renderOvalOffset(_renderer, _viewxform, ipos, {6*sin(phase*walkspeed), -6}, sdata.orientation, 0, 4, 4, renderable.color); // legs
			} else{
				renderOvalOffset(_renderer, _viewxform, ipos, {0,  12}, sdata.orientation, 18, 5, 5, renderable.color);
				renderOvalOffset(_renderer, _viewxform, ipos, {-6*sin(phase*walkspeed), 6}, sdata.orientation, 0, 4, 4, renderable.color);
			}
			break;
		}
		case Renderable::Type::Cube:
		{
			break; // TOO SLOW
			// TODO: add scale dependency
			float width = 50;
			vec2f xstep = {-width, 0};
			xstep = _viewxform->screenCoordFromGlobal(xstep);
			vec2f ystep = {0, -width};
			ystep = _viewxform->screenCoordFromGlobal(ystep);
			Sint16 vx[4];
			Sint16 vy[4];
			vx[0] = ipos.x;
			vy[0] = ipos.y;
			vx[1] = ipos.x;
			vy[1] = ipos.y - width;
			vx[2] = ipos.x + xstep.x;
			vy[2] = ipos.y + xstep.y - width;
			vx[3] = ipos.x + xstep.x;
			vy[3] = ipos.y + xstep.y;

			filledPolygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::LIGHTGREY));
			polygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::BLACK));

			vx[2] = ipos.x + ystep.x;
			vy[2] = ipos.y + ystep.y - width;
			vx[3] = ipos.x + ystep.x;
			vy[3] = ipos.y + ystep.y;

			filledPolygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::LIGHTGREY));
			polygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::BLACK));

			vx[0] = ipos.x + xstep.x;
			vy[0] = ipos.y + xstep.y - width;
			vx[3] = ipos.x + ystep.x + xstep.x;
			vy[3] = ipos.y + ystep.y + xstep.y - width;

			filledPolygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::LIGHTGREY));
			polygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::BLACK));


		}
	}
	// render HP bar if needed
	if (!world->registry.has<Destructible>(entity)) return;
	const auto &destructible = world->registry.get<Destructible>(entity);
	static const vec2i HP_BAR_OFFSET = {0, -70};
	static const vec2i HP_BAR_SIZE = {50, 10};
	vec2i topleft = ipos + HP_BAR_OFFSET * _viewxform->scale - HP_BAR_SIZE/2;
	vec2i bottomright = ipos + HP_BAR_OFFSET * _viewxform->scale + HP_BAR_SIZE/2;
	boxColor(_renderer, topleft.x, topleft.y, bottomright.x, bottomright.y, SDL_ColortoUint32(SDL_Colors::RED));
	bottomright.x -= HP_BAR_SIZE.x * (1.0 - float(destructible.HP)/destructible.HP.max);
	boxColor(_renderer, topleft.x, topleft.y, bottomright.x, bottomright.y, SDL_ColortoUint32(SDL_Colors::GREEN));
}

void RenderSystem::update(TimeDelta dt) {
	// Fill screen with white
	SDL_Renderer* renderer = _renderer;
	SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255); // white
	SDL_RenderClear(renderer);

	// distance from camera -> renderable
	std::multimap<float, Entity> drawqueue;

	world->registry.view<SpatialData, Renderable>().each([dt, this, &drawqueue](Entity entity, SpatialData &sdata, Renderable &renderable) {
		switch(renderable.type) {
			// Different renderables have a different effective z-buffer for back-to-front painting
			case Renderable::Type::Cube:
				drawqueue.emplace(sdata.position.x - 25 + sdata.position.y - 25 + sdata.z, entity);
				break;
			default:
				drawqueue.emplace(sdata.position.x + sdata.position.y + sdata.z, entity);
		}
	});
	for (auto& pair : drawqueue) {
		auto entity = pair.second;
		renderEntity(entity);
	}
	SDL_RenderPresent(renderer);
	timeSinceStart += dt;
};

CollisionSystem::CollisionSystem(int gridwidth)
	: gridwidth(gridwidth)
{}

bool CollisionSystem::collides(Entity one, Entity two) {
	if (one == two) return false;
	if (!world->registry.has<SpatialData>(one)
	 || !world->registry.has<Collidable>(one)
	 || !world->registry.has<SpatialData>(two)
	 || !world->registry.has<Collidable>(two)) return false;
	auto spatial1 = world->registry.get<SpatialData>(one);
	auto spatial2 = world->registry.get<SpatialData>(two);
	auto collide1 = world->registry.get<Collidable>(one);
	auto collide2 = world->registry.get<Collidable>(two);
	return _collides(spatial1, collide1, spatial2, collide2);
}

bool CollisionSystem::_collides(const SpatialData &spatial1,
                                const Collidable  &collide1,
                                const SpatialData &spatial2,
                                const Collidable  &collide2) {
	switch (collide1.type) {
	case Collidable::Type::Circle:
		switch(collide2.type) {
		case Collidable::Type::Circle:
			// TODO: 3D collisions
			return spatial1.position.dist(spatial2.position) < collide1.circle_radius + collide2.circle_radius;
		}
		// TODO: rectangle-circle collisions
		break;
	}
	assert(false);
	return false;
}

// TODO: listen to EntityCreated and EntityDestroyed?

void CollisionSystem::receive(const MovedEvent &e) {
	auto oldPos = e.oldPos;
	auto oldGridCoords = getGridCoords(oldPos);
	auto newGridCoords = getGridCoords(e.newPos);
	if (oldGridCoords != newGridCoords) {
		spatial_hash[oldGridCoords].erase(e.entity);
		spatial_hash[newGridCoords].insert(e.entity);
	}
	for (int dx=-1; dx<=1; dx++) {
		for (int dy=-1; dy<=1; dy++) {
			vec2i d = {dx, dy};
			auto iter = spatial_hash.find(newGridCoords + d);
			if (iter != spatial_hash.end()) {
				for (const auto &entity : iter->second) {
					if (collides(e.entity, entity)) {
						auto collide1 = world->registry.get<Collidable>(e.entity);
						auto collide2 = world->registry.get<Collidable>(entity);
						if (collide1.canCollide(entity) && collide2.canCollide(e.entity)) {
							world->bus.publish<CollidedEvent>(e.entity, entity);
						}
					}
				}
			}
		}
	}
}

DestructibleSystem::DestructibleSystem() {}

void DestructibleSystem::receive(const DamagedEvent &e) {
	// TODO: Pay attention to damage types, source
	if (!world->registry.has<Destructible>(e.damaged)) return;
	auto &destructible = world->registry.get<Destructible>(e.damaged);
	if (!destructible.indestructible) {
		destructible.HP -= e.damage;
	}
}

void DestructibleSystem::update(TimeDelta dt) {}

vec2f InputSystem::getMouseGlobalCoords() const {
	vec2i screenpos;
	SDL_GetMouseState(&screenpos.x, &screenpos.y);
	return _viewxform->globalCoordFromScreen(screenpos);
}

void InputSystem::receive(const SDL_Event& e) {
	Ability speedup;
	speedup.type = Ability::Type::SelfCondition;
	speedup.cooldown = 0;
	speedup.condition = {Condition::Priority::Multiplier, Condition::Type::MOD_SPEED, 2, 1 /* seconds */};

	Ability accelup;
	accelup.type = Ability::Type::SelfCondition;
	accelup.cooldown = 0;
	accelup.condition = {Condition::Priority::Multiplier, Condition::Type::MOD_ACCEL, 2, 1};

	Ability fireball;
	fireball.type = Ability::Type::FireProjectile;
	fireball.cooldown = 0;
	fireball.projectile_speed = 50;
	// Condition burn = {Condition::Priority::None, Condition::Type::BURN, 10, 200000000};

	switch(e.type) {
	case SDL_KEYDOWN:
		switch(e.key.keysym.sym) {
		case SDLK_q:
			world->registry.view<Controllable>().each([this, &speedup, &accelup](auto entity, const auto &controllable) {
				world->bus.publish<Control_UseAbilityEvent>(entity, vec2f{0, 0}, &speedup);
				world->bus.publish<Control_UseAbilityEvent>(entity, vec2f{0, 0}, &accelup);
			});
			break;
		case SDLK_e:
			auto target = getMouseGlobalCoords();
			world->registry.view<Controllable>().each([this, target, &fireball](auto entity, const auto &controllable) {
				world->bus.publish<Control_UseAbilityEvent>(entity, target, &fireball);
			});
			break;
		}
		break;
	}
}

void InputSystem::update(TimeDelta dt) {
	SDL_Event e;

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT)
			world->bus.publish<WindowExitEvent>();
		else
			world->bus.publish<SDL_Event>(e);
	}

	world->bus.publish<SDL_Event>(e);
	world->registry.view<SpatialData, Controllable>().each([this, dt, keys](auto entity, auto &sdata, auto &controllable) {
		vec2f accel = {0, 0};
		if (keys[SDL_SCANCODE_LEFT]) {
			accel.x -= 1;
			accel.y += 1;
		}
		if (keys[SDL_SCANCODE_RIGHT]) {
			accel.x += 1;
			accel.y -= 1;
		}
		if (keys[SDL_SCANCODE_UP]) {
			accel.x -= 1;
			accel.y -= 1;
		}
		if (keys[SDL_SCANCODE_DOWN]) {
			accel.x += 1;
			accel.y += 1;
		}
		world->bus.publish<Control_MoveAccelEvent>(entity, accel);
	});
}

void ConditionSystem::receive(const ConditionEvent& e) {
	auto &stats = world->registry.get<Stats>(e.receiver);
	auto &conditions = world->registry.get<Conditions>(e.receiver);
	conditions.list.push_front(e.condition);
	stats.dirty = true;
};

void ConditionSystem::update(TimeDelta dt) {
	world->registry.view<Conditions, Stats>().each([dt, this](auto entity, auto &conditions, auto &stats) {
		for (auto condition = conditions.list.begin(); condition != conditions.list.end(); condition++) {
			tickCondition(entity, *condition, dt);
			if (condition->isExpired()) {
				condition = conditions.list.erase(condition);
				stats.dirty = true;
			}
		}
		if (stats.dirty) {
			recalculateStats(stats, conditions);
		}
	});
}

void ConditionSystem::tickCondition(Entity entity, Condition& condition, TimeDelta dt) {
	switch (condition.type) {
		case Condition::Type::BURN: // damage
		case Condition::Type::BLEED: // damage
		case Condition::Type::POISON: // damage
		world->bus.publish<DamagedEvent>(entity, entity, DamageType::Puncture, condition.strength * dt);
		break;
	}
	condition.timeLeft -= dt;
}

void ConditionSystem::recalculateStats(Stats &stats, Conditions &conditions) {
	memcpy(&stats.stats, &stats.basestats, sizeof(stats.stats));
	conditions.list.sort();
	for (const auto &condition : conditions.list) {
		float* var = nullptr;
		switch (condition.type) {
			case Condition::Type::MOD_SPEED: var = &stats.stats.speed; break;
			case Condition::Type::MOD_ACCEL: var = &stats.stats.accel; break;
		}
		switch (condition.priority) {
			case Condition::Priority::Adder: *var += condition.strength; break;
			case Condition::Priority::Multiplier: *var *= condition.strength; break;
		}
	}
	stats.dirty = false;
}

void CollisionHandlerSystem::receive(const CollidedEvent &e) {
	handle(e.one, e.two);
	handle(e.two, e.one);
}

void CollisionHandlerSystem::handle(Entity source, Entity target) {
	// Applies source's on-collision effects to target
	if (world->registry.has<OnCollision>(source)) {
		const auto &oncollision = world->registry.get<OnCollision>(source);
		for (const auto &condition : oncollision.conditions) {
			world->bus.publish<ConditionEvent>(condition, source, target);
		}
		if (oncollision.damage) {
			world->bus.publish<DamagedEvent>(source, target, oncollision.damagetype, oncollision.damage);
		}
	}
}

void ControlSystem::receive(const Control_MoveAccelEvent& e) {
	SpatialData& sdata = world->registry.get<SpatialData>(e.entity);
	const Stats& stats = world->registry.get<Stats>(e.entity);
	vec2f accel = e.accel;
	accel.normalize();
	accel *= stats.accel();
	sdata.velocity += accel;
	if (sdata.velocity.x || sdata.velocity.y) {
		sdata.orientation = atan2f(sdata.velocity.y, sdata.velocity.x);
	}
	sdata.velocity *= stats.speed() / (stats.accel() + stats.speed());
	if (!accel.x && !accel.y && sdata.velocity.length() < 1 /*epsilon value*/ ) {
		sdata.velocity *= 0;
	}
}

void ControlSystem::receive(const Control_UseAbilityEvent& e) {
	std::cout<<"a"<<std::endl;
	if (!e.ability->isOffCooldown()) return;
	std::cout<<"b"<<std::endl;
	SpatialData& abilities = world->registry.get<SpatialData>(e.entity);
	switch(e.ability->type) {
	case Ability::Type::FireProjectile: {
		auto &sdata = world->registry.get<SpatialData>(e.entity);
		auto velocity = e.target - sdata.position;
		velocity.truncate(e.ability->projectile_speed);
		Entity projectile = world->registry.create();
		world->registry.assign<SpatialData>(projectile, sdata.position, velocity);
		world->registry.assign<Renderable>(projectile, Renderable::Type::Circle);
		auto& collidable = world->registry.assign<Collidable>(projectile, Collidable::Circle, 10);
		collidable.addIgnored(e.entity);
		auto& oncollision = world->registry.assign<OnCollision>(projectile);
		oncollision.damage = 1;
		}
		break;
	case Ability::Type::SelfCondition:
		std::cout<<"c"<<std::endl;
		world->bus.publish<ConditionEvent>(e.ability->condition, e.entity, e.entity);
		break;
	}
	e.ability->timeSinceUsed = 0;
}

void AbilitySystem::update(TimeDelta dt) {
	world->registry.view<AbilityData>().each([dt, this](auto entity, auto &abilitydata) {
		for (auto &ability : abilitydata.abilities) {
			ability.timeSinceUsed += dt;
		}
	});
}
