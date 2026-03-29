#pragma once

#include <iostream>
#include <ostream>
#include <string_view>

namespace plus::diag {

	/** User-visible errors: single prefix, no redundant "Error:" in the message body. */
	inline std::ostream& error_stream() {
		return std::cerr << "plus: error: ";
	}

	inline void error(std::string_view message) {
		error_stream() << message;
	}

} // namespace plus::diag
