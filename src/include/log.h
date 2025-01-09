#pragma once

#ifdef DEBUG
#include <iostream>
#define LOG_DEBUG(x)                                                                               \
	do {                                                                                           \
		std::cerr << "[DEBUG]: " << __FILE__ << ":" << __LINE__ << " " << __func__ << " " << #x    \
				  << " = " << x << '\n';                                                           \
	} while(0)

#define LOG_DEBUG_MSG(x)                                                                           \
	do {                                                                                           \
		std::cerr << "[DEBUG]: " << __FILE__ << ":" << __LINE__ << " " << __func__ << " " << #x    \
				  << " = " << x << '\n';                                                           \
	} while(0)

#define LOG_DEBUG_NEWLINE(x)                                                                       \
	do {                                                                                           \
		std::cerr << '\n';                                                                         \
	} while(0)

#define LOG_STREAM(ost, msg)                                                                       \
	do {                                                                                           \
		ost << "[DEBUG]: " << __FILE__ << ":" << __LINE__ << " " << __func__ << " " << msg         \
			<< std::endl;                                                                          \
	} while(0)

#define LOG_STREAM_MSG(ost, msg)                                                                   \
	do {                                                                                           \
		ost << "[DEBUG]: " << __FILE__ << ":" << __LINE__ << " " << __func__ << " " << msg         \
			<< std::endl;                                                                          \
	} while(0)

#else
#define LOG_DEBUG(x)
#define LOG_DEBUG_MSG(msg)
#define LOG_DEBUG_NEWLINE(x)
#define LOG_STREAM(ost, msg)
#define LOG_STREAM_MSG(ost, msg)
#endif
