#pragma once

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <string_view>

namespace larcs::runtime {

// Log levels
enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL };

// Logger class wrapping spdlog
class Logger {
 public:
  // Initialize logger with file output
  static void Initialize(const std::string& log_file_path, LogLevel level = LogLevel::INFO);

  // Get logger instance
  static std::shared_ptr<spdlog::logger> Get();

  // Logging convenience functions
  template <typename... Args>
  static void Trace(fmt::format_string<Args...> fmt, Args&&... args) {
    Get()->trace(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Debug(fmt::format_string<Args...> fmt, Args&&... args) {
    Get()->debug(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Info(fmt::format_string<Args...> fmt, Args&&... args) {
    Get()->info(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Warn(fmt::format_string<Args...> fmt, Args&&... args) {
    Get()->warn(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Error(fmt::format_string<Args...> fmt, Args&&... args) {
    Get()->error(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Critical(fmt::format_string<Args...> fmt, Args&&... args) {
    Get()->critical(fmt, std::forward<Args>(args)...);
  }

 private:
  static std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace larcs::runtime
