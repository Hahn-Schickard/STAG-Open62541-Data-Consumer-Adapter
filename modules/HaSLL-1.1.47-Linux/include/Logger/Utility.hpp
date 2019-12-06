/**
 * @file Utility.hpp
 * @author Dovydas Girdvainis (dovydas.girdvainis@hahn-schickard.de)
 * @version 0.1
 * @date 2019-11-28
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __LOGGER_UTILITY_FUNCTIONS_
#define __LOGGER_UTILITY_FUNCTIONS_

#include "spdlog/common.h"

/**
 * @addtogroup HaSLL
 * @brief Logging Frontend
 *
 */
namespace HaSLL {
  /**
 * @enum SeverityLevelEnum
 * @brief Seretiy logging levels in hierarchical order.
 *
 * Heighest order takes presidence. For example, if logging level is set to
 * INFO, DEBUG and TRACE messages will not be show, whereas INFO, WARNNING,
 * ERROR and CRITICAL will be.
 *
 */
  typedef enum SeverityLevelEnum {
    TRACE,    /*!< Severity level 0 */
    DEBUG,    /*!< Severity level 1 */
    INFO,     /*!< Severity level 2 */
    WARNNING, /*!< Severity level 3 */
    ERROR,    /*!< Severity level 4 */
    CRITICAL, /*!< Severity level 5 */
  } SeverityLevel;

  /**
   * @brief converts the generic SeverityLevel into spdlog specific enum value
   * 
   * @param level 
   * @return spdlog::level::level_enum 
   */
  inline spdlog::level::level_enum getSPDLogLevel(SeverityLevel level) {
    switch(level) {
      case SeverityLevel::CRITICAL:
	return spdlog::level::critical;
      case SeverityLevel::DEBUG:
	return spdlog::level::debug;
      case SeverityLevel::ERROR:
	return spdlog::level::err;
      case SeverityLevel::INFO:
	return spdlog::level::info;
      case SeverityLevel::TRACE:
	return spdlog::level::trace;
      case SeverityLevel::WARNNING:
	return spdlog::level::warn;
      default:
	return spdlog::level::off;
    }
  }

}   // namespace HaSLL

#endif   //__LOGGER_UTILITY_FUNCTIONS_