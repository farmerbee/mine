#include <bits/types/sigset_t.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <unistd.h>

#include "process_cycle.h"
#include "config.h"
#include "global.h"

void master_process_cycle() {
  // block signals while gennerating worker processes
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  sigaddset(&set, SIGALRM);
  sigaddset(&set, SIGIO);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGHUP);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  sigaddset(&set, SIGWINCH);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGQUIT);
  if (sigprocmask(SIG_BLOCK, &set, NULL) < 0) {
    // TODO: log errors
  }

  // start n workers
  auto conf = Config::getInstance();
  int nworker = std::stoi(conf->getValue("WorkerProcesses"));
  if (nworker) {
    start_worker_process(nworker);
  }

  // unblock signals and wait for signals
  sigemptyset(&set);
  while (1) {
    sigsuspend(&set);

    std::cout << "master receive a signal\n";

    sleep(1);
  }
}

void start_worker_process(int nworker)
{
    for (int i = 0; i < nworker; i++)
    {
        spawn_worker_process();
    }
}

void spawn_worker_process()
{
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        // TODO: log errors
        return;
    }
    if (pid > 0)
    {
        // TODO: record the worker process id
        return ;
    }

    processId = getpid();
    parentId = getppid();
    worker_process_cycle();
}

void worker_process_cycle()
{
    init_worker_process();


}

void init_worker_process()
{
    sigset_t set;
    sigemptyset(&set);
    if (sigprocmask(SIG_SETMASK, &set, NULL) < 0)
    {
        // TODO: log errors
    }

}