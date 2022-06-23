#ifndef __MUTEX_H
#define __MUTEX_H

#include "noncopyable.h"
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

namespace mine {

// lock guard: lock and unlock the mutex automatically
template <typename T> class ScoppedLockImpl : public NonCopyable {
public:
  ScoppedLockImpl(T &mutex) : m_mutex(mutex) { mutex.lock(); }

  ~ScoppedLockImpl() { m_mutex.unlock(); }

private:
  T& m_mutex;
};

class Mutex : public NonCopyable {
public:
  typedef ScoppedLockImpl<Mutex> Lock;

  Mutex() : m_holder(0) { pthread_mutex_init(&m_mutex, nullptr); }

  ~Mutex() {
    assert(m_holder == 0);
    pthread_mutex_destroy(&m_mutex);
  }

  void lock() {
    pthread_mutex_lock(&m_mutex);
    m_holder = gettid();
  }

  void unlock() {
    m_holder = 0;
    pthread_mutex_unlock(&m_mutex);
  }

  pthread_mutex_t *getMutex() { return &m_mutex; }

  bool lockedByThisThread() { return m_holder == gettid(); }

private:
  pthread_mutex_t m_mutex;
  pid_t m_holder;
};

// TODO: encapsulate reader and writter lockguard
// 1. RAII to pthread_rwlock_t RWMutex with method: readLock, writeLock, unlock
// 2. encapsulate ReadLockGuard with RWMutex.readLock() and RWMutex.unlock()
//  encapsulate WriteLockGuard with RWMutex.writeLock() and RWMutex.unlock()

} // namespace mine

#endif