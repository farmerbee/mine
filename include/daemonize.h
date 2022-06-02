#ifndef __DEAMONIZE_H
#define __DEAMONIZE_H

// daemonize a process, thus let it to run in the background
// -1: error  0: the child process   1: the parent process
int daemonize();

#endif