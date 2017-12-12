#include "systems.hpp"
#include "components.hpp"
#include "sdlTools.hpp"

#include <map>

using namespace entityx;

void MovementSystem::update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) {
	es.each<SpatialData>([dt](Entity entity, SpatialData &sdata) {
		sdata.position.x += sdata.velocity.x * dt;
		sdata.position.y += sdata.velocity.y * dt;
	});
};

void renderOvalOffset(SDL_Renderer* renderer, View* view, vec2i pos, vec2i offset, float angle, int height, int rx, int ry, SDL_Color color) {
	offset.rotate(angle);
	vec2i screenpos = pos + view->viewCoordFromGlobal(offset);
	filledEllipseColor(renderer, screenpos.x, screenpos.y - height, rx, ry, SDL_ColortoUint32(color));
	ellipseColor(renderer, screenpos.x, screenpos.y - height, rx, ry, SDL_ColortoUint32(SDL_Colors::BLACK));
}

void RenderSystem::renderEntity(entityx::Entity entity) {
	const auto renderable = *entity.component<Renderable>();
	const auto sdata = *entity.component<SpatialData>();
	vec2f fpos = _view.viewCoordFromGlobal(sdata.position);
	vec2i ipos = vec2i(fpos.x, fpos.y);
	ipos.y -= sdata.z;
	switch(renderable.type) {
		case Renderable::Type::Circle : {
			filledCircleColor(_renderer, ipos.x, ipos.y, renderable.circle_radius, SDL_ColortoUint32(renderable.color));
			break;
		}
		case Renderable::Type::Line : {
			vec2i offset(renderable.line_displacement_x, renderable.line_displacement_y);
			vec2i start = ipos - offset/2.0;
			vec2i end   = ipos + offset/2.0;
			lineColor(_renderer, start.x, start.y, end.x, end.y, SDL_ColortoUint32(renderable.color));
			break;
		}
		case Renderable::Type::Person : {
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
		case Renderable::Type::Cube : {
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

void RenderSystem::update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) {
	// Fill screen with white
	SDL_Renderer* renderer = _renderer;
	const View* view = &_view;
	SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255); // white
	SDL_RenderClear(renderer);

	// distance from camera -> renderable
	std::multimap<float, Entity> drawqueue;

	es.each<SpatialData, Renderable>([dt, &view, &drawqueue](Entity entity, SpatialData &sdata, Renderable &renderable) {
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
