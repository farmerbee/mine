#include "log.h"
#include "mutex.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

/**----------------- LogLevel ---------------------------**/
using namespace mine;

std::string LogLevel::toString(LogLevel::Level level) {
  switch (level) {
#define CASE(lv)                                                               \
  case LogLevel::Level::lv:                                                    \
    return #lv;                                                                \
    break;

    CASE(DEBUG)
    CASE(INFO)
    CASE(WARN)
    CASE(ERROR)
    CASE(FATAL)

#undef CASE

  default:
    return "UNKNOWN";
  }
}

LogLevel::Level LogLevel::fromString(std::string &hint) {
  std::transform(hint.begin(), hint.end(), hint.end(),
                 [](int ch) { return std::toupper(ch); });

#define IF(lv, hint)                                                           \
  if (#lv == hint)                                                             \
    return LogLevel::Level::lv;

  IF(DEBUG, hint)
  IF(INFO, hint)
  IF(WARN, hint)
  IF(ERROR, hint)
  IF(FATAL, hint)

#undef IF

  return LogLevel::UNKOWN;
}

/**----------------- LogEvent ---------------------------**/

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   uint64_t elapse, std::time_t timestamp, uint32_t threadid,
                   uint32_t fiberid)
    : m_logger(logger), m_level(level), m_elapse(elapse),
      m_timestamp(std::time(NULL)), m_threadid(threadid), m_fiberid(fiberid),
      m_filename(__FILE__), m_lineno(__LINE__) {}

LogEventWrapper::LogEventWrapper(LogEvent::ptr event) : m_event(event) {}

// log the string stream when deconstructed
LogEventWrapper::~LogEventWrapper() {
  std::stringstream &ss = m_event->getSS();
  if (!ss.str().empty()) {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
  }
}

std::stringstream &LogEventWrapper::getSS() { return m_event->getSS(); }

/**----------------- LogFormatter ---------------------------**/

LogFormatter::LogFormatterItem::~LogFormatterItem() {}

void LogFormatter::LogFormatterItem::format(std::stringstream &ss,
                                            LogLevel::Level level,
                                            LogEvent::ptr event) {}

class MessageFormatterItem : public LogFormatter::LogFormatterItem {
public:
  MessageFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getContent();
  }
};

class FileFormatterItem : public LogFormatter::LogFormatterItem {
public:
  FileFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getFilename();
  }
};

class ElapseFormatterItem : public LogFormatter::LogFormatterItem {
public:
  ElapseFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getElapse();
  }
};

class LevelFormatterItem : public LogFormatter::LogFormatterItem {
public:
  LevelFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << LogLevel::toString(event->getLevel());
  }
};

class ThreadIdFormatterItem : public LogFormatter::LogFormatterItem {
public:
  ThreadIdFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getThreadId();
  }
};

class FiberIdFormatterItem : public LogFormatter::LogFormatterItem {
public:
  FiberIdFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getFiberId();
  }
};

class TimestampFormatterItem : public LogFormatter::LogFormatterItem {
public:
  TimestampFormatterItem(const std::string &format) : m_format(format) {
    if (format.empty()) {
      m_format = "%Y-%m-%d %H:%M:%S";
    }
  }

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    struct tm ltime;
    time_t time = event->getTimestamp();
    if (!localtime_r(&time, &ltime)) {
      perror("localtime_r()");
      return;
    }
    char buff[101];
    strftime(buff, 100, m_format.c_str(), &ltime);
    os << buff;
  }

private:
  std::string m_format;
};

class LineFormatterItem : public LogFormatter::LogFormatterItem {
public:
  LineFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << event->getLineNumber();
  }
};

class NewLineFormatterItem : public LogFormatter::LogFormatterItem {
public:
  NewLineFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << std::endl;
  }
};

class TabFormatterItem : public LogFormatter::LogFormatterItem {
public:
  TabFormatterItem(const std::string &str = "") {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << "\t";
  }
};

class StringFormatterItem : public LogFormatter::LogFormatterItem {
public:
  StringFormatterItem(const std::string &str) : m_str(str) {}

  void format(std::ostream &os, LogLevel::Level level,
              LogEvent::ptr event) override {
    os << m_str;
  }

private:
  std::string m_str;
};

LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern), m_error(false) {

  init();
}

