#ifndef __PROCESS_CYCLE_H
#define __PROCESS_CYCLE_H

// generate worker process, and wait for signals
void master_process_cycle();

// start  n workers
void start_worker_process(int nworker);

// spawn a worker process which is responsible for the connections
void spawn_worker_process();

// process the connections
void worker_process_cycle();

void init_worker_process();

#endif