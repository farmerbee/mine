#include "log.h"
#include <cstddef>

int main() {
  Logger::ptr logger(new Logger());

  LogEvent::ptr e(new LogEvent(logger, LogLevel::DEBUG, 0, time(NULL), 22, 33));

  LogFormatter fmt("%d{%Y-%m-%d %H:%M:%S}%T%t%T%T%F%T[%p]%T%f:%l%T%m%n");

  logger->addAppender(LogAppender::ptr(new StdLogAppender()));

  logger->log(LogLevel::DEBUG, e);
}