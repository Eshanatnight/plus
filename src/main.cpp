#include "include/githandler.h"
#include <string>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1



int main()
{
    std::string git_dir = ".\\test\\";

    plusutil::initGitRepository(git_dir);


    return EXIT_SUCCESS;
}
