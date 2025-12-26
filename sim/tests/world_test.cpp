#include "larcs/sim/world.hpp"

#include <gtest/gtest.h>

TEST(WorldTest, InitializeAndShutdown) {
  larcs::sim::World world;
  ASSERT_TRUE(world.initialize());
  EXPECT_EQ(world.get_sim_time(), 0.0);
  world.shutdown();
}

TEST(WorldTest, Step) {
  larcs::sim::World world;
  ASSERT_TRUE(world.initialize());

  world.step(0.01);
  EXPECT_DOUBLE_EQ(world.get_sim_time(), 0.01);

  world.step(0.01);
  EXPECT_DOUBLE_EQ(world.get_sim_time(), 0.02);

  world.shutdown();
}

TEST(WorldTest, TimeScale) {
  larcs::sim::World world;
  ASSERT_TRUE(world.initialize());

  world.set_time_scale(10.0);
  EXPECT_DOUBLE_EQ(world.get_time_scale(), 10.0);

  // Step with 10x time scale
  world.step(0.01);
  EXPECT_DOUBLE_EQ(world.get_sim_time(), 0.1);  // 0.01 * 10

  world.shutdown();
}

TEST(WorldTest, MultipleSteps) {
  larcs::sim::World world;
  ASSERT_TRUE(world.initialize());

  for (int i = 0; i < 100; ++i) {
    world.step(0.004167);  // 240 Hz
  }

  EXPECT_NEAR(world.get_sim_time(), 0.4167, 0.001);

  world.shutdown();
}
