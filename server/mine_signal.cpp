#include <cstring>
#include <vector>

#include "mine_signal.h"

MineSigal::MineSigal() {}

MineSigal::~MineSigal() {}

MineSigal::m_signals = {
    {SIGHUP, "SIGHUP", MineSigal::signalHandler},
    {SIGINT, "SIGINT", MineSigal::signalHandler},
    {SIGTERM, "SIGTERM", MineSigal::signalHandler},
    {SIGCHLD, "SIGCHLD", MineSigal::signalHandler},
    {SIGQUIT, "SIGQUIT", MineSigal::signalHandler},
    {SIGIO, "SIGIO", MineSigal::signalHandler},
    {SIGSYS, "SIGSYS, SIG_IGN", NULL},
    // TODO
    {0, NULL, NULL},
};

bool MineSigal::registerSignals() {
  if (m_signals.empty()) {
    return false;
  }

  for (auto sig : m_signals) {
    struct sigaction act;
    std::memset(&act, 0, sizeof(act));
    if (sig.handler) {
      act.sa_sigaction = sig.handler;
      act.sa_flags = SA_SIGINFO;
    } else {
      act.sa_handler = SIG_IGN;
    }
    // TODO: set signal mask with specific signal
    std::memset(&act.sa_mask, 0, sizeof(act.sa_flags));

    if (sigaction(sig.signo, &act, NULL) < 0) {
      return false;
    }
  }

  return true;
}

void MineSigal::signalHandler(int signo, siginfo_t *info, void *ucontext) {
  for (const auto &sig : m_signals) {
  }
}