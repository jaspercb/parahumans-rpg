#include <gtest/gtest.h>
#include <memory>

#include "../src/World.hpp"
#include "../src/systems.hpp"

namespace {

class ConditionSystemTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  	world = World();
    conditionsystem = std::make_shared<ConditionSystem>();
    world.addSystem(conditionsystem);
    world.addSystem(std::make_shared<DestructibleSystem>());
  }

  World world;
  std::shared_ptr<ConditionSystem> conditionsystem;
};

TEST_F(ConditionSystemTest, BleedConditionDamages) {
  auto bleedingEntity = world.registry.create();
  auto &destructible = world.registry.assign<Destructible>(bleedingEntity, 100 /* HP */);
  world.registry.assign<Conditions>(bleedingEntity);
  Condition bleed = {Condition::Type::BLEED, 20 /* power */, 1 /* seconds */};
  world.bus.publish<ConditionEvent>(bleed, bleedingEntity, bleedingEntity);
  conditionsystem->update(0.3);
  // 20 damage/sec for 0.3 sec is 6 damage from 100 starting HP
  EXPECT_NEAR(94, destructible.HP.value, 0.00001);
}

TEST_F(ConditionSystemTest, HealConditionHeals) {
  auto regenEntity = world.registry.create();
  auto &destructible = world.registry.assign<Destructible>(regenEntity, 100 /* HP */);
  destructible.HP = 10;
  auto &conditions = world.registry.assign<Conditions>(regenEntity);
  Condition regen = {Condition::Type::REGEN, 10 /* power */, 1 /* seconds */};
  world.bus.publish<ConditionEvent>(regen, regenEntity, regenEntity);
  conditionsystem->update(0.3);
  // 10 regen/sec for 0.3 sec is 3 up from 10 starting HP
  EXPECT_NEAR(13, destructible.HP.value, 0.00001);
  conditionsystem->update(0.3);
  EXPECT_NEAR(16, destructible.HP.value, 0.00001);
  conditionsystem->update(0.3);
  EXPECT_NEAR(19, destructible.HP.value, 0.00001);
  conditionsystem->update(0.3);
  // A 1 second heal shouldn't heal for 12 just because it's ticked 4 times
  EXPECT_NEAR(20, destructible.HP.value, 0.00001);
  // After another small tick, the heal should be removed
  conditionsystem->update(0.1);
  EXPECT_NEAR(20, destructible.HP.value, 0.00001);
  EXPECT_EQ(0, conditions.size());
}

TEST_F(ConditionSystemTest, StatChangeConditionChangesStats) {
  auto entity      = world.registry.create();
  auto &stats      = world.registry.assign<Stats>(entity, 5 /* speed */, 7 /* accel */);
  world.registry.assign<Conditions>(entity);
  
  Condition speedup = {Condition::Type::STAT_MULTIPLY, 10 /* multiplier */, 100, Stat::SPEED};
  world.bus.publish<ConditionEvent>(speedup, entity, entity);
  conditionsystem->update(0.3);
  EXPECT_NEAR(50, stats.speed(), 0.00001);
  EXPECT_NEAR(7,  stats.accel(), 0.00001);
  conditionsystem->update(100);
  conditionsystem->update(100);
  EXPECT_NEAR(5, stats.speed(), 0.00001);
  EXPECT_NEAR(7, stats.accel(), 0.00001);  
}
}  // namespace
