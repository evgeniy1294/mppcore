#include <cstdint>

namespace mpp::core::__private {
  volatile std::uint32_t systime = 0u;

  std::uint32_t GetTick() { return systime; }
  void IncTick() { systime++; }
  void ResetTick() { systime = 0u; }
}

