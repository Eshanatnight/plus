#pragma once
#include <future>
#include <vector>

inline void resolve(std::vector<std::future<void>>& futures) {
	for(auto& fut: futures) {
		fut.get();
	}
}
