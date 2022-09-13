/**
 * Static initializer for git_buf from static buffer
 */


#define GIT_BUF_INIT_CONST(STR,LEN) { (char *)(STR), 0, (size_t)(LEN) }
