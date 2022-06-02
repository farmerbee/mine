#include <iostream>
#include <string>

#include "config.h"


int main()
{
    std::string configPath = "server.conf";

    auto config = Config::getInstance();

    if (! config->load(configPath))
    {
        std::cerr << "failed to load the config file\n";
        exit(EXIT_FAILURE);
    }

    config->printConfig();
}