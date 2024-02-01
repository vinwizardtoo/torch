#include <gtest/gtest.h>

#include <c10/util/irange.h>
#include <c10/xpu/XPUCachingAllocator.h>

#define ASSERT_EQ_XPU(X, Y) \
  {                         \
    bool _isEQ = X == Y;    \
    ASSERT_TRUE(_isEQ);     \
  }

bool has_xpu() {
  return c10::xpu::device_count() > 0;
}

TEST(XPUCachingAllocatorTest, GetXPUAllocator) {
  auto* allocator = c10::xpu::XPUCachingAllocator::get();

  auto _500mb = 500 * 1024 * 1024;
  auto buffer = allocator->allocate(_500mb);
  ASSERT_TRUE(buffer.get());

  auto* xpu_allocator = c10::GetAllocator(buffer.device().type());
  ASSERT_EQ_XPU(allocator, xpu_allocator);
}

TEST(XPUCachingAllocatorTest, DeviceCachingAllocate) {
  auto* allocator = c10::xpu::XPUCachingAllocator::get();
  auto _10mb = 10 * 1024 * 1024;
  auto buffer = allocator->allocate(_10mb);
  void* ptr0 = buffer.get();
  // tmp is not allocated via device caching allocator.
  void* tmp = sycl::aligned_alloc_device(
      512, _10mb, c10::xpu::get_raw_device(0), c10::xpu::get_device_context());
  void* ptr1 = c10::xpu::XPUCachingAllocator::raw_alloc(_10mb);
  // ptr0 and ptr1 should be on the same block, and ptr1 is the next block of
  // ptr0, like [ptr0, ptr1]. So the offset of their pointers should equal to
  // ptr0's size (10M).
  auto diff = static_cast<char*>(ptr1) - static_cast<char*>(ptr0);
  ASSERT_EQ_XPU(diff, _10mb);
  c10::xpu::XPUCachingAllocator::raw_delete(ptr1);
  sycl::free(tmp, c10::xpu::get_device_context());
}

TEST(XPUCachingAllocatorTest, AllocateMemory) {
  c10::xpu::XPUCachingAllocator::emptyCache();
  auto* allocator = c10::xpu::XPUCachingAllocator::get();
  auto _10mb = 10 * 1024 * 1024;
  auto buffer = allocator->allocate(_10mb);
  auto* deviceData = static_cast<int*>(buffer.get());

  constexpr int numel = 1024;
  int hostData[numel];
  for (const auto i : c10::irange(numel)) {
    hostData[i] = i;
  }

  auto stream = c10::xpu::getStreamFromPool();
  // H2D
  stream.queue().memcpy(deviceData, hostData, sizeof(int) * numel);
  c10::xpu::syncStreamsOnDevice();

  for (const auto i : c10::irange(numel)) {
    hostData[i] = 0;
  }

  // D2H
  stream.queue().memcpy(hostData, deviceData, sizeof(int) * numel);
  c10::xpu::syncStreamsOnDevice();

  for (const auto i : c10::irange(numel)) {
    ASSERT_EQ_XPU(hostData[i], i);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  auto device = c10::xpu::device_count();
  if (device <= 0) {
    return 0;
  }
  c10::xpu::XPUCachingAllocator::init(device);
  return RUN_ALL_TESTS();
}
