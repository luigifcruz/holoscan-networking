#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace holoscan::advanced_network::detail {

inline void append_format(std::ostringstream& stream, const std::string& format) {
  stream << format;
}

template <typename T, typename... Args>
void append_format(std::ostringstream& stream, const std::string& format, T&& value, Args&&... args) {
  const std::size_t placeholder = format.find("{}");
  if (placeholder == std::string::npos) {
    stream << format;
    stream << ' ' << std::forward<T>(value);
    ((stream << ' ' << std::forward<Args>(args)), ...);
    return;
  }

  stream << format.substr(0, placeholder);
  stream << std::forward<T>(value);
  append_format(stream, format.substr(placeholder + 2), std::forward<Args>(args)...);
}

template <typename... Args>
void log_message(const char* level, const std::string& format, Args&&... args) {
  std::ostringstream stream;
  append_format(stream, format, std::forward<Args>(args)...);
  std::clog << "[advanced_network] " << level << ": " << stream.str() << std::endl;
}

}  // namespace holoscan::advanced_network::detail

#define HOLOSCAN_LOG_TRACE(...) ::holoscan::advanced_network::detail::log_message("trace", __VA_ARGS__)
#define HOLOSCAN_LOG_DEBUG(...) ::holoscan::advanced_network::detail::log_message("debug", __VA_ARGS__)
#define HOLOSCAN_LOG_INFO(...) ::holoscan::advanced_network::detail::log_message("info", __VA_ARGS__)
#define HOLOSCAN_LOG_WARN(...) ::holoscan::advanced_network::detail::log_message("warn", __VA_ARGS__)
#define HOLOSCAN_LOG_ERROR(...) ::holoscan::advanced_network::detail::log_message("error", __VA_ARGS__)
#define HOLOSCAN_LOG_CRITICAL(...) \
  ::holoscan::advanced_network::detail::log_message("critical", __VA_ARGS__)
