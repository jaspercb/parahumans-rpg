#include <gtest/gtest.h>
#include <memory>

#include "../src/World.hpp"
#include "../src/systems.hpp"

namespace {

class DestructibleSystemTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  	world = World();
    world.addSystem(std::make_shared<DestructibleSystem>());
  }

  World world;
};

TEST_F(DestructibleSystemTest, VulnerabilityAdjustsDamage) {
  auto entity = world.registry.create();
  auto &destructible = world.registry.assign<Destructible>(entity, 100 /* HP */);
  auto &stats = world.registry.assign<Stats>(entity, 0, 0);
  stats[Stat::VULNERABILITY_PUNCTURE] = 2.0;
  stats[Stat::VULNERABILITY]          = 1.0;
  world.bus.publish<DamagedEvent>(entity, entity, Damage{Damage::Type::Puncture, 10});
  EXPECT_NEAR(80, destructible.HP.value, 0.00001);

  stats[Stat::VULNERABILITY_PUNCTURE] = 2.0;
  stats[Stat::VULNERABILITY]          = 3.0;
  world.bus.publish<DamagedEvent>(entity, entity, Damage{Damage::Type::Puncture, 10});
  EXPECT_NEAR(20, destructible.HP.value, 0.00001);
}
}  // namespace
