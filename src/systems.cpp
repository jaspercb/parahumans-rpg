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

	vec2i screenpos = ipos + view->viewCoordFromGlobal(offset) - view->viewCoordFromGlobal(vec2f{0, 0});
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
	SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255); // white
	SDL_RenderClear(_renderer);

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
	SDL_RenderPresent(_renderer);
	timeSinceStart += dt;
};

void CameraTrackingSystem::update(TimeDelta dt) {
	// Update center point
	const static float VELOCITY_WEIGHTING = 0.5;
	const static float CATCHUP_SPEED = 0.07;
	Entity attachee = world->registry.attachee<CameraFocus>();
	const auto& spatial = world->registry.get<SpatialData>(attachee); 
	vec2f diff = spatial.position + VELOCITY_WEIGHTING * spatial.velocity - _viewxform->viewcenter;
	_viewxform->viewcenter += CATCHUP_SPEED * diff;
}

CollisionSystem::CollisionSystem(int gridwidth)
	: gridwidth(gridwidth)
{}

bool CollisionSystem::collides(Entity one, Entity two) const {
	if (one == two) return false;
	if (!world->registry.has<SpatialData>(one)
	 || !world->registry.has<Collidable>(one)
	 || !world->registry.has<SpatialData>(two)
	 || !world->registry.has<Collidable>(two)) return false;
	const auto& spatial1 = world->registry.get<SpatialData>(one);
	const auto& spatial2 = world->registry.get<SpatialData>(two);
	const auto& collide1 = world->registry.get<Collidable>(one);
	const auto& collide2 = world->registry.get<Collidable>(two);
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

void CollisionSystem::receive(const MovedEvent &e) {
	auto oldGridCoords = getGridCoords(e.oldPos);
	auto newGridCoords = getGridCoords(e.newPos);
	if (oldGridCoords != newGridCoords) {
		mSpatialHash[oldGridCoords].erase(e.entity);
		mSpatialHash[newGridCoords].insert(e.entity);
		mGridCoords[e.entity] = newGridCoords;
	}
}

void CollisionSystem::receive(const EntityDestroyedEvent &e) {
	if (isWatching(e.entity)) {
		const auto gridCoords = mGridCoords[e.entity];
		mSpatialHash[gridCoords].erase(e.entity);
		mGridCoords.erase(e.entity);
	}
}

void CollisionSystem::receive(const CollidedEvent &e) {
	auto& collide1 = world->registry.get<Collidable>(e.one);
	auto& collide2 = world->registry.get<Collidable>(e.two);
	if (collide1.ignoreRepeatCollisions)
		collide1.addIgnored(e.two);
	if (collide2.ignoreRepeatCollisions)
		collide2.addIgnored(e.one);
	collide1.collisionsUntilDestroyed = std::max(-1, collide1.collisionsUntilDestroyed-1);
	collide2.collisionsUntilDestroyed = std::max(-1, collide2.collisionsUntilDestroyed-1);
	if (collide1.collisionsUntilDestroyed == 0)
		mToDestroy.insert(e.one);
	if (collide2.collisionsUntilDestroyed == 0)
		mToDestroy.insert(e.two);
}

void CollisionSystem::update(TimeDelta dt) {
	// add all relevant things to the collision map
	world->registry.view<SpatialData, Collidable>().each([this](auto entity, const auto &spatial, const auto &collidable) {
		if (!isWatching(entity)) {
			auto gridCoords = getGridCoords(spatial.position);
			mSpatialHash[gridCoords].insert(entity);
			mGridCoords[entity] = gridCoords;
		}
	});

	auto checkAndHandleCollisions = [this](Entity e1, Entity e2) {
		if (collides(e1, e2)) {
			assert(e1 != e2);
			auto& collide1 = world->registry.get<Collidable>(e1);
			auto& collide2 = world->registry.get<Collidable>(e2);
			if (collide1.canCollide(e2) && collide2.canCollide(e1)) {
				world->bus.publish<CollidedEvent>(e1, e2);
			}
		}
	};

	// generate all collisions
	const static vec2i dd[4] = {{0, 1}, {1, -1}, {1, 0}, {1, 1}};
	for (const auto& pair: mSpatialHash ) {
		const auto& gridCoords = pair.first;
		const auto& entities = pair.second;
		if (entities.size() == 0) continue;
		// check within the cell
		for (auto a = entities.cbegin(); a != entities.cend(); a++) {
			for (auto b = std::next(a, 1); b != entities.cend(); b++) {
				checkAndHandleCollisions(*a, *b);
			}
		}

		// check neighboring cells
		for (int i=0; i<4; i++) {
			const vec2i d = dd[i];
			auto iter = mSpatialHash.find(gridCoords + d);
			if (iter != mSpatialHash.end()) {
				for (const auto &e1 : entities) {
					for (const auto &e2 : iter->second) {
						checkAndHandleCollisions(e1, e2);
					}
				}
			}
		}
	}
	for (Entity entity : mToDestroy) {
		std::cout<<"CollisionSystem destroyed "<<entity<<std::endl;
		world->destroy(entity);
	}
	mToDestroy.clear();
}

void DestructibleSystem::receive(const DamagedEvent &e) {
	// TODO: Pay attention to damage types, source
	std::cout<<"DestructibleSystem::receive(DamagedEvent), amount="<<e.damage.amount<<std::endl;
	if (!world->registry.has<Destructible>(e.target)) return;
	auto &destructible = world->registry.get<Destructible>(e.target);
	if (!destructible.indestructible) {
		destructible.HP -= e.damage.amount;
	}
	if (destructible.HP <= 0) {
		mToDestroy.insert(e.target);
	}
}

void DestructibleSystem::receive(const HealedEvent &e) {
	std::cout<<"DestructibleSystem::receive(HealedEvent), amount="<<e.amount<<std::endl;
	if (!world->registry.has<Destructible>(e.target)) return;
	auto &destructible = world->registry.get<Destructible>(e.target);
	if (destructible.healable) {
		destructible.HP += e.amount;
	}
}

void DestructibleSystem::update(TimeDelta dt) {
	for (Entity deadEntity : mToDestroy) {
		world->destroy(deadEntity);
	}
	mToDestroy.clear();
}

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
	speedup.timeSinceUsed = 0;

	Ability accelup;
	accelup.type = Ability::Type::SelfCondition;
	accelup.cooldown = 0;
	accelup.condition = {Condition::Priority::Multiplier, Condition::Type::MOD_ACCEL, 2, 1};
	accelup.timeSinceUsed = 0;

	Ability fireball;

	ProjectileTemplate fireball_template;
	fireball_template.projectile_speed = 500;

	Collidable collidable{Collidable::Circle, 10, 1 /* collisions until destroyed */};
	fireball_template.collidable = &collidable;

	OnCollision oncollision;
	oncollision.damage = {Damage::Type::Heat, 5};
	fireball_template.oncollision = &oncollision;

	Renderable renderable;
	fireball_template.renderable = &renderable;

	TimeOut timeout;
	timeout.timeLeft = 2;
	fireball_template.timeout = &timeout;

	fireball.projectile_template = fireball_template;
	fireball.type = Ability::Type::FireProjectile;
	fireball.cooldown = 0;
	fireball.timeSinceUsed = 0;
	Condition burn = {Condition::Priority::None, Condition::Type::BURN, 10, 200000000};

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
	if (world->registry.has<Stats>(e.receiver)) {
		auto &stats = world->registry.get<Stats>(e.receiver);
		stats.dirty = true;
	}
	if (world->registry.has<Conditions>(e.receiver)) {
		auto &conditions = world->registry.get<Conditions>(e.receiver);
		conditions.push_front(e.condition);
	}
};

