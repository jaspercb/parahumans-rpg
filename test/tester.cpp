#include <gtest/gtest.h>
#include <memory>

#include "../src/World.hpp"
#include "../src/systems.hpp"

namespace {

class CollisionSystemTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.
  World world;
  std::shared_ptr<CollisionSystem> collisionsystem;
  CollisionSystemTest() {
    // You can do set-up work for each test here.
  }

  virtual ~CollisionSystemTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  	world = World();
    collisionsystem = std::make_shared<CollisionSystem>(100 /* gridwidth */);
    world.addSystem(collisionsystem);
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
};

// Tests that the Foo::Bar() method does Abc.
TEST_F(CollisionSystemTest, DoesNotSelfCollide) {
  auto entity = world.registry.create();
  world.registry.assign<SpatialData>(entity, vec2f(0, 0));
  world.registry.assign<Collidable>(entity, Collidable::Circle, 10);
  EXPECT_EQ(false, collisionsystem->collides(entity, entity));
}

TEST_F(CollisionSystemTest, CircleCollision) {
  auto one = world.registry.create();
  world.registry.assign<SpatialData>(one, vec2f(0, 0));
  world.registry.assign<Collidable>(one, Collidable::Circle, 10);

  auto two = world.registry.create();
  world.registry.assign<SpatialData>(two, vec2f(5, -5));
  world.registry.assign<Collidable>(two, Collidable::Circle, 20);

  auto three = world.registry.create();
  world.registry.assign<SpatialData>(three, vec2f(20, -5));
  world.registry.assign<Collidable>(three, Collidable::Circle, 10);

  EXPECT_EQ(true, collisionsystem->collides(one, two));
  EXPECT_EQ(true, collisionsystem->collides(two, three));
  EXPECT_EQ(false, collisionsystem->collides(one, three));
}
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
