#include "systems.hpp"
#include "World.hpp"
#include "sdlTools.hpp"

#include <map>

void MovementSystem::update(TimeDelta dt) {
	world->registry.view<SpatialData>().each([dt](auto entity, auto &sdata) {
		auto oldpos = sdata.position;
		sdata.position.x += sdata.velocity.x * dt;
		sdata.position.y += sdata.velocity.y * dt;
		// events.emit<MovedEvent>(entity, oldpos, sdata.position);
	});
};

void renderOvalOffset(SDL_Renderer* renderer, View* view, vec2i pos, vec2i offset, float angle, int height, int rx, int ry, SDL_Color color) {
	offset.rotate(angle);
	vec2i screenpos = pos + view->viewCoordFromGlobal(offset);
	filledEllipseColor(renderer, screenpos.x, screenpos.y - height, rx, ry, SDL_ColortoUint32(color));
	ellipseColor(renderer, screenpos.x, screenpos.y - height, rx, ry, SDL_ColortoUint32(SDL_Colors::BLACK));
}

void RenderSystem::renderEntity(Entity entity) {
	auto sdata = world->registry.get<SpatialData>(entity);
	auto renderable = world->registry.get<Renderable>(entity);
	vec2f fpos = _view.viewCoordFromGlobal(sdata.position);
	vec2i ipos = vec2i(fpos.x, fpos.y);
	ipos.y -= sdata.z;
	switch(renderable.type) {
		case Renderable::Type::Circle: {
			filledCircleColor(_renderer, ipos.x, ipos.y, renderable.circle_radius, SDL_ColortoUint32(renderable.color));
			break;
		}
		case Renderable::Type::Line: {
			vec2i offset(renderable.line_displacement_x, renderable.line_displacement_y);
			vec2i start = ipos - offset/2.0;
			vec2i end   = ipos + offset/2.0;
			lineColor(_renderer, start.x, start.y, end.x, end.y, SDL_ColortoUint32(renderable.color));
			break;
		}
		case Renderable::Type::Person: {
			vec2f orientation(0, 1);
			float walkspeed = 8;
			orientation.rotate(sdata.orientation);
			float depthy = _view.viewCoordFromGlobal(orientation).y;
			if (depthy > 0) {
				renderOvalOffset(_renderer, &_view, ipos, {0, -12}, sdata.orientation, 18, 5, 5, renderable.color); // arms
				renderOvalOffset(_renderer, &_view, ipos, {6*cos(timeSinceStart*walkspeed), -6}, sdata.orientation, 0, 4, 4, renderable.color); // legs
			} else{
				renderOvalOffset(_renderer, &_view, ipos, {0,  12}, sdata.orientation, 18, 5, 5, renderable.color);
				renderOvalOffset(_renderer, &_view, ipos, {-6*cos(timeSinceStart*walkspeed), 6}, sdata.orientation, 0, 4, 4, renderable.color);
			}
			// head, body
			renderOvalOffset(_renderer, &_view, ipos, {0, 0}, sdata.orientation, 16, 10, 15, renderable.color); // body
			renderOvalOffset(_renderer, &_view, ipos, {0, 0}, sdata.orientation, 43, 15, 15, renderable.color); // head
			if (depthy <= 0) {
				renderOvalOffset(_renderer, &_view, ipos, {0, -12}, sdata.orientation, 18, 5, 5, renderable.color); // arms
				renderOvalOffset(_renderer, &_view, ipos, {6*cos(timeSinceStart*walkspeed), -6}, sdata.orientation, 0, 4, 4, renderable.color); // legs
			} else{
				renderOvalOffset(_renderer, &_view, ipos, {0,  12}, sdata.orientation, 18, 5, 5, renderable.color);
				renderOvalOffset(_renderer, &_view, ipos, {-6*cos(timeSinceStart*walkspeed), 6}, sdata.orientation, 0, 4, 4, renderable.color);
			}
			break;
		}
		case Renderable::Type::Cube:
		{
			break; // TOO SLOW
			float width = 50;
			vec2f xstep(-width, 0);
			xstep = _view.viewCoordFromGlobal(xstep);
			vec2f ystep(0, -width);
			ystep = _view.viewCoordFromGlobal(ystep);
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
}

void RenderSystem::update(TimeDelta dt) {
	// Fill screen with white
	SDL_Renderer* renderer = _renderer;
	const View* view = &_view;
	SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255); // white
	SDL_RenderClear(renderer);

	// distance from camera -> renderable
	std::multimap<float, Entity> drawqueue;

	world->registry.view<SpatialData, Renderable>().each([dt, &view, &drawqueue](Entity entity, SpatialData &sdata, Renderable &renderable) {
		switch(renderable.type) {
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
{
	// TODO: register as listener for ThingMoved events
}

bool CollisionSystem::collides(Entity one, Entity two) {
	if (!world->registry.has<SpatialData>(one)
	 || !world->registry.has<Collidable>(one)
	 || !world->registry.has<SpatialData>(two)
	 || !world->registry.has<Collidable>(two)) return false;
	auto spatial1 = world->registry.get<SpatialData>(one);
	auto spatial2 = world->registry.get<SpatialData>(two);
	auto collide1 = world->registry.get<Collidable>(one);
	auto collide2 = world->registry.get<Collidable>(two);

	switch (collide1.type) {
	case Collidable::Type::Circle: {
		switch(collide2.type) {
		case Collidable::Type::Circle: {
			// TODO: 3D collisions
			return spatial1.position.dist(spatial2.position) < collide1.circle_radius + collide2.circle_radius;
			break;
		}
		}
		break;
	}
	default:
		assert(false);
	}
}

// TODO: listen to EntityCreated and EntityDestroyed?

void CollisionSystem::receive(MovedEvent e) {
	auto oldPos = e.oldPos;
	auto oldGridCoords = getGridCoords(oldPos);
	auto newGridCoords = getGridCoords(e.newPos);
	if (oldGridCoords != newGridCoords) {
		spatial_hash[oldGridCoords].erase(e.entity);
		spatial_hash[newGridCoords].insert(e.entity);
	}
	static vec2i ddpos[5] = {{0,0},{1,0},{1,-1},{0,-1},{-1,-1}};
	for (int i=0; i<5; i++) {
		auto dpos = ddpos[i];
		auto iter = spatial_hash.find(newGridCoords + dpos);
		if (iter != spatial_hash.end()) {
			for (const auto &entity : iter->second) {
				if ((e.entity != entity) && collides(e.entity, entity)) {
					// events.emit<CollidedEvent>(e.entity, entity);
				}
			}
		}
	}
}

void CollisionSystem::update(TimeDelta dt) {
	// fuck this nobody liked it anyway 
}
