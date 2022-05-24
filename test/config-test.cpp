#include <iostream>
#include <string>

#include "config.h"

using namespace mine;

int main()
{
    std::string configPath = "/home/farmer/mine/nginx.conf";

    auto config = Config::getInstance();

    if (! config->load(configPath))
    {
        std::cerr << "failed to load the config file\n";
        exit(EXIT_FAILURE);
    }

    config->printConfig();
}