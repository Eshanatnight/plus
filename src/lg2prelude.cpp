#include "include/lg2prelude.h"
#include <fmt/ostream.h>

void lg2_prelude::check_lg2(int error, const char *message, const char *extra)
{
    const git_error *lg2err;
    const char *lg2msg = "", *lg2spacer = "";

    if (!error)
        return;

    if ((lg2err = git_error_last()) != NULL && lg2err->message != NULL) {
        lg2msg = lg2err->message;
        lg2spacer = " - ";
    }

    if (extra)
        fmt::print(stderr, "`{}` `{}` [{}]{}{}\n", message, extra, error, lg2spacer, lg2msg);

    else
        fmt::print(stderr, "%s [%d]%s%s\n",message, error, lg2spacer, lg2msg);

    exit(1);
}

void lg2_prelude::init_repo(const std::string &repo_path)
{
    git_repository* repo;

    check_lg2(
            git_repository_init(&repo, repo_path.c_str(), 0),
            "Unable to init repository at `%s`",
            repo_path.c_str()
        );
}