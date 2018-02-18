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
  world.registry.assign<Collidable>(entity, CircleCollidable(10));
  EXPECT_EQ(false, collisionsystem->collides(entity, entity));
}

TEST_F(CollisionSystemTest, CircleCollision) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, CircleCollidable(10));

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, CircleCollidable(20));

  auto three = world.registry.create();
  world.registry.assign<SpatialData>(three, vec2f{20, -5});
  world.registry.assign<Collidable>(three, CircleCollidable(10));

  EXPECT_EQ(true, collisionsystem->collides(one, two));
  EXPECT_EQ(true, collisionsystem->collides(two, three));
  EXPECT_EQ(false, collisionsystem->collides(one, three));
}

TEST_F(CollisionSystemTest, RectRectCollision) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, RectangleCollidable(20, 20));

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, RectangleCollidable(20, 20));

  EXPECT_EQ(true, collisionsystem->collides(one, two));
}

TEST_F(CollisionSystemTest, CircleRectCollision) {
  auto circle1 = world.registry.create();
  world.registry.assign<SpatialData>(circle1, vec2f{2, 2});
  world.registry.assign<Collidable>(circle1, CircleCollidable(1));

  auto circle2 = world.registry.create();
  world.registry.assign<SpatialData>(circle2, vec2f{5, 1});
  world.registry.assign<Collidable>(circle2, CircleCollidable(0.2));

  auto circle3 = world.registry.create();
  world.registry.assign<SpatialData>(circle3, vec2f{50, 50});
  world.registry.assign<Collidable>(circle3, CircleCollidable(100));

  auto rect1 = world.registry.create();
  world.registry.assign<SpatialData>(rect1, vec2f{0, 0});
  world.registry.assign<Collidable>(rect1, RectangleCollidable(2, 2));

  auto rect2 = world.registry.create();
  world.registry.assign<SpatialData>(rect2, vec2f{4, 0});
  world.registry.assign<Collidable>(rect2, RectangleCollidable(2, 2));

  EXPECT_EQ(true,  collisionsystem->collides(rect1, circle1));
  EXPECT_EQ(false, collisionsystem->collides(rect1, circle2));
  EXPECT_EQ(false, collisionsystem->collides(rect2, circle1));
  EXPECT_EQ(true,  collisionsystem->collides(rect2, circle2));
  EXPECT_EQ(true,  collisionsystem->collides(rect1, circle3));
  EXPECT_EQ(true,  collisionsystem->collides(rect2, circle3));
}


TEST_F(CollisionSystemTest, CollisionTriggersCollideEvent) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, CircleCollidable(10));

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, CircleCollidable(20));

  auto receiver = std::make_shared<Receiver>();
  world.bus.reg(receiver);

  collisionsystem->update(0.0);
  EXPECT_EQ(1, receiver->callcount);
}


TEST_F(CollisionSystemTest, NoCollisionAfterEntityDestroyed) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f{0, 0});
  world.registry.assign<Collidable>(one, CircleCollidable(10));

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, CircleCollidable(20));
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
  world.registry.assign<Collidable>(one, CircleCollidable(10, 1));

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, CircleCollidable(20));

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
  world.registry.assign<Collidable>(one, CircleCollidable(10));
  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f{5, -5});
  world.registry.assign<Collidable>(two, CircleCollidable(20));

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
    auto &collide = world.registry.assign<Collidable>(one, CircleCollidable(10));
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
