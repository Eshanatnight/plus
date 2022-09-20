#include "cli.h"
#include <git2.h>
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fmt/color.h>


int main(int argc, char** argv)
{
	if(argc < 2)
    {
        Cli::printHelp();
        return 0;
    }

    Cli app;
    std::vector<std::string> args(argv, argv + argc);
    app.run(args);
}
