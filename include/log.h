#ifndef __LOG_H
#define __LOG_H

#include <bits/types/time_t.h>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <list>
#include <locale>
#include <memory>
#include <ostream>
#include <sstream>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "mutex.h"
#include "singleton.h"

#define LOG_EVENT(logger)                                                      \
  (LogEvent::ptr(new LogEvent(logger, LogLevel::INFO, 0, time(NULL), 22, 33)))

#define LOG_LEVEL(level, logger) LogEventWrapper(LOG_EVENT(logger)).getSS()

#define LOG_DEBUG(logger) LOG_LEVEL(LogLevel::DEBUG, logger)
#define LOG_INFO(logger) LOG_LEVEL(LogLevel::INFO, logger)
#define LOG_WARN(logger) LOG_LEVEL(LogLevel::WARN, logger)
#define LOG_ERROR(logger) LOG_LEVEL(LogLevel::ERROR, logger)
#define LOG_FATAL(logger) LOG_LEVEL(LogLevel::FATAL, logger)

#define LOG_ROOT() mine::g_log_manager::getInstance()->getRoot()
#define LOG_NAME(name) mine::g_log_manager::getInstance()->getLogger(name)

// enum class LogLevel { Debug, Info, Warn, Error, Fatal,
// };
namespace mine {

class Logger;

class LogLevel {
public:
  enum Level { UNKOWN, DEBUG, INFO, WARN, ERROR, FATAL };

  // convert LogLevel to string
  static std::string toString(LogLevel::Level level);

  // convert hint string to LogLevel
  static LogLevel::Level fromString(std::string &hint);

private:
};

// log information
class LogEvent {
public:
  typedef std::shared_ptr<LogEvent> ptr;

  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
           uint64_t elapse, std::time_t timestamp, uint32_t threadid,
           uint32_t fiberid);

  std::string getFilename() const { return m_filename; }
  uint32_t getLineNumber() const { return m_lineno; }
  uint64_t getElapse() const { return m_elapse; }
  std::time_t getTimestamp() const { return m_timestamp; }
  uint32_t getThreadId() const { return m_threadid; }
  uint32_t getFiberId() const { return m_fiberid; }
  std::shared_ptr<Logger> getLogger() const { return m_logger; }
  LogLevel::Level getLevel() const { return m_level; }
  std::string getContent() const { return m_ss.str(); }
  std::stringstream &getSS() { return m_ss; }

private:
  std::string m_filename;  // the name of the log file
  uint32_t m_lineno;       // the line number of the log item
  uint64_t m_elapse;       // time passed since the log system started
  std::time_t m_timestamp; // timestamp of the log item
  uint32_t m_threadid;     // the id of the thread who writes this log item
  uint32_t m_fiberid;
  std::shared_ptr<Logger> m_logger;
  LogLevel::Level m_level;
  std::stringstream
      m_ss; // utilize string stream to store the contents of the log event
};

class LogEventWrapper {
public:
  LogEventWrapper(LogEvent::ptr event);
  ~LogEventWrapper();

  std::stringstream &getSS();

private:
  LogEvent::ptr m_event;
};

class LogFormatter {
public:
  typedef std::shared_ptr<LogFormatter> ptr;

  /**
   * @brief,
   *  %m message,
   *  %p log-level,
   *  %r elapse,
   *  %c log-name,
   *  %t thread-id,
   *  %n new-line,
   *  %d timestamp,
   *  %f file-name,
   *  %l line-number,
   *  %T table,
   *  %F fiber-id,
   * @param pattern
   */
  LogFormatter(const std::string &pattern);

  // parse the pattern and add proper format items
  // pattern format: ...%x...$x{format}...%%...
  void init();

  std::string format(std::stringstream &ss, LogLevel::Level level,
                     LogEvent::ptr event);
  std::ostream &format(std::ostream &os, LogLevel::Level level,
                       LogEvent::ptr event);

  bool error() const { return m_error; }

public:
  class LogFormatterItem {
  public:
    typedef std::shared_ptr<LogFormatterItem> ptr;
    virtual ~LogFormatterItem();

    virtual void format(std::ostream &os, LogLevel::Level level,
                        LogEvent::ptr event) = 0;
    virtual void format(std::stringstream &ss, LogLevel::Level level,
                        LogEvent::ptr event);
  };

private:
  std::string m_pattern;
  std::vector<LogFormatterItem::ptr> m_items;
  bool m_error;
};

class LogAppender {
  friend class Logger;
public:
  typedef SpinLock LockType;
  typedef std::shared_ptr<LogAppender> ptr;

  LogAppender();
  virtual ~LogAppender();
  virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;

  void setFormatter(LogFormatter::ptr fmt);
  void initFormatter(LogFormatter::ptr fmt);
  LogFormatter::ptr getFormatter();

  void setLevel(LogLevel::Level level);
  LogLevel::Level getLevel();

protected:
  LogLevel::Level m_level;
  LogFormatter::ptr m_formatter;
  LockType m_lock;
};

class StdLogAppender : public LogAppender {
public:
  typedef std::shared_ptr<StdLogAppender> ptr;

  ~StdLogAppender();
  void log(LogLevel::Level level, LogEvent::ptr event) override;

private:
};

class FileLogAppender : public LogAppender {
public:
  typedef std::shared_ptr<FileLogAppender> ptr;

  FileLogAppender(const std::string &file);

  ~FileLogAppender();
  void log(LogLevel::Level level, LogEvent::ptr event) override;

  bool reopen();

private:
  std::string m_filename;
  std::ofstream m_filestream;
  time_t m_lastTime;
};

class Logger {
public:
  typedef std::shared_ptr<Logger> ptr;
  typedef SpinLock LockType;

  Logger(std::string name = "root");
  ~Logger();

  void setLevel(LogLevel::Level level) { m_level = level; }
  LogLevel::Level getLevel() const { return m_level; }
  std::string getName() const { return m_name; }
  void setRoot(Logger::ptr root) { m_root = root; }

  void addAppender(LogAppender::ptr appender);
  void delAppender(LogAppender::ptr appender);
  void clearAppender();

  LogFormatter::ptr getFormatter() const { return m_formatter; }
  void setFormatter(LogFormatter::ptr fmt);
  void setFormatter(const std::string &pattern);

  // log the event by the appenders
  void log(LogLevel::Level level, LogEvent::ptr event);

  void debug(LogEvent::ptr event);
  void info(LogEvent::ptr event);
  void warn(LogEvent::ptr event);
  void error(LogEvent::ptr event);
  void fatal(LogEvent::ptr event);

private:
  LogLevel::Level m_level;
  std::list<LogAppender::ptr> m_appenders;
  std::string m_name;
  LogFormatter::ptr m_formatter;
  Logger::ptr m_root;
  LockType m_lock;
};

class LogManager {
public:
  typedef std::shared_ptr<LogManager> ptr;
  typedef SpinLock LockType;

  LogManager();
  ~LogManager() {}

  Logger::ptr getLogger(const std::string &name);
  Logger::ptr getRoot() const;

private:
  std::unordered_map<std::string, Logger::ptr> m_loggers;
  Logger::ptr m_root;
  LockType m_lock;
};

typedef Singleton<LogManager> g_log_manager;

} // namespace mine

#endif