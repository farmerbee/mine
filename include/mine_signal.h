#ifndef __MINE_SIGNAL_
#define __MINE_SIGNAL_

/**
 * @brief signal registration, mainly used for communication between master and
 * worker processes
 *
 */

#include <signal.h>
#include <string>
#include <vector>

typedef void (*sa_handler_t)(int, siginfo_t *, void *);

class MineSigal {
public:
  MineSigal();

  ~MineSigal();

  // add signals to be registered
  void initSignals();

  bool registerSignals();

private:
  typedef struct {
    int signo;
    std::string signame;
    sa_handler_t handler;
  } m_signal_t;

public:
  // signals to be registered
  static std::vector<m_signal_t> m_signals;

  static void signalHandler(int signo, siginfo_t *info, void *ucontext);
};

#endif