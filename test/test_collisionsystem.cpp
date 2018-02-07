#include <gtest/gtest.h>
#include <memory>

#include "../src/World.hpp"
#include "../src/systems.hpp"

namespace {

class Receiver {
public:
  Receiver() : callcount(0) {}
  void receive(const CollidedEvent &e) {
    callcount += 1;
  }
  int callcount;
};

class EntityDestroyedReceiver {
public:
  EntityDestroyedReceiver() : callcount(0) {}
  void receive(const EntityDestroyedEvent &e) {
    callcount += 1;
  }
  int callcount;
};

class CollisionSystemTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  	world = World();
    collisionsystem = std::make_shared<CollisionSystem>(100 /* gridwidth */);
    world.addSystem(collisionsystem);
  }

  World world;
  std::shared_ptr<CollisionSystem> collisionsystem;
};

TEST_F(CollisionSystemTest, DoesNotSelfCollide) {
  auto entity = world.registry.create();
  world.registry.assign<SpatialData>(entity, vec2f{0, 0});
  world.registry.assign<Collidable>(entity, Collidable::Circle, 10);
  EXPECT_EQ(false, collisionsystem->collides(entity, entity));
}

TEST_F(CollisionSystemTest, CircleCollision) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, Collidable::Circle, 10);

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, Collidable::Circle, 20);

  auto three = world.registry.create();
  world.registry.assign<SpatialData>(three, vec2f{20, -5});
  world.registry.assign<Collidable>(three, Collidable::Circle, 10);

  EXPECT_EQ(true, collisionsystem->collides(one, two));
  EXPECT_EQ(true, collisionsystem->collides(two, three));
  EXPECT_EQ(false, collisionsystem->collides(one, three));
}

TEST_F(CollisionSystemTest, CollisionTriggersCollideEvent) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, Collidable::Circle, 10);

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, Collidable::Circle, 20);

  auto receiver = std::make_shared<Receiver>();
  world.bus.reg(receiver);

  collisionsystem->update(0.0);
  EXPECT_EQ(1, receiver->callcount);
}

TEST_F(CollisionSystemTest, NoCollisionAfterEntityDestroyed) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, Collidable::Circle, 10);

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, Collidable::Circle, 20);
  collisionsystem->update(0.0);
  world.destroy(two);

  auto receiver = std::make_shared<Receiver>();
  world.bus.reg(receiver);

  collisionsystem->update(0.0);
  EXPECT_EQ(0, receiver->callcount);
}

TEST_F(CollisionSystemTest, EntityDestroyedOnCollision) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, Collidable::Circle, 10, 1);

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, Collidable::Circle, 20);

  auto receiver = std::make_shared<EntityDestroyedReceiver>();
  world.bus.reg(receiver);

  collisionsystem->update(0.0);
  collisionsystem->update(0.0);
  EXPECT_EQ(1, receiver->callcount);
}

TEST_F(CollisionSystemTest, NoEventIfCollisionIgnored) {
  // These two entities should collide, but `one`
  // is ignoring collisions with `two`.
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, Collidable::Circle, 10);
  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, Collidable::Circle, 20);

  auto &one_collidable = world.registry.get<Collidable>(one);
  one_collidable.ignored.insert(two);

  auto receiver = std::make_shared<Receiver>();
  world.bus.reg(receiver);

  collisionsystem->update(0.0);

  EXPECT_EQ(0, receiver->callcount);
}
}  // namespace