void LogFormatter::init() {
  std::string pureString = "";
  // tuple(symbol, format, type)
  std::vector<std::tuple<std::string, std::string, int>> vec;
  auto size = m_pattern.size();

  // std::cout << "pattern: " << m_pattern << std::endl;
  for (std::string::size_type i = 0; i < size; i++) {
    char ch = m_pattern[i];
    // pure string
    if (ch != '%') {
      pureString.push_back(ch);
      continue;
    }
    // %%, escapse charator
    if (i + 1 < size && m_pattern[i + 1] == '%') {
      pureString.push_back(m_pattern[i + 1]);
      i++;
      continue;
    }
    // start parse the pattern with format
    std::string::size_type next = i + 1, formatBegin = next;
    // 0 : parse symbol  1 : parse format
    int status = 0;
    std::string symbol = "", format = "";
    while (next < size) {
      char nextChar = m_pattern[next];
      if (status == 0) {
        if (!std::isalpha(nextChar)) {
          symbol = m_pattern.substr(i + 1, next - i - 1);
          if (nextChar != '{')
            break;
          status = 1;
          formatBegin = next;
        }
      } else if (nextChar == '}') {
        format = m_pattern.substr(formatBegin + 1, next - formatBegin - 1);
        status = 0;
        next++;
        break;
      }

      next++;
    }

    if (status == 0) {
      if (next == size) {
        symbol = m_pattern.substr(i + 1, next - i - 1);
      }
      if (!pureString.empty()) {
        vec.emplace_back(
            std::tuple<std::string, std::string, int>{pureString, "", 0});
        pureString.clear();
      }

      vec.emplace_back(
          std::tuple<std::string, std::string, int>{symbol, format, 1});
      i = next - 1;
    } else {
      std::cerr << "log format error since : " << m_pattern.substr(i)
                << std::endl;
      vec.emplace_back(std::make_tuple("<<pattern_error>>", "", 0));
      m_error = true;
      break;
    }

    // std::cout << i << ": " << symbol << "\t" << format << std::endl;
  }

  if (!pureString.empty()) {
    vec.emplace_back(std::make_tuple(pureString, "", 0));
  }

  std::unordered_map<std::string,
                     std::function<LogFormatterItem::ptr(const std::string &)>>
      itemMap{
#define KV(symbol, item)                                                       \
  {                                                                            \
#symbol, [](const std::string                                              \
                    &str) { return LogFormatterItem::ptr(new item(str)); }     \
  }

          KV(m, MessageFormatterItem), KV(p, LevelFormatterItem),
          KV(r, ElapseFormatterItem),  KV(t, ThreadIdFormatterItem),
          KV(n, NewLineFormatterItem), KV(d, TimestampFormatterItem),
          KV(f, FileFormatterItem),    KV(l, LineFormatterItem),
          KV(T, TabFormatterItem),     KV(F, FiberIdFormatterItem),
#undef KV
      };

  for (const auto &tpl : vec) {
    std::string symbol = std::get<0>(tpl), fmt = std::get<1>(tpl);
    int type = std::get<2>(tpl);
    if (type == 0) {
      m_items.emplace_back(
          LogFormatterItem::ptr(new StringFormatterItem(symbol)));
    } else {
      if (itemMap.count(symbol)) {
        m_items.emplace_back(itemMap[symbol](fmt));
      } else {
        m_items.emplace_back(LogFormatterItem::ptr(
            new StringFormatterItem("<<error_format : " + symbol + ">>")));
        m_error = true;
      }
    }
  }
}

std::string LogFormatter::format(std::stringstream &ss, LogLevel::Level level,
                                 LogEvent::ptr event) {
  for (const auto &item : m_items) {
    item->format(ss, level, event);
  }

  return ss.str();
}

std::ostream &LogFormatter::format(std::ostream &os, LogLevel::Level level,
                                   LogEvent::ptr event) {
  for (const auto &item : m_items) {
    item->format(os, level, event);
  }

  return os;
}

/**----------------- LogAppender ---------------------------**/

LogAppender::LogAppender() : m_level(LogLevel::DEBUG), m_formatter() {}

LogAppender::~LogAppender() {}

void LogAppender::setFormatter(LogFormatter::ptr fmt) {
  LockType::Lock lock(m_lock);
  m_formatter = fmt;
}

void LogAppender::initFormatter(LogFormatter::ptr fmt) {
  LockType::Lock lock(m_lock);
  if (!m_formatter)
    m_formatter = fmt;
}

