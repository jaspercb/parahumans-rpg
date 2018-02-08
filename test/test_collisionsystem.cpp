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
  // is ignoring collisions with `two`, so they don't.
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, Collidable::Circle, 10);
  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, Collidable::Circle, 20);

  auto &one_collidable = world.registry.get<Collidable>(one);
  one_collidable.addIgnored(two);

  auto receiver = std::make_shared<Receiver>();
  world.bus.reg(receiver);

  collisionsystem->update(0.0);

  EXPECT_EQ(0, receiver->callcount);
}

TEST_F(CollisionSystemTest, TryToBruteForceCauseSegfault) {
  /* entt was segfaulting on certain entity destroy operations. I think I
     fixed it, but this test is here to check for regressions.
  */
  std::vector<void*> ptrs;
  auto ent = world.registry.create();
  for (int i=0; i<100; i++) {
    auto one = world.registry.create();
    world.registry.assign<SpatialData>(one, vec2f{0, 0+200*i});
    auto &collide = world.registry.assign<Collidable>(one, Collidable::Circle, 10);
    collide.addIgnored(ent);
    world.registry.assign<Renderable>(one);
    void* ptr = malloc(200 * 10);
    ptrs.push_back(ptr);
  }
  std::vector<Entity> toDestroy;
  world.registry.view<SpatialData>().each(
    [this, &toDestroy, &ptrs](auto entity, const auto &spatial) {
    toDestroy.push_back(entity);
    void* ptr = malloc(220);
    ptrs.push_back(ptr);
  });
  for (auto i : toDestroy) {
    world.destroy(i);
    void* ptr = malloc(220);
    ptrs.push_back(ptr);
  }

  collisionsystem->update(0.0);

  for (auto &i : ptrs) {
    free(i);
  }
}
}  // namespace
