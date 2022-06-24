#include "log.h"
#include <cstddef>

using namespace mine;

int main() {
  Logger::ptr logger(new Logger());
  // log event
  LogEvent::ptr e(new LogEvent(logger, LogLevel::DEBUG, 0, time(NULL), 22, 33));
  // log formatter
  LogFormatter fmt("%d{%Y-%m-%d %H:%M:%S}%T%t%T%T%F%T[%p]%T%f:%l%T%m%n");

  // logger
  logger->addAppender(LogAppender::ptr(new StdLogAppender()));
  logger->addAppender(LogAppender::ptr(new FileLogAppender("log.txt")));
  logger->setLevel(LogLevel::INFO);
  logger->log(LogLevel::DEBUG, e);

  // log manager
  g_log_manager::getInstance()->getRoot()->log(LogLevel::DEBUG, e);

  LOG_DEBUG(logger) << "test macro"; 
  LOG_INFO(logger) << "test macro"; 
  LOG_WARN(logger) << "test macro"; 
  LOG_ERROR(logger) << "test macro"; 
  LOG_FATAL(logger) << "test macro"; 
}