#pragma once
/*
 * lg2prelude.h contains any and all helpers need to use the libgit2 library.
 * Most of the code in this file comes from the libgit2 source code and examples.
 * This will act as a wrapper for the libgit2 library.
*/

#include <git2.h>
#include <string>
#include <cstdio>
# include <io.h>
# include <Windows.h>

# define open _open
# define read _read
# define ssize_t int
# define sleep(a) Sleep(a * 1000)

#define snprintf _snprintf
#define strcasecmp strcmpi
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*x))
#define UNUSED(x) (void)(x)



namespace lg2_prelude
{
    void check_lg2(int error, const char *message, const char *extra);
    void init_repo(const std::string& repo_path);
}