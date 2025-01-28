#ifndef MALLOC_H
# define MALLOC_H
# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <sys/mman.h>
# include "data_structs.h"
# include "../libft/libft.h"
# define SUCCESS 0
# define FATAL_ERROR 1
# define MAX_PAGES 1024
#endif

void print_page_list(s_page *page_head);
