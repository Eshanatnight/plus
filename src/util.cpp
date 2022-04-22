#include <fmt/os.h>
#include <fmt/ostream.h>

namespace plusutil
{
    // creates and returns a fmt::ostream with the given filename
    [[nodiscard]] inline fmt::ostream createFile(const std::string& filename)
    {
        return fmt::output_file(filename, fmt::file::WRONLY | fmt::file::CREATE);
    }
}