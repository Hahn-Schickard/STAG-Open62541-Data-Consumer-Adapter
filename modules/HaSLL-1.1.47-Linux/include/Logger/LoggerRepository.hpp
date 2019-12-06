/**
 * @file LoggerRepository.hpp
 * @author Dovydas Girdvainis (dovydas.girdvainis@hahn-schickard.de)
 * @version 0.1
 * @date 2019-11-07
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __LOGGER_REPOSITORY_HPP
#define __LOGGER_REPOSITORY_HPP

#include "ConfigSerializer.hpp"
#include "Logger.hpp"
#include "Utility.hpp"

#ifdef ASYNC_LOGGING
#include "spdlog/async.h"

#include <mutex>
#endif

#include "spdlog/spdlog.h"

#include <algorithm>
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @defgroup HaSLL Logging Frontend
 *
 * @{
 */

#ifdef DOXYGEN
/**
 * @brief Enables asynchrounus logging.
 *
 * Defaults to OFF
 *
 * Defined at compile time with -DASYNC_LOGGING or
 * add_compile_definitions(ASYNC_LOGGING) from CMakeLists.txt
 *
 */
#define ASYNC_LOGGING
#endif
/**
 * @brief Specifies the path to json configuration file
 *
 * This file is read on singelton creation. It set the following options: 
 * 1. Logfile name 
 * 2. Global logging pattern
 * 3. Global logging level
 * 4. Duplicate logging to STD stream
 * 5. Maximum logfile count
 * 6. Maximum logfile size
 *
 */
#ifndef CONFIG_PATH
#define CONFIG_PATH "exampleConfig.json"
#endif

/**
 * @brief Specifies the maximum amount of messages in the queue. Only used in
 * conjuction to ASYNC_LOGGING.
 *
 * Defaults to 1024 when ASYNC_LOGGING is ON
 *
 * Defined at compile time with -DSET_MESSAGE_QUEUE_SIZE="5000" or
 * add_compile_definitions(SET_MESSAGE_QUEUE_SIZE="2048") from CMakeLists.txt
 *
 */
#ifndef MESSAGE_QUEUE_SIZE
#define MESSAGE_QUEUE_SIZE 1024
#endif

/**
 * @brief Specifies the thread count for the logging theread pool. Only used in
 * conjuction to ASYNC_LOGGING.
 *
 * Defaults to 4 when ASYNC_LOGGING is ON
 *
 * Defined at compile time with -DSET_THREAD_COUNT="8" or
 * add_compile_definitions(SET_THREAD_COUNT="1") from CMakeLists.txt
 *
 */
#ifndef THREAD_COUNT
#define THREAD_COUNT 4
#endif

/** @} */
/** @} */

/**
 * @addtogroup HaSLL
 * @brief Hahn-Schickard Logging Library
 *
 */
