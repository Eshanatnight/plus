#include "cli.h"

#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fmt/color.h>


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printHelp();
        return 0;
    }

    std::vector<std::string> args(argv, argv + argc);
    run(args);
}
