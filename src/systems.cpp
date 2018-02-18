#include "systems.hpp"
#include "World.hpp"
#include "sdlTools.hpp"
#include "abilities.hpp"

#include <unistd.h>
#include <iostream>

#include <map>

Stat StatVulnerabilityTo(Damage::Type damagetype) {
	switch(damagetype) {
	case Damage::Type::Puncture:    return Stat::VULNERABILITY_PUNCTURE;
	case Damage::Type::Slash:       return Stat::VULNERABILITY_SLASH;
	case Damage::Type::Impact:      return Stat::VULNERABILITY_IMPACT;
	case Damage::Type::Heat:        return Stat::VULNERABILITY_HEAT;
	case Damage::Type::Cold:        return Stat::VULNERABILITY_COLD;
	case Damage::Type::Electricity: return Stat::VULNERABILITY_ELECTRICITY;
	case Damage::Type::Toxin:       return Stat::VULNERABILITY_TOXIN;
	default:                        return Stat::INVALID;
	}
}

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

void RenderSystem::renderRectangularPrism(const SpatialData& sdata, const Renderable& renderable) {
	// Too slow :(
	// TODO: add scale dependency
	ViewTransform zerotransform = *_viewxform;
	zerotransform.viewcenter = {0, 0};
	vec2f fpos = _viewxform->screenCoordFromGlobal(sdata.position);
	vec2i ipos = {fpos.x, fpos.y};
	vec2f xstep = {-renderable.rectangle_x, 0};
	xstep = zerotransform.viewCoordFromGlobal(xstep);
	vec2f ystep = {0, -renderable.rectangle_y};
	ystep = zerotransform.viewCoordFromGlobal(ystep);
	float height = renderable.rectangle_z;
	Sint16 vx[4];
	Sint16 vy[4];
	vx[0] = ipos.x;
	vy[0] = ipos.y;
	vx[1] = ipos.x;
	vy[1] = ipos.y - height;
	vx[2] = ipos.x + xstep.x;
	vy[2] = ipos.y + xstep.y - height;
	vx[3] = ipos.x + xstep.x;
	vy[3] = ipos.y + xstep.y;

	filledPolygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::LIGHTGREY));
	polygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::BLACK));

	vx[2] = ipos.x + ystep.x;
	vy[2] = ipos.y + ystep.y - height;
	vx[3] = ipos.x + ystep.x;
	vy[3] = ipos.y + ystep.y;

	filledPolygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::LIGHTGREY));
	polygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::BLACK));

	vx[0] = ipos.x + xstep.x;
	vy[0] = ipos.y + xstep.y - height;
	vx[3] = ipos.x + ystep.x + xstep.x;
	vy[3] = ipos.y + ystep.y + xstep.y - height;

	filledPolygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::LIGHTGREY));
	polygonColor(_renderer, vx, vy, 4, SDL_ColortoUint32(SDL_Colors::BLACK));
}

