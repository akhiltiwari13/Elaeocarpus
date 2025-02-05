#pragma once

#include <charconv>
#include <chrono>
#include <filesystem>
#include <format>
#include <string>
#include <thread>

// linux/POSIX specific CPU pinning.
#ifdef __linux__
#include <sched.h>
#endif

namespace elaeo::util {
/**
 * @class: CommonUtils
 * @brief: This class holds common utilities that may be required for app-dev in
 * this platform. Authors have tried to used std c++ as much as possible to keep
 * the code relatively portable.
 */
class Utils {

public:
  /**
   * @brief: This function returns microseconds elapsed since the epoch.
   */
  static auto getTimeInMicros() {
    return std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::system_clock::now().time_since_epoch()).count();
  }

  /**
   * @brief: template functions to convert strings to specified types.
   *
   * @tparam T:  type to convert the string.
   * @param strvw: a string view type to convert.
   * @param val: value of type T to hold the conversion.
   * @return
   */
  template <typename T> static bool strtoT(std::string_view strvw, T &val) {
    auto [ptr, ec] =
        std::from_chars(strvw.data(), strvw.data() + strvw.size(), val);
    return ec == std::errc();
  }

  /**
   * @brief return's today's date.
   * @return return's today's date in YYYY-MM-DD format
   */
  static std::string today() {
    return std::format("{:%Y%m%d}", std::chrono::system_clock::now());
  }

  /**
   * @brief  create a directory named today's date.
   *
   * @param folder_path 
   * @return bool
   */
  static bool createTodayFolder(const std::string &folder_path) {
    auto todayFolder = std::string(folder_path + today());
    return std::filesystem::create_directory(todayFolder);
  }

  /**
   * @brief set cpu affinity (only implemented for linux systems)
   *
   * @param cpu_id 
   */
  static void setCpuAffinity(int cpu_id) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
#endif
  }
};
} // namespace elaeo::util
