#include "log.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

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

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   uint64_t elapse, std::time_t timestamp, uint32_t threadid,
                   uint32_t fiberid)
    : m_logger(logger), m_level(level), m_elapse(elapse),
      m_timestamp(std::time(NULL)), m_threadid(66), m_fiberid(88) {}

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
    os << event->getThreadId();
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
        }
        if (nextChar != '{')
          break;
        status = 1;
        formatBegin = next;
      } else {
        if (!std::isalpha(nextChar)) {
          if (nextChar != '}') {
            std::cerr << "log format pattern error : '}' is missing!";
          } else {
            format = m_pattern.substr(formatBegin + 1, next - formatBegin - 1);
            status = 0;
          }
          break;
        }
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
    }
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

    for (const auto& tpl : vec)
    {
      std::string symbol = std::get<0>(tpl),
                  fmt = std::get<1>(tpl);
      int type = std::get<2>(tpl);
      if (type == 0)
      {
        m_items.emplace_back(LogFormatterItem::ptr(new StringFormatterItem(symbol)));
      }else {
        if (itemMap.count(symbol))
        {
          m_items.emplace_back(itemMap[symbol](fmt));
        } else  
        {
          m_items.emplace_back(LogFormatterItem::ptr(new StringFormatterItem("<<error_format : " + symbol + ">>")));
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

StdLogAppender::~StdLogAppender() {}

void StdLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level > m_level) {
    m_formatter->format(std::cout, level, event);
  }
}

FileLogAppender::~FileLogAppender() {}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {}

bool FileLogAppender::reopen() {
  if (m_filestream) {
    m_filestream.close();
  }
  m_filestream.open(m_filename);

  return !!m_filestream;
}

Logger::Logger(std::string name) : m_name(name) {}

Logger::~Logger() {}

void Logger::addAppender(LogAppender::ptr appender) {
  return m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
  for (auto it = m_appenders.begin(); it != m_appenders.end(); it++) {
    if (*it == appender) {
      m_appenders.erase(it);
      return;
    }
  }
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
  if (level > m_level) {
    for (auto app : m_appenders) {
      app->log(level, event);
    }
  }
}

void Logger::info(LogEvent::ptr event) { log(LogLevel::Level::INFO, event); }

void Logger::debug(LogEvent::ptr event) { log(LogLevel::Level::DEBUG, event); }

void Logger::warn(LogEvent::ptr event) { log(LogLevel::Level::WARN, event); }

void Logger::error(LogEvent::ptr event) { log(LogLevel::Level::ERROR, event); }

void Logger::fatal(LogEvent::ptr event) { log(LogLevel::Level::FATAL, event); }
