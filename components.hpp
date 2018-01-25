#pragma once

#include "sdlTools.hpp"

struct SpatialData {
	SpatialData(vec2f position={0, 0}, vec2f velocity={0, 0}, float z=0, float orientation=0) :
		position(position),
		velocity(velocity),
		z(z),
		orientation(orientation) {};

	vec2f orientationToVec() { return vec2f(cos(orientation), sin(orientation)); };

	vec2f position, velocity;
	float z, orientation;
};

struct Renderable {
	enum Type {
		Circle,
		Line,
		Person,
		Cube
	};

	Renderable(Type type=Circle, SDL_Color color=SDL_Colors::RED, float radius=10, int x=0, int y=0) :
		type(type),
		color(color),
		line_thickness(radius),
		line_displacement_x(x),
		line_displacement_y(y)
	{}

	Type type;
	SDL_Color color;
	union {
		// Circle
		struct {
			float circle_radius;
		};
		// Line
		struct {
			float line_thickness;
			int line_displacement_x;
			int line_displacement_y;
		};
		struct {
			int cube_x, cube_y, cube_z;
		};
	};
};

struct Collidable {
	enum Type {
		Circle
	};
	Type type;
	union {
		struct {
			float circle_radius;
		};
	};
};

struct Destructible {
	Destructible(HPType maxHP, bool indestructible=false)
		: HP(maxHP, 0, maxHP), indestructible(indestructible)
		{}
	BoundedQuantity<HPType> HP;
	bool indestructible;
};

struct Controllable {
	Controllable() {}
};
