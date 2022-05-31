#include <iostream>
#include <unistd.h>

#include "mine_signal.h"

int main()
{
    MineSigal sigs;
    if (sigs.registerSignals())
    {
        std::cout << "all signals are registered\n";
    }
    else
    {
        std::cerr << "failed to register some signal\n";
    }

    while (1)
    {
        pause();
    }

    return 0;
}