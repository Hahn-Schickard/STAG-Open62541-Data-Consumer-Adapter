[![pipeline status](https://git.hahn-schickard.de/software-sollutions/hasll/badges/master/pipeline.svg)](https://git.hahn-schickard.de/software-sollutions/hasll/commits/master)
[![coverage report](https://git.hahn-schickard.de/software-sollutions/hasll/badges/master/coverage.svg)](https://git.hahn-schickard.de/software-sollutions/hasll/commits/master)

<img src="docs/code_documentation/vendor-logo.png" alt="logo" width="200"/>

# HaSLL - Hahn-Schickard Logging Library for Cpp

## Brief description

HaSLL - Hahn-Schickard Logging Library for Cpp is a logger front-end with some filesystem management utilities. Mainly, this library functions as a wrapper for [SPD-Log](https://github.com/gabime/spdlog) with directory path creation. 

## Usage

```cpp
auto logger = LoggerRepository::getInstance().registerLoger("YOUR_NEW_LOGER_NAME");

logger->log(SeverityLevel::TRACE, "Welcome to a frontend for spdlog!");
logger->log(SeverityLevel::ERROR, "Some error message with arg: {}", 1);
logger->log(SeverityLevel::WARNNING, "Easy padding in numbers like {:08d}", 12);
logger->log(SeverityLevel::CRITICAL, "Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
logger->log(SeverityLevel::INFO, "Support for floats {:03.2f}", 1.23456);
logger->log(SeverityLevel::INFO, "Positional args are {1} {0}..", "too", "supported");
logger->log(SeverityLevel::DEBUG, "{:<30}", "left aligned");
```

## Logging pattern format

Logging pattern configuration is directly linked to [spdlog](https://github.com/gabime/spdlog) implementation, for reference check [Custom formating](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting), in spdlog wiki. 

Our implementation only supprots global pattern changing, and can only be set throught LoggerRepository: 

```cpp 
LoggerRepository::getInstance().configure_pattern("*** [%H:%M:%S %z] [thread %t] %v ***")
```

### Pattern flags
Pattern flags are in the form of ```%flag``` and resembles the [strftime](http://www.cplusplus.com/reference/ctime/strftime/) function:

| flag | meaning| example |
| :------ | :-------: | :-----: |
|`%v`|The actual text to log|"some user text"|
|`%t`|Thread id|"1232"|
|`%P`|Process id|"3456"|
|`%n`|Logger's name|"some logger name"
|`%l`|The log level of the message|"debug", "info", etc|
|`%L`|Short log level of the message|"D", "I", etc|
|`%a`|Abbreviated weekday name|"Thu"|
|`%A`|Full weekday name|"Thursday"|
|`%b`|Abbreviated month name|"Aug"|
|`%B`|Full month name|"August"|
|`%c`|Date and time representation|"Thu Aug 23 15:35:46 2014"|
|`%C`|Year in 2 digits|"14"|
|`%Y`|Year in 4 digits|"2014"|
|`%D` or `%x`|Short MM/DD/YY date|"08/23/14"|
|`%m`|Month 01-12|"11"|
|`%d`|Day of month 01-31|"29"|
|`%H`|Hours in 24 format  00-23|"23"|
|`%I`|Hours in 12 format  01-12|"11"|
|`%M`|Minutes 00-59|"59"|
|`%S`|Seconds 00-59|"58"|
|`%e`|Millisecond part of the current second 000-999|"678"|
|`%f`|Microsecond part of the current second 000000-999999|"056789"|
|`%F`|Nanosecond part of the current second 000000000-999999999|"256789123"|
|`%p`|AM/PM|"AM"|
|`%r`|12 hour clock|"02:55:02 pm"|
|`%R`|24-hour HH:MM time, equivalent to %H:%M|"23:55"|
|`%T` or `%X`|ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S|"23:55:59"|
|`%z`|ISO 8601 offset from UTC in timezone ([+/-]HH:MM)|"+02:00"|
|`%E`|Seconds since the epoch |"1528834770"|
|`%i`|Message sequence number (disabled by default - edit 'tweakme.h' to enable)|"1154"|
|`%%`|The % sign|"%"|
|`%+`|spdlog's default format|"[2014-10-31 23:46:59.678] [mylogger] [info] Some message"|
|`%^`|start color range (can be used only once)|"[mylogger] [info(green)] Some message"|
|`%$`|end color range (for example %^[+++]%$ %v) (can be used only once)|[+++] Some message|
|`%@`|Source file and line (use SPDLOG_TRACE(..), SPDLOG_INFO(...) etc.)|my_file.cpp:123|
|`%s`|Basename of the source file (use SPDLOG_TRACE(..), SPDLOG_INFO(...) etc.)|my_file.cpp|
|`%g`|Full path of the source file (use SPDLOG_TRACE(..), SPDLOG_INFO(...) etc.)|/some/dir/my_file.cpp|
|`%#`|Source line (use SPDLOG_TRACE(..), SPDLOG_INFO(...) etc.)|123|
|`%!`|Source function (use SPDLOG_TRACE(..), SPDLOG_INFO(...) etc. see tweakme for pretty-print)|my_func|
|`%o`|Elapsed time in milliseconds since previous message|00456|
|`%i`|Elapsed time in microseconds since previous message|00456|
|`%u`|Elapsed time in nanoseconds since previous message|11456|
|`%O`|Elapsed time in seconds since previous message|00004|

## Aligning 
Each pattern flag can can be aligned by prepending a width number(upto 64).

Use`-`(left align) or `=` (center align) to control the align side:

| align | meaning| example | result|
| :------ | :-------: | :-----: |  :-----: |
|`%<width><flag>`|Align to the right|`%8l`|"&nbsp;&nbsp;&nbsp;&nbsp;info"|
|`%-<width><flag>`|Align to the left|`%-8l`|"info&nbsp;&nbsp;&nbsp;&nbsp;"|
|`%=<width><flag>`|Align to the center|`%=8l`|"&nbsp;&nbsp;info&nbsp;&nbsp;"|


## Dependencies
This library is self containing and requires no extra dependencies
