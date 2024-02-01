#include <gtest/gtest.h>

#include <c10/xpu/XPUGuard.h>

#define ASSERT_EQ_XPU(X, Y) \
  {                         \
    bool _isEQ = X == Y;    \
    ASSERT_TRUE(_isEQ);     \
  }

bool has_xpu() {
  return c10::xpu::device_count() > 0;
}

TEST(XPUGuardTest, GuardBehavior) {
  if (!has_xpu()) {
    return;
  }

  ASSERT_EQ_XPU(c10::xpu::current_device(), 0);
  std::vector<c10::xpu::XPUStream> streams0 = {
      c10::xpu::getStreamFromPool(), c10::xpu::getStreamFromPool(true)};
  ASSERT_EQ_XPU(streams0[0].device_index(), 0);
  ASSERT_EQ_XPU(streams0[1].device_index(), 0);
  c10::xpu::setCurrentXPUStream(streams0[0]);
  ASSERT_EQ_XPU(c10::xpu::getCurrentXPUStream(), streams0[0]);

  if (c10::xpu::device_count() <= 1) {
    return;
  }

  // Test for XPUGuard.
  std::vector<c10::xpu::XPUStream> streams1;
  {
    c10::xpu::XPUGuard device_guard(1);
    streams1.push_back(c10::xpu::getStreamFromPool());
    streams1.push_back(c10::xpu::getStreamFromPool());
  }

  ASSERT_EQ_XPU(streams1[0].device_index(), 1);
  ASSERT_EQ_XPU(streams1[1].device_index(), 1);
  ASSERT_EQ_XPU(c10::xpu::current_device(), 0);

  // Test for OptionalXPUGuard.
  {
    c10::xpu::OptionalXPUGuard device_guard;
    ASSERT_EQ_XPU(device_guard.current_device().has_value(), false);
    device_guard.set_index(1);
    ASSERT_EQ_XPU(c10::xpu::current_device(), 1);
  }

  ASSERT_EQ_XPU(c10::xpu::current_device(), 0);
  c10::xpu::setCurrentXPUStream(streams1[0]);

  // Test for XPUStreamGuard.
  {
    c10::xpu::XPUStreamGuard guard(streams1[1]);
    ASSERT_EQ_XPU(guard.current_device(), at::Device(at::kXPU, 1));
    ASSERT_EQ_XPU(c10::xpu::current_device(), 1);
    ASSERT_EQ_XPU(c10::xpu::getCurrentXPUStream(1), streams1[1]);
  }

  ASSERT_EQ_XPU(c10::xpu::current_device(), 0);
  ASSERT_EQ_XPU(c10::xpu::getCurrentXPUStream(), streams0[0]);
  ASSERT_EQ_XPU(c10::xpu::getCurrentXPUStream(1), streams1[0]);

  // Test for OptionalXPUStreamGuard.
  {
    c10::xpu::OptionalXPUStreamGuard guard;
    ASSERT_EQ_XPU(guard.current_stream().has_value(), false);
    guard.reset_stream(streams1[1]);
    ASSERT_EQ_XPU(guard.current_stream(), streams1[1]);
    ASSERT_EQ_XPU(c10::xpu::current_device(), 1);
    ASSERT_EQ_XPU(c10::xpu::getCurrentXPUStream(1), streams1[1]);
  }

  ASSERT_EQ_XPU(c10::xpu::current_device(), 0);
  ASSERT_EQ_XPU(c10::xpu::getCurrentXPUStream(), streams0[0]);
  ASSERT_EQ_XPU(c10::xpu::getCurrentXPUStream(1), streams1[0]);
}
