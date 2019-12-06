/**
 * @file Logger.hpp
 * @author Dovydas Girdvainis (dovydas.girdvainis@hahn-schickard.de)
 * @version 0.1
 * @date 2019-11-07
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __LOGGER_HPP
#define __LOGGER_HPP

#include "Utility.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

#ifdef ASYNC_LOGGING
#include "spdlog/async.h"
#endif

#include <memory>
#include <string>
#include <vector>

namespace HaSLL {

  /**
 * @brief Logging wrapper
 *
 */
  class Logger {
   public:
    /**
   * @brief Construct a new Logger object
   * @attention This ctor should only be called by @ref
   * LoggerRepository
   *
   * @param sinks
   * @param logger_name
   */
    Logger(std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks,
        const std::string& logger_name);
    /**
   * @brief Configures the logging level for this instance
   *
   * @param level
   */
    void configure(SeverityLevel level = SeverityLevel::DEBUG);

    /**
   * @brief Logs a message to SeverityLevel.
   * Example use:
   * @code
   * logger->log(SeverityLevel::TRACE, "Welcome to a frontend for spdlog!");
   *
   * @param level
   * @param message
   */
    void log(SeverityLevel level, spdlog::string_view_t message);

    /**
   * @brief Logs a message to SeverityLevel with variadic arguments.
   * Example use:
   * @code
   * logger->log(SeverityLevel::ERROR, "Some error message with arg: {}", 1);
   * logger->log(SeverityLevel::WARNNING, "Easy padding in numbers like {:08d}",
   *             12);
   * logger->log(SeverityLevel::CRITICAL,
   *             "Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}",
   *             42);
   * logger->log(SeverityLevel::INFO, "Support for floats {:03.2f}", 1.23456);
   * logger->log(SeverityLevel::INFO, "Positional args are {1} {0}..", "too",
   *             "supported");
   * logger->log(SeverityLevel::DEBUG, "{:<30}", "left aligned");
   * @endcode
   *
   * @tparam Args
   * @param level
   * @param message
   * @param args
   */
    template<typename... Args>
    void log(SeverityLevel level,
        spdlog::string_view_t message,
        const Args&... args) {
      logger_->log(getSPDLogLevel(level), message, args...);
    }
    /**
     * @brief Returns the name of this logger
     * 
     * @return std::string 
     */
    std::string getName();

   private:
    std::shared_ptr<spdlog::logger> logger_;
    std::string name_;
  };
}   // namespace HaSLL
/** @} */
#endif   //__LOGGER_HPP