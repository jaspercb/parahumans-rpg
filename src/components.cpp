#include "components.hpp"

Collidable CircleCollidable(float radius, int collisionsUntilDestroyed, TimeDelta timeUntilCollidable) {
	Collidable ret = Collidable(Collidable::Type::Circle, radius, collisionsUntilDestroyed, timeUntilCollidable);
	return ret;
}

Collidable RectangleCollidable(float width, float height, int collisionsUntilDestroyed, TimeDelta timeUntilCollidable) {
	Collidable ret = Collidable(Collidable::Type::Rectangle, 0, collisionsUntilDestroyed, timeUntilCollidable);
	ret.rectangle_width = width;
	ret.rectangle_height = height;
	return ret;
}
