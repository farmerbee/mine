#include "mine_thread.h"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

namespace mine {
static thread_local Thread *g_thread = nullptr;
static thread_local std::string g_tname = "UNKNOWN";

Semaphore::Semaphore(int counter) : m_counter(counter) {
  if (sem_init(&m_sem, 0, m_counter) < 0) {
    int error = errno;
    // TODO: log the error
  }
}

Semaphore::~Semaphore() {
  if (sem_destroy(&m_sem) < 0) {
    int error = errno;
    // TODO: log the error
  }
}

void Semaphore::wait() {
lblwait:
  if (sem_wait(&m_sem) < 0) {
    int error = errno;
    if (error == EINTR) {
      goto lblwait;
    }
    // TODO: log the eror
  }
}

void Semaphore::notify() {
  if (sem_post(&m_sem) < 0) {
    int error = errno;
    // TODO: log the error
  }
}

Thread::Thread(std::function<void()> cb, const std::string &name)
    : m_cb(cb), m_name(name) {
  int err;
  if ((err = pthread_create(&m_thread, NULL, &Thread::run, this))) {
    // TODO: log the error
  }

  // wait to confirm that the new thread is truly started
  m_sem_start.wait();
}

Thread::~Thread() {
  if (m_thread)
    pthread_detach(m_thread);
  m_thread = 0;
}

void Thread::join() {
  if (m_thread) {
    int err = pthread_join(m_thread, NULL);
    if (err) {
      // TODO: log the error
    }
    // std::cout << "thread is joined\n";
    m_thread = 0;
  }
}

std::string Thread::ThreadName() { return g_tname; }

void Thread::SetName(const std::string &name) {
  if (!name.empty()) {
    g_tname = name;
    if (g_thread)
      g_thread->m_name = name;
  }
}

Thread *Thread::GetThis() { return g_thread; }

void *Thread::run(void *arg) {
  Thread *thisThread = static_cast<Thread *>(arg);
  g_thread = thisThread;
  g_tname = thisThread->getName();
  thisThread->m_id = gettid();
  thisThread->m_thread = pthread_self();
  pthread_setname_np(thisThread->m_thread,
                     thisThread->m_name.substr(0, 15).c_str());

  std::function<void()> cb;
  cb.swap(thisThread->m_cb);

  thisThread->m_sem_start.notify();

  cb();
  // thisThread->m_cb();

  return nullptr;
}

} // namespace mine