LogFormatter::ptr LogAppender::getFormatter() {
  LockType::Lock lock(m_lock);
  return m_formatter;
}

void LogAppender::setLevel(LogLevel::Level level) {
  LockType::Lock lock(m_lock);
  m_level = level;
}

LogLevel::Level LogAppender::getLevel() {
  LockType::Lock lock(m_lock);
  return m_level;
}

StdLogAppender::~StdLogAppender() {}

void StdLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level >= m_level) {
    LockType::Lock lock(m_lock);
    m_formatter->format(std::cout, level, event);
  }
}

FileLogAppender::FileLogAppender(const std::string &file)
    : m_filename(file), m_lastTime(time(NULL)) {
  reopen();
}

FileLogAppender::~FileLogAppender() {
  LockType::Lock lock(m_lock);
  if (m_filestream.is_open()) {
    m_filestream.close();
  }
}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level < m_level)
    return;

  auto logTime = event->getTimestamp();
  if (logTime > m_lastTime + 3) {
    reopen();
    m_lastTime = logTime;
  }

  LockType::Lock lock(m_lock);
  m_formatter->format(m_filestream, level, event);
}

bool FileLogAppender::reopen() {
  LockType::Lock lock(m_lock);
  if (m_filestream.is_open()) {
    m_filestream.close();
  }
  m_filestream.open(m_filename, std::ofstream::app);

  return m_filestream.is_open();
}

/**----------------- Logger ---------------------------**/

Logger::Logger(std::string name) : m_name(name), m_level(LogLevel::DEBUG) {
  m_formatter.reset(
      new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%T%F%T[%p]%T%f:%l%T%m%n"));
}

Logger::~Logger() {}

void Logger::addAppender(LogAppender::ptr appender) {
  // if (!appender->getFormatter()) {
  //   appender->setFormatter(m_formatter);
  // }
  // {
  //   LogAppender::LockType::Lock lock(appender->m_lock);
  //   if (!appender->m_formatter) {
  //     appender->m_formatter = m_formatter;
  //   }
  // }
  appender->initFormatter(m_formatter);

  LockType::Lock lock(m_lock);
  m_appenders.emplace_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
  LockType::Lock lock(m_lock);
  for (auto it = m_appenders.begin(); it != m_appenders.end(); it++) {
    if (*it == appender) {
      m_appenders.erase(it);
      return;
    }
  }
}

void Logger::clearAppender() {
  LockType::Lock lock(m_lock);
  return m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr fmt) {
  LockType::Lock lock(m_lock);
  m_formatter = fmt;
  for (auto app : m_appenders) {
    // if (!app->getFormatter()) {
    //   app->setFormatter(fmt);
    // }
    app->initFormatter(m_formatter);
  }
}

void Logger::setFormatter(const std::string &pattern) {
  LogFormatter::ptr fmt(new LogFormatter(pattern));
  if (fmt->error()) {
    std::cerr << "formatter pattern error : " << pattern << std::endl;
    return;
  }
  setFormatter(fmt);
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level >= m_level) {
    LockType::Lock lock(m_lock);
    if (!m_appenders.empty()) {
      for (auto app : m_appenders) {
        app->log(level, event);
      }
    } else {
      m_root->log(level, event);
    }
  }
}

void Logger::info(LogEvent::ptr event) { log(LogLevel::Level::INFO, event); }

void Logger::debug(LogEvent::ptr event) { log(LogLevel::Level::DEBUG, event); }

void Logger::warn(LogEvent::ptr event) { log(LogLevel::Level::WARN, event); }

void Logger::error(LogEvent::ptr event) { log(LogLevel::Level::ERROR, event); }

void Logger::fatal(LogEvent::ptr event) { log(LogLevel::Level::FATAL, event); }

/**----------------- LogManager---------------------------**/

LogManager::LogManager() {
  m_root.reset(new Logger);
  m_root->addAppender(StdLogAppender::ptr(new StdLogAppender));

  m_loggers[m_root->getName()] = m_root;
}

Logger::ptr LogManager::getLogger(const std::string &name) {
  LockType::Lock lock(m_lock);
  if (!m_loggers.count(name)) {
    Logger::ptr newLogger(new Logger(name));
    newLogger->setRoot(m_root);

    m_loggers[name] = newLogger;
  }

  return m_loggers[name];
}

Logger::ptr LogManager::getRoot() const { return m_root; }