void ConditionSystem::update(TimeDelta dt) {
	world->registry.view<Conditions>().each([dt, this](auto entity, auto &conditions) {
		Stats* stats = world->registry.has<Stats>(entity) ? &world->registry.get<Stats>(entity) : nullptr;
		bool conditionExpired = false;
		for (auto condition = conditions.begin(); condition != conditions.end(); condition++) {
			if (condition->isExpired()) {
				condition = conditions.erase(condition);
				conditionExpired = true;
			} else {
				tickCondition(entity, *condition, std::min(condition->timeLeft, dt));
			}
		}
		if (stats) {
			if (conditionExpired || stats->dirty) {
				recalculateStats(*stats, conditions);
			}
		}
	});
}

void ConditionSystem::tickCondition(Entity entity, Condition& condition, TimeDelta dt) {
	switch (condition.type) {
	case Condition::Type::BURN: // damage
	case Condition::Type::BLEED: // damage
	case Condition::Type::POISON: // damage
		world->bus.publish<DamagedEvent>(entity, entity, Damage{Damage::Type::Puncture, condition.strength * dt});
		break;
	case Condition::Type::REGEN: // damage
		world->bus.publish<HealedEvent>(entity, entity, condition.strength * dt);
		break;
	default:
		break;
	}
	condition.timeLeft -= dt;
}

void ConditionSystem::recalculateStats(Stats &stats, Conditions &conditions) {
	memcpy(&stats.stats, &stats.basestats, sizeof(stats.stats));
	conditions.sort();
	for (const auto &condition : conditions) {
		float* var = nullptr;
		switch (condition.type) {
			case Condition::Type::MOD_SPEED: var = &stats.stats.speed; break;
			case Condition::Type::MOD_ACCEL: var = &stats.stats.accel; break;
			default: continue;
		}
		switch (condition.priority) {
			case Condition::Priority::Adder: *var += condition.strength; break;
			case Condition::Priority::Multiplier: *var *= condition.strength; break;
			default: continue;
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
		if (oncollision.damage.amount) {
			world->bus.publish<DamagedEvent>(source, target, oncollision.damage);
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
	std::cout<<"ControlSystem::receive(UseAbilityEvent)"<<std::endl;
	if (!e.ability->isOffCooldown()) return;
	switch(e.ability->type) {
	case Ability::Type::FireProjectile: {
		const ProjectileTemplate& templat = e.ability->projectile_template;
		auto &sdata = world->registry.get<SpatialData>(e.entity);
		auto velocity = e.target - sdata.position;
		velocity.truncate(templat.projectile_speed);
		Entity projectile = world->registry.create();
		world->registry.assign<SpatialData>(projectile, sdata.position, velocity);
		if (templat.renderable)
			world->registry.assign<Renderable>(projectile, *templat.renderable);
		if (templat.oncollision)
			world->registry.assign<OnCollision>(projectile, *templat.oncollision);
		if (templat.timeout)
			world->registry.assign<TimeOut>(projectile, *templat.timeout);
		if (templat.collidable) {
			auto& collidable = world->registry.assign<Collidable>(projectile, *templat.collidable);
			collidable.addIgnored(e.entity);
		}
		std::cout<<"ControlSystem created projectile, id="<<projectile<<std::endl;
		}
		break;
	case Ability::Type::SelfCondition:
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

void TimeOutSystem::update(TimeDelta dt) {
	std::vector<Entity> destroyed;
	world->registry.view<TimeOut>().each([dt, &destroyed](auto entity, auto &timeout) {
		timeout.timeLeft -= dt;
		if (timeout.isExpired()) {
			destroyed.push_back(entity);
		} 
	});
	for (auto entity : destroyed) {
		world->destroy(entity);
	}
}