namespace HaSLL {
  /**
 * @brief A singleton class that is used for logger creation, registration and
 * global configuration of all register logger severity levels and output
 * pattern.
 * @attention This singleton implementation is based on c++11 handling of satic
 * field instantiation and is thread safe.
 *
 */
  class LoggerRepository {
   public:
    /**
   * @brief Get the Instance of C++11 compliant singleton object. This calls the
   * ctor for the LoggerRepository and will try to create a logfile, specified
   * by @ref LOGFILE_NAME, which will be fou in a director,y specified by
   * @ref LOG_DIR_PATH.
   *
   * @ref LoggerRepository()
   * @ref LoggerRepository(const std::string&,size_t,size_t);
   *
   * @attention If for some reasone the logfile creation fails, the default ctor
   * will be called and a message will be logged to standard error stream.
   *
   * @return LoggerRepository&
   */
    static LoggerRepository& getInstance();
    /**
   * @brief Creates and registers a logger within the repository
   *
   * This method is thread save, if @ref ASYNC_LOGGING is activated.
   *
   * @ref Logger
   *
   * @param logger_name
   * @return std::shared_ptr<Logger>
   */
    static std::shared_ptr<Logger> registerLoger(
        const std::string& logger_name);
    /**
     * @brief Creates and registers a logger based on a given typename
     * 
     * This method is thread save, if @ref ASYNC_LOGGING is activated.
     * 
     * @tparam T 
     * @param type 
     * @return std::shared_ptr<Logger> 
     */
    template<typename T>
    static std::shared_ptr<Logger> registerTypedLoger(T type) {
      return registerLoger(getTypeName(type));
    }
    /**
     * @brief Removes a given logger from the registery
     * 
     * @param logger_name 
     */
    static void deregisterLoger(const std::string& logger_name);
    /**
   * @brief Get the Logger object from the repositroy
   *
   * This method is thread save, if @ref ASYNC_LOGGING is activated.
   *
   * @param logger_name
   * @return std::shared_ptr<Logger>
   */
    static std::shared_ptr<Logger> getLogger(const std::string& logger_name);
    /**
   * @brief Controls logging level and pattern for all of the registered loggers
   *
   * This method is thread save, if @ref ASYNC_LOGGING is activated.
   *
   * @param level
   * @param logging_pattern
   */
    static void configure(SeverityLevel level = SeverityLevel::DEBUG,
        const std::string& logging_pattern
        = "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$");
    /**
   * @brief Changes the global logging pattern
   *
   * This method is thread save, if @ref ASYNC_LOGGING is activated.
   *
   * @param logging_pattern
   */
    static void configurePattern(const std::string& logging_pattern);

    /**
     * @brief returns a demangeled string from a given typename
     * 
     * @param T type - templated type 
     *
     * @acknowledgment 
     * Kudos to Bunkar & M. Dudley from stackoverflow community for this implementation
     * @link https://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
    */
    template<typename T>
    static std::string getTypeName(T type) {
      int status;
      std::string type_name = typeid(type).name();
      char* demangled_name
          = abi::__cxa_demangle(type_name.c_str(), NULL, NULL, &status);
      if(status == 0) {
	type_name = demangled_name;
	type_name.erase(std::remove(type_name.begin(), type_name.end(), '*'),
	    type_name.end());
	std::free(demangled_name);
      }
      return type_name;
    }

    ///@cond
    LoggerRepository(LoggerRepository const&) = delete;
    LoggerRepository(LoggerRepository&&)      = delete;
    LoggerRepository& operator=(LoggerRepository const&) = delete;
    LoggerRepository& operator=(LoggerRepository&&) = delete;
    ///@endcond

    /**
   * @brief Destroy the Logger Repository object and clean up the logger
   * registery
   *
   */
    ~LoggerRepository();

   private:
    /**
   * @brief Constructs an instace of LoggerRepository object with a null sink.
   * All logged messages will be descarded
   *
   */
    LoggerRepository();

    /**
   * @brief Tries to construct an instance of LoggerRepository object with a
   * rotating file sink, named by @ref LOGFILE_NAME, located at @ref
   * LOG_DIR_PATH.
   *
   * @throws std::runtime_error on failled filesink creation.
   *
   * @param config
   */
    LoggerRepository(const LoggerConfig& config);

    /**
   * @brief Non threaded call to change the global logging pattern
   *
   * @param logging_pattern
   */
    static void configureUnguardedPattern(const std::string& logging_pattern);
    /**
   * @brief Non threaded call to change the global logging level
   *
   * @param level
   */
    static void configureUnguarded(SeverityLevel level);
    /**
   * @brief Non threaded call to get a specific logger from the repository
   *
   * @param logger_name
   * @return std::shared_ptr<Logger>
   */
    static std::shared_ptr<Logger> getUnguardedLogger(
        const std::string& logger_name);

    static std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks_;
    static std::unordered_map<std::string, std::shared_ptr<Logger>> loggers_;
    static SeverityLevel global_level_;
    static std::string previous_pattern_;
  };
}   // namespace HaSLL

#endif   //__LOGGER_REPOSITORY_HPP