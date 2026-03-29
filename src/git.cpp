#include "git.h"
#include "plus/diagnostics.hpp"

#include <git2/global.h>
#include <git2/repository.h>
#include <git2/types.h>
#include <iostream>

auto initialize_git_repository(const std::filesystem::path& path, bool make_path) -> void {

	git_libgit2_init();

	git_repository* repo = nullptr;
	git_repository_init_options opts{};
	opts.version = GIT_REPOSITORY_INIT_OPTIONS_VERSION;

	if(make_path) {
		opts.flags |= GIT_REPOSITORY_INIT_MKPATH; /* mkdir as needed to create repo */
	}

	int error = git_repository_init_ext(&repo, path.c_str(), &opts);

	if(error != 0) {
		plus::diag::error_stream() << "could not initialize git repository at `" << path.string()
								   << "` (libgit2 error " << error << ").\n";
		std::quick_exit(error);
	}

	git_repository_free(repo);
	git_libgit2_shutdown();
}
