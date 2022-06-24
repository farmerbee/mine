#include "log.h"
#include "mine_thread.h"
#include "mutex.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <unistd.h>

using namespace mine;

uint64_t volatile num = 0;
Mutex numMutex;
auto rootLogger = LOG_ROOT();

void handler() {
  for (uint64_t i = 0; i < 50000; i++) {
    Mutex::Lock lock(numMutex);
    num++;
  }

  // std::cout << Thread::GetThis()->getName() <<  "  " << num << std::endl;
  // std::cout << "in thread, the result is " << num << std::endl;
}

int main() {
  Thread::ptr t1 = std::make_shared<Thread>(handler, "thread1");
  Thread::ptr t2 = std::make_shared<Thread>(&handler, "thread2");
  Thread::ptr t3 = std::make_shared<Thread>(&handler, "thread3");
  Thread::ptr t4 = std::make_shared<Thread>(&handler, "thread4");

  rootLogger->addAppender(FileLogAppender::ptr(new FileLogAppender("log.txt")));

  t1->join();
  t2->join();
  t3->join();
  t4->join();

  // std::cout << "pseudo message" << num << std::endl;
  LOG_INFO((LOG_ROOT())) << num;
  { LOG_INFO((LOG_ROOT())) << "pseudo message"; }
  std::cout << "pseudo message" << num << std::endl;
  std::cout << "pseudo message" << num << std::endl;
  std::cout << "pseudo message" << num << std::endl;
  std::cout << "pseudo message" << num << std::endl;
  std::cout << "pseudo message" << num << std::endl;
  std::cout << "pseudo message" << num << std::endl;
  std::cout << "pseudo message" << num << std::endl;
  std::cout << "pseudo message" << num << std::endl;

  return 0;
}