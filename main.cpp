#include <signal.h>
#include <cstring>

#include "app.h"
#include "log.h"

App app{};

void sigintHandler(int)
{
    exit(app.stop() ? 0 : -1);
}

int main(int argc, char *argv[])
{
    bool printHelp = false;

    std::string address = "127.0.0.1";
    int port = 8080;

    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];

        if (strcmp(arg, "-h") == 0)
        {
            printHelp = true;
            break;
        }

        if (strcmp(arg, "-b") == 0 && i + 1 < argc)
        {
            address = std::string(argv[i + 1]);
            i++;
            continue;
        }

        if (strcmp(arg, "-p") == 0 && i + 1 < argc)
        {
            port = std::stoi(argv[i + 1]);
            i++;
            continue;
        }
    }

    if (printHelp)
    {
        PRINT_LOG_SIMPLE("Usage: videoProcessor [options...]" << std::endl
                         << "-h\t Print help" << std::endl
                         << "-b\t Bind IP address. Default 127.0.0.1" << std::endl
                         << "-p\t Listen port. Default 8080" << std::endl << std::endl
                         << "example: curl -v -d \"gain=3&"
                            "filename=/path/to/file.mp4\" -X POST \"127.0.0.1:8080\"");

        return 0;
    }

    PRINT_LOG_SIMPLE("example: curl -v -d \"gain=3&"
                     "filename=/path/to/file.mp4\" -X POST \"127.0.0.1:8080\""
                     << std::endl);

    signal(SIGINT, &sigintHandler);

    if (!app.start(address, port))
        return -1;

    app.stop();

    return 0;
}
