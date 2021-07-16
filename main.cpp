#include <iostream>
#include <signal.h>
#include "app.h"

App app{};

void sigintHandler(int)
{
    exit(app.stop() ? 0 : -1);
}

int main()
{
    std::cout << "Hello World!" << std::endl;

    signal(SIGINT, &sigintHandler);

    if (!app.start("127.0.0.1", 8080))
        return -1;

    app.stop();

    return 0;
}
