#ifndef CD_HANDLE_H
#define CD_HANDLE_H

/** Change the current working directory.
 *  Supports cd, cd -, cd ~, cd ~user via expand_tilde().
 *  Returns 0 on success, -1 on error. */
int cd_handle(char** args, char* cwd, char* last_dir);

#endif

