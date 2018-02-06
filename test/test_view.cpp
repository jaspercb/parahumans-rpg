#include <gtest/gtest.h>
#include <memory>

#include "../src/systems.hpp"

namespace {
class ViewTransformTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  	view = ViewTransform();
    view.scale = 1;
    view.viewcenter = {0, 0};
  }

  ViewTransform view;
};

TEST_F(ViewTransformTest, ZeroSanityCheck) {
  vec2f zero = {0, 0};
  EXPECT_EQ(zero, view.viewCoordFromGlobal(zero));
}

TEST_F(ViewTransformTest, InverseTransform) {
  static vec2f vectors[3] = {{1, 3}, {100, 700}, {-200, 1400}};
  for (auto &a : vectors) {
    for (auto &b : vectors) {
      for (int scale=-1; scale<5; scale+=2) {
        view.viewcenter = a;
        view.scale = scale;
        vec2f output = view.globalCoordFromView(view.viewCoordFromGlobal(b));
        EXPECT_NEAR(b.x, output.x, 0.001);
        EXPECT_NEAR(b.y, output.y, 0.001);
      }
    }
  }
}
}  // namespace
