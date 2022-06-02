#include "daemonize.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int daemonize() {
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    return pid;
  }
  // parent process
  if (pid > 0) {
    return 1;
  }

  /* the child process  */
  // create a new session and detatch from the controlling terminal
  pid_t newPid = setsid();
  if (newPid < 0) {
    return -1;
  }
  umask(0);
  // file discriptor redirection
  int nullFd = open("/dev/null", O_RDWR);
  if (nullFd < 0) {
    return nullFd;
  }
  // stdin
  if (dup2(nullFd, STDIN_FILENO) < 0) {
    return -1;
  }
  // stdout
  if (dup2(nullFd, STDOUT_FILENO) < 0) {
    return -1;
  }
  // stderr
  if (dup2(nullFd, STDERR_FILENO) < 0) {
    return -1;
  }

  return 0;
}