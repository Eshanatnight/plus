#include "buffer.h"

namespace git
{
    Buffer::~Buffer()
    {
        git_buf_dispose(&buf_);
    }
}
