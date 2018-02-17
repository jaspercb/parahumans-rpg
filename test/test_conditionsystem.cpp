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
  EXPECT_NEAR(50, stats[Stat::SPEED], 0.00001);
  EXPECT_NEAR(7,  stats[Stat::ACCEL], 0.00001);
  conditionsystem->update(100);
  conditionsystem->update(100);
  EXPECT_NEAR(5, stats[Stat::SPEED], 0.00001);
  EXPECT_NEAR(7, stats[Stat::ACCEL], 0.00001);
}

TEST_F(ConditionSystemTest, IgnoreSubjectiveTime) {
  auto entity      = world.registry.create();
  auto &stats      = world.registry.assign<Stats>(entity, 5 /* speed */, 7 /* accel */);
  world.registry.assign<Conditions>(entity);
  
  Condition timefreeze = {Condition::Type::STAT_MULTIPLY, 0.0 /* multiplier */, 5 /* seconds */, Stat::SUBJECTIVE_TIME_RATE};
  timefreeze.ignoreSubjectiveTime = true;
  world.bus.publish<ConditionEvent>(timefreeze, entity, entity);
  conditionsystem->update(0.0);
  EXPECT_NEAR(0.0, stats[Stat::SUBJECTIVE_TIME_RATE], 0.00001);
  conditionsystem->update(5.1);
  EXPECT_NEAR(1.0, stats[Stat::SUBJECTIVE_TIME_RATE], 0.00001);
}

TEST_F(ConditionSystemTest, InfiniteTimeLock) {
  auto entity      = world.registry.create();
  auto &stats      = world.registry.assign<Stats>(entity, 5 /* speed */, 7 /* accel */);
  world.registry.assign<Conditions>(entity);
  
  Condition timefreeze = {Condition::Type::STAT_MULTIPLY, 0.0 /* multiplier */, 5 /* seconds */, Stat::SUBJECTIVE_TIME_RATE};
  timefreeze.ignoreSubjectiveTime = false;
  world.bus.publish<ConditionEvent>(timefreeze, entity, entity);
  conditionsystem->update(0.0);
  EXPECT_NEAR(0.0, stats[Stat::SUBJECTIVE_TIME_RATE], 0.00001);
  for (int i=0; i<100; i++) {
    conditionsystem->update(5.1);
    EXPECT_NEAR(0.0, stats[Stat::SUBJECTIVE_TIME_RATE], 0.00001);
  }
}

TEST_F(ConditionSystemTest, TimeLockDelaysConditionExpiry) {
  auto entity      = world.registry.create();
  auto &stats      = world.registry.assign<Stats>(entity, 5 /* speed */, 7 /* accel */);
  world.registry.assign<Conditions>(entity);
  
  Condition timefreeze = {Condition::Type::STAT_MULTIPLY, 0.0 /* multiplier */, 5 /* seconds */, Stat::SUBJECTIVE_TIME_RATE};
  timefreeze.ignoreSubjectiveTime = true;
  Condition speedup = {Condition::Type::STAT_MULTIPLY, 10 /* multiplier */, 100, Stat::SPEED};
  world.bus.publish<ConditionEvent>(timefreeze, entity, entity);
  world.bus.publish<ConditionEvent>(speedup, entity, entity);
  conditionsystem->update(0.0);
  EXPECT_NEAR(50, stats[Stat::SPEED], 0.00001);
  conditionsystem->update(5.1);
  EXPECT_NEAR(50, stats[Stat::SPEED], 0.00001);
  conditionsystem->update(100);
  conditionsystem->update(100);
  EXPECT_NEAR(5, stats[Stat::SPEED], 0.00001);
}

TEST_F(ConditionSystemTest, TimeLockHaltsBleeding) {
  auto entity      = world.registry.create();
  auto &stats      = world.registry.assign<Stats>(entity, 5 /* speed */, 7 /* accel */);
  auto &destructible = world.registry.assign<Destructible>(entity, 100 /* HP */);
  world.registry.assign<Conditions>(entity);
  
  Condition timefreeze = {Condition::Type::STAT_MULTIPLY, 0.0 /* multiplier */, 5 /* seconds */, Stat::SUBJECTIVE_TIME_RATE};
  timefreeze.ignoreSubjectiveTime = true;
  Condition bleed = {Condition::Type::BLEED, 20 /* power */, 1 /* seconds */};
  world.bus.publish<ConditionEvent>(timefreeze, entity, entity);
  world.bus.publish<ConditionEvent>(bleed, entity, entity);

  conditionsystem->update(0.0);
  for (int i=0; i<6; i++) {
    EXPECT_NEAR(100, destructible.HP.value, 0.00001);
    conditionsystem->update(1.1);
  }
  // The entity shouldn't take damage from bleeding until the time lock expires.
  EXPECT_NEAR(80, destructible.HP.value, 0.00001);
  conditionsystem->update(1.0);
  EXPECT_NEAR(80, destructible.HP.value, 0.00001);
}

TEST_F(ConditionSystemTest, TimeSlowSlowsBleeding) {
  auto entity      = world.registry.create();
  auto &stats      = world.registry.assign<Stats>(entity, 5 /* speed */, 7 /* accel */);
  auto &destructible = world.registry.assign<Destructible>(entity, 100 /* HP */);
  world.registry.assign<Conditions>(entity);
  
  Condition timeslow = {Condition::Type::STAT_MULTIPLY, 0.5 /* multiplier */, 5 /* seconds */, Stat::SUBJECTIVE_TIME_RATE};
  timeslow.ignoreSubjectiveTime = true;
  Condition bleed = {Condition::Type::BLEED, 20 /* power */, 5 /* seconds */};
  world.bus.publish<ConditionEvent>(timeslow, entity, entity);
  world.bus.publish<ConditionEvent>(bleed, entity, entity);

  // HP values should be 100, 90, 80, 70, 60, 50, 30, 10, 0
  conditionsystem->update(0.0);
  for (int i=0; i<5; i++) {
    EXPECT_NEAR(100 - 10 * i, destructible.HP.value, 0.00001);
    conditionsystem->update(1.0);
  }
  for (int i=0; i<4; i++) {
    EXPECT_NEAR(std::max(0, 60 - 20 * i), destructible.HP.value, 0.00001);
    conditionsystem->update(1.0);
  }
}
}  // namespace