void RenderSystem::renderEntity(Entity entity) {
	const auto& sdata = world->registry.get<SpatialData>(entity);
	const auto& renderable = world->registry.get<Renderable>(entity);
	vec2f fpos = _viewxform->screenCoordFromGlobal(sdata.position);
	vec2i ipos = {fpos.x, fpos.y};
	// hitbox
	if (world->registry.has<Collidable>(entity)) {
		const auto &collidable = world->registry.get<Collidable>(entity);
		ellipseColor(_renderer, ipos.x, ipos.y, _viewxform->scale * collidable.circle_radius, _viewxform->scale * collidable.circle_radius / 1.73, SDL_ColortoUint32(SDL_Colors::GREEN));
	}
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
			float walkspeed = stats[Stat::SPEED] * 0.065;
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
		case Renderable::Type::RectangularPrism:
		{
			renderRectangularPrism(sdata, renderable);
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

	// Draw collidable walls
	{
	auto& tiles = world->registry.get<TileLayout>();
	vec2i screenEdgeTileCoords[4];
	float screenwidth  = _viewxform->screensize.x;
	float screenheight = _viewxform->screensize.y;
	screenEdgeTileCoords[0] = tiles.getGridCoords(_viewxform->globalCoordFromScreen(vec2f{0, 0}));
	screenEdgeTileCoords[1] = tiles.getGridCoords(_viewxform->globalCoordFromScreen(vec2f{0, screenheight}));
	screenEdgeTileCoords[2] = tiles.getGridCoords(_viewxform->globalCoordFromScreen(vec2f{screenwidth, 0}));
	screenEdgeTileCoords[3] = tiles.getGridCoords(_viewxform->globalCoordFromScreen(vec2f{screenwidth, screenheight}));
	int minx = screenEdgeTileCoords[0].x;
	int miny = screenEdgeTileCoords[0].y;
	int maxx = minx;
	int maxy = miny;
	for (int i=1; i<4; i++) {
		minx = std::min(minx, screenEdgeTileCoords[i].x);
		miny = std::min(miny, screenEdgeTileCoords[i].y);
		maxx = std::max(maxx, screenEdgeTileCoords[i].x);
		maxy = std::max(maxy, screenEdgeTileCoords[i].y);
	}
	SpatialData sdata;
	Renderable renderable;
	renderable.type = Renderable::Type::RectangularPrism;
	renderable.rectangle_x = tiles.gridwidth;
	renderable.rectangle_y = tiles.gridwidth;
	renderable.rectangle_z = 0;

	for (int x = minx; x <= maxx; x++) {
		for (int y = miny; y <= maxy; y++) {
			if(tiles[vec2i{x, y}].collides) {
				sdata.position = tiles.gridwidth * vec2i{x+1, y+1};
				renderRectangularPrism(sdata, renderable);
			}
		}
	}
	}

	// distance from camera -> renderable
	std::multimap<float, Entity> drawqueue;

	world->registry.view<SpatialData, Renderable>().each([dt, this, &drawqueue](Entity entity, SpatialData &sdata, Renderable &renderable) {
		switch(renderable.type) {
			// Different renderables have a different effective z-buffer for back-to-front painting
			case Renderable::Type::RectangularPrism:
				// TODO: correct math
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
	if (!world->registry.has<CameraFocus>()) return;
	Entity attachee = world->registry.attachee<CameraFocus>();
	const auto& spatial = world->registry.get<SpatialData>(attachee);
	std::cout<<spatial.position.x << " " << spatial.position.y <<std::endl;
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

namespace {

bool CircleCollidesCircle(const SpatialData &spatial1,
                          const Collidable  &collide1,
                          const SpatialData &spatial2,
                          const Collidable  &collide2) {
	return spatial1.position.dist(spatial2.position) < collide1.circle_radius + collide2.circle_radius;
}

bool CircleCollidesRect(const SpatialData &spatial1,
                      const Collidable  &collide1,
                      const SpatialData &spatial2,
                      const Collidable  &collide2) {
	// Find the point in the rectangle closest to the circle
	float x = std::clamp(spatial1.position.x, spatial2.position.x, spatial2.position.x + collide2.rectangle_width);
	float y = std::clamp(spatial1.position.y, spatial2.position.y, spatial2.position.y + collide2.rectangle_height);
	// Check if that point is in the circle
	return vec2f{x, y}.dist(spatial1.position) < collide1.circle_radius;
}

bool RectCollidesRect(const SpatialData &spatial1,
                      const Collidable  &collide1,
                      const SpatialData &spatial2,
                      const Collidable  &collide2) {
	return (spatial1.position.x < spatial2.position.x + collide2.rectangle_width
		 && spatial2.position.x < spatial1.position.x + collide1.rectangle_width
		 && spatial1.position.y < spatial2.position.y + collide2.rectangle_height
		 && spatial2.position.y < spatial1.position.y + collide1.rectangle_height);
}
} // namespace

bool CollisionSystem::_collides(const SpatialData &spatial1,
                                const Collidable  &collide1,
                                const SpatialData &spatial2,
                                const Collidable  &collide2) {
	// TODO: 3D collisions
	switch (collide1.type) {
	case Collidable::Type::Circle:
		switch(collide2.type) {
		case Collidable::Type::Circle:
			return CircleCollidesCircle(spatial1, collide1, spatial2, collide2);
		case Collidable::Type::Rectangle:
			return CircleCollidesRect(spatial1, collide1, spatial2, collide2);
		}
		break;
	case Collidable::Type::Rectangle:
		switch(collide2.type) {
		case Collidable::Type::Rectangle:
			return RectCollidesRect(spatial1, collide1, spatial2, collide2);
		case Collidable::Type::Circle:
			return CircleCollidesRect(spatial2, collide2, spatial1, collide1);
		}
		break;
	}
	assert(false);
	return false;
}

void CollisionSystem::receive(const MovedEvent &e) {
	if (!world->registry.has<Collidable>(e.entity)) return;
	mPotentiallyMoved.insert(e.entity);

	// Resolve tile collisions.
	auto& tiles = world->registry.get<TileLayout>();
	auto& spatial = world->registry.get<SpatialData>(e.entity);
	const auto& collidable = world->registry.get<Collidable>(e.entity);

	if (isCollidingWithTile(spatial, collidable, tiles)) {
		spatial.velocity.truncate(0.01 * tiles.gridwidth);
		while (isCollidingWithTile(spatial, collidable, tiles)) {
			// Quick hacky solution: rewind the movement that created the collision.
			// TODO: Less hacky collision resolution. Maybe wait to see which collision geometry is common?
			spatial.position -= spatial.velocity;
		}
		auto nullentity = world->registry.create();
		world->bus.publish<CollidedEvent>(e.entity, nullentity);
		spatial.velocity = {0, 0};
		mPotentiallyMoved.insert(e.entity);
		world->registry.destroy(nullentity);
		std::cout<<"FIXED IT YO"<<std::endl;
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
	if (world->registry.has<Collidable>(e.one)) {
		auto& collide1 = world->registry.get<Collidable>(e.one);
		if (collide1.ignoreRepeatCollisions)
			collide1.addIgnored(e.two);
		collide1.collisionsUntilDestroyed = std::max(-1, collide1.collisionsUntilDestroyed-1);
		if (collide1.collisionsUntilDestroyed == 0)
			mToDestroy.insert(e.one);		
	}
	if (world->registry.has<Collidable>(e.two)) {
		auto& collide2 = world->registry.get<Collidable>(e.two);
		if (collide2.ignoreRepeatCollisions)
			collide2.addIgnored(e.one);
		collide2.collisionsUntilDestroyed = std::max(-1, collide2.collisionsUntilDestroyed-1);
		if (collide2.collisionsUntilDestroyed == 0)
			mToDestroy.insert(e.two);
	}
}


bool CollisionSystem::isCollidingWithTile(const SpatialData& spatial, const Collidable& collidable, TileLayout& tiles) {
	vec2i topleft, bottomright;
	switch (collidable.type) {
	case Collidable::Type::Circle:
		topleft     = tiles.getGridCoords(spatial.position - collidable.circle_radius);
		bottomright = tiles.getGridCoords(spatial.position + collidable.circle_radius);
		break;
	case Collidable::Type::Rectangle:
		topleft     = tiles.getGridCoords(spatial.position);
		bottomright = tiles.getGridCoords(spatial.position + vec2i{collidable.rectangle_width,
		                                                           collidable.rectangle_height});
		break;
	default:
		assert(false); // unhandled case;
	}
	SpatialData tilespatial;
	Collidable tilecollidable(Collidable::Type::Rectangle, 0);
	tilecollidable.rectangle_height = tiles.gridwidth;
	tilecollidable.rectangle_width  = tiles.gridwidth;
	for (int x = topleft.x; x <= bottomright.x; x++) {
		for (int y = topleft.y; y <= bottomright.y; y++) {
			tilespatial.position = {tiles.gridwidth * x, tiles.gridwidth * y};
			if (tiles[vec2i{x, y}].collides &&
			    _collides(tilespatial, tilecollidable, spatial, collidable)) return true;
		}
	}
	return false;
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
	for (auto &entity : mPotentiallyMoved) {
		auto oldGridCoords = mGridCoords[entity];
		const auto &spatial = world->registry.get<SpatialData>(entity);
		auto newGridCoords = getGridCoords(spatial.position);
		if (oldGridCoords != newGridCoords) {
			mSpatialHash[oldGridCoords].erase(entity);
			mSpatialHash[newGridCoords].insert(entity);
			mGridCoords[entity] = newGridCoords;
		}
	}
	mPotentiallyMoved.clear();

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
	Damage damage = e.damage;
	std::cout<<"DestructibleSystem::receive(DamagedEvent), amount="<<damage.amount<<std::endl;
	if (!world->registry.has<Destructible>(e.target)) return;
	auto &destructible = world->registry.get<Destructible>(e.target);
	if (world->registry.has<Stats>(e.target)) {
		auto &stats = world->registry.get<Stats>(e.target);
		damage.amount *= stats[StatVulnerabilityTo(damage.type)] * stats[Stat::VULNERABILITY];
	}
	if (!destructible.indestructible) {
		destructible.HP -= damage.amount;
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
	if (e.key.repeat == true) return;
	Control_UseAbilityEvent::Type abilityUseType;
	switch(e.type) {
	case SDL_KEYDOWN:
		abilityUseType = Control_UseAbilityEvent::Type::KeyDown; break;
	case SDL_KEYUP:
		abilityUseType = Control_UseAbilityEvent::Type::KeyUp; break;
	}
	if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
		unsigned int abilityID;
		switch(e.key.keysym.sym) {
		// TODO: actual keybindings
		case SDLK_q: abilityID = 0; break;
		case SDLK_e: abilityID = 1; break;
		default: abilityID = 9999999; break;
		}
		auto target = getMouseGlobalCoords();
		world->registry.view<Controllable>().each([this, &target, abilityUseType, abilityID](auto entity, const auto &controllable) {
			const auto &abilityData = world->registry.get<AbilityData>(entity);
			if (abilityID < abilityData.abilities.size())
				world->bus.publish<Control_UseAbilityEvent>(entity, abilityUseType, target, abilityData.abilities.at(abilityID));
		});
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
	world->registry.view<SpatialData, Controllable>().each([this, keys](auto entity, auto &sdata, auto &controllable) {
		vec2f accel = {0, 0};
		if (keys[SDL_SCANCODE_A]) {
			accel.x -= 1;
			accel.y += 1;
		}
		if (keys[SDL_SCANCODE_D]) {
			accel.x += 1;
			accel.y -= 1;
		}
		if (keys[SDL_SCANCODE_W]) {
			accel.x -= 1;
			accel.y -= 1;
		}
		if (keys[SDL_SCANCODE_S]) {
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
		TimeDelta subjectivedt = stats ? (*stats)[Stat::SUBJECTIVE_TIME_RATE] * dt : dt;
		bool conditionsDirty = false;
		for (auto condition = conditions.begin(); condition != conditions.end(); condition++) {
			tickCondition(entity, *condition, std::min(condition->timeLeft, condition->ignoreSubjectiveTime ? dt : subjectivedt));
			if (condition->isExpired()) {
				condition = conditions.erase(condition);
				conditionsDirty = true;
			}
		}
		if (stats) {
			if (conditionsDirty || stats->dirty) {
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
		switch (condition.type) {
			case Condition::Type::STAT_ADD:      stats[condition.stat] += condition.strength; break;
			case Condition::Type::STAT_MULTIPLY: stats[condition.stat] *= condition.strength; break;
			default: continue;
		}
	}
	stats.dirty = false;
}

void CollisionHandlerSystem::receive(const CollidedEvent &e) {
	std::cout<<"CollisionHandlerSystem::receive(CollidedEvent)"<<std::endl;
	handle(e.one, e.two);
	handle(e.two, e.one);
}

void CollisionHandlerSystem::handle(Entity source, Entity target) {
	// Applies source's on-collision effects to target
	if (world->registry.has<OnCollision>(source)) {
		const auto &oncollision = world->registry.get<OnCollision>(source);
		for (const auto &callback : oncollision.callbacks) {
			(*callback)(world, target);
		}
	}
}

void ControlSystem::receive(const Control_MoveAccelEvent& e) {
	SpatialData& sdata = world->registry.get<SpatialData>(e.entity);
	const Stats& stats = world->registry.get<Stats>(e.entity);
	const float accel = stats[Stat::ACCEL];
	const float speed = stats[Stat::SPEED];
	vec2f accelvector = e.accel;
	accelvector.normalize();
	accelvector *= accel;
	sdata.velocity += accelvector;
	if (sdata.velocity.x || sdata.velocity.y) {
		sdata.orientation = atan2f(sdata.velocity.y, sdata.velocity.x);
	}
	sdata.velocity *= speed / (accel + speed);
	if (!accelvector.x && !accelvector.y && sdata.velocity.length() < 1 /*epsilon value*/ ) {
		sdata.velocity *= 0;
	}
}

void ControlSystem::receive(const Control_UseAbilityEvent& e) {
	std::cout<<"ControlSystem::receive(UseAbilityEvent)"<<std::endl;
	//if (!e.ability->isUsable()) return;
	switch(e.type) {
	case Control_UseAbilityEvent::KeyDown:
		e.ability->onKeyDown(e.target); break;
	case Control_UseAbilityEvent::KeyUp:
		e.ability->onKeyUp(e.target); break;
	}
}

void AbilitySystem::update(TimeDelta dt) {
	world->registry.view<AbilityData>().each([dt, this](auto entity, auto &abilitydata) {
		Stats* stats = world->registry.has<Stats>(entity) ? &world->registry.get<Stats>(entity) : nullptr;
		TimeDelta subjectivedt = stats ? (*stats)[Stat::SUBJECTIVE_TIME_RATE] * dt : dt;
		for (auto &ability : abilitydata.abilities) {
			ability->update(subjectivedt);
		}
	});
}

void TimeOutSystem::update(TimeDelta dt) {
	std::vector<Entity> destroyed;
	world->registry.view<TimeOut>().each([this, dt, &destroyed](auto entity, auto &timeout) {
		Stats* stats = world->registry.has<Stats>(entity) ? &world->registry.get<Stats>(entity) : nullptr;
		TimeDelta subjectivedt = stats ? (*stats)[Stat::SUBJECTIVE_TIME_RATE] * dt : dt;
		timeout.timeLeft -= subjectivedt;
		if (timeout.isExpired()) {
			destroyed.push_back(entity);
		} 
	});
	for (auto entity : destroyed) {
		world->destroy(entity);
	}
}
