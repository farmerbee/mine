#include <iostream>

#include "config.h"
#include "daemonize.h"
#include "process_cycle.h"

pid_t processId = -1;
pid_t parentId = -1;


int 
main()
{
    auto conf = Config::getInstance();

    if (!conf->load("/home/farmer/mine/server.conf"))
    {
        std::cerr << "fail to load config file\n";
    }



    return 0;
}