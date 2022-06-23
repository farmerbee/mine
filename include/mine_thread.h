#ifndef __MINE_THREAD_H
#define __MINE_THREAD_H

#include "noncopyable.h"
#include <functional>
#include <memory>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace mine {
class Semaphore : public NonCopyable {
public:
  Semaphore(int counter = 0);
  ~Semaphore();

  void wait();
  void notify();

private:
  sem_t m_sem;
  int m_counter;
};

class Thread : public NonCopyable {
public:
  typedef std::shared_ptr<Thread> ptr;
  Thread(std::function<void()> cb, const std::string &name);
  ~Thread();

  std::string getName() const { return m_name; }

  void join();

  static std::string ThreadName();
  static void SetName(const std::string &name);

  static Thread *GetThis();

private:
  static void *run(void *arg);

private:
  pid_t m_id = -1;
  pthread_t m_thread = 0;
  std::function<void()> m_cb;
  Semaphore m_sem_start;
  std::string m_name;
};

} // namespace mine

#endif