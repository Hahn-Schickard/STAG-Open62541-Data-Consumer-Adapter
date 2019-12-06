/**
 * @file ConfigSerializer.hpp
 * @author Dovydas Girdvainis (dovydas.girdvainis@hahn-schickard.de)
 * @version 0.1
 * @date 2019-11-28
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef __LOGGER_CONFIG_JSON_SERIALIZER_
#define __LOGGER_CONFIG_JSON_SERIALIZER_

#include "Utility.hpp"

#include <string>

namespace HaSLL {

  /**
   * @brief Logger Configuration Container
   * 
   */
  class LoggerConfig {
   public:
    /**
     * @brief Construct a default Logger Config object
     * 
     */
    LoggerConfig()
        : LoggerConfig("./log",
              "logfile.log",
              "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
              SeverityLevel::INFO,
              false,
              25,
              100) {}

    /**
     * @brief Construct a new Logger Config object
     * 
     * @param logfile_path 
     * @param logfile_name 
     * @param logging_pattern 
     * @param logging_level 
     * @param log_to_std 
     * @param file_count 
     * @param file_size_in_MB 
     */
    LoggerConfig(const std::string& logfile_path,
        const std::string& logfile_name,
        const std::string& logging_pattern,
        SeverityLevel logging_level,
        bool log_to_std,
        size_t file_count,
        size_t file_size_in_MB)
        : logfile_path_(logfile_path)
        , logfile_name_(logfile_name)
        , message_pattern_(logging_pattern)
        , logging_level_(logging_level)
        , log_to_std_(log_to_std)
        , file_count_(file_count)
        , file_size_(1024 * 1024 * file_size_in_MB) {}

    /**
     * @brief Get the path to the logging life 
     * 
     * @return std::string 
     */
    std::string getLogfilePath() const {
      return logfile_path_;
    }

    /**
     * @brief Get the Logfile Name 
     * 
     * @return std::string 
     */
    std::string getLogfileName() const {
      return logfile_name_;
    }

    /**
     * @brief Get the Logging Message Pattern 
     * 
     * @return std::string 
     */
    std::string getLoggingPattern() const {
      return message_pattern_;
    }

    /**
     * @brief Get the Logging Level
     * 
     * @return SeverityLevel 
     */
    SeverityLevel getLoggingLevel() const {
      return logging_level_;
    }

    /**
     * @brief check if standard output is required
     * 
     * @return true 
     * @return false 
     */
    bool logToSTD() const {
      return log_to_std_;
    }

    /**
     * @brief Get the Maximum Logging File Count 
     * 
     * @return size_t 
     */
    size_t getFileCount() const {
      return file_count_;
    }

    /**
     * @brief Get the Maximum Logging File Size
     * 
     * @return size_t 
     */
    size_t getFileSize() const {
      return file_size_;
    }

   private:
    std::string logfile_path_;
    std::string logfile_name_;
    std::string message_pattern_;
    SeverityLevel logging_level_;
    bool log_to_std_;
    size_t file_count_;
    size_t file_size_;
  };

  /**
   * @brief Deserialize a JSON file into LoggerConfig
   * 
   * @param file_path 
   * @return LoggerConfig 
   */
  LoggerConfig deserializeConfig(const std::string& file_path);
}   // namespace HaSLL

#endif   //__LOGGER_CONFIG_JSON_SERIALIZER_