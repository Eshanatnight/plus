#include "git.h"

#include <git2/global.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <iostream>

auto initializGitRepo(const std::filesystem::path& path, const bool makePath) -> void {

	git_libgit2_init();

	git_repository* repo			 = nullptr;
	git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;

	if(makePath) {
		opts.flags |= GIT_REPOSITORY_INIT_MKPATH; /* mkdir as needed to create repo */
	}

	int error = git_repository_init_ext(&repo, path.c_str(), &opts);

	if(error != 0) {
		std::cerr << "Failed to init Git Repo at Path: " << path.c_str() << std::endl;
		std::cerr << "Error Code: " << error << std::endl;

		// Exit out as this is unrecoverable
		std::quick_exit(error);
	}

	git_repository_free(repo);
	git_libgit2_shutdown();
}
