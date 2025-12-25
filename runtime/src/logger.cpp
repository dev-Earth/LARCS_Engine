#include "larcs/runtime/logger.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace larcs::runtime {

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;

void Logger::Initialize(const std::string& log_file_path, LogLevel level) {
  // Create sinks
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto file_sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, true);

  // Set format
  console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
  file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");

  // Create logger with both sinks
  logger_ = std::make_shared<spdlog::logger>(
      "larcs", spdlog::sinks_init_list{console_sink, file_sink});

  // Set level
  spdlog::level::level_enum spdlog_level;
  switch (level) {
    case LogLevel::TRACE:
      spdlog_level = spdlog::level::trace;
      break;
    case LogLevel::DEBUG:
      spdlog_level = spdlog::level::debug;
      break;
    case LogLevel::INFO:
      spdlog_level = spdlog::level::info;
      break;
    case LogLevel::WARN:
      spdlog_level = spdlog::level::warn;
      break;
    case LogLevel::ERROR:
      spdlog_level = spdlog::level::err;
      break;
    case LogLevel::CRITICAL:
      spdlog_level = spdlog::level::critical;
      break;
    default:
      spdlog_level = spdlog::level::info;
  }

  logger_->set_level(spdlog_level);
  logger_->flush_on(spdlog::level::warn);

  // Register as default logger
  spdlog::set_default_logger(logger_);
}

std::shared_ptr<spdlog::logger> Logger::Get() {
  if (!logger_) {
    // Initialize with default settings if not already initialized
    Initialize("larcs.log", LogLevel::INFO);
  }
  return logger_;
}

}  // namespace larcs::runtime
