#ifndef MALLOC_H
# define MALLOC_H
# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <sys/mman.h>
# include <stdlib.h>
# include <limits.h>
# include "data_structs.h"
# include "../libft/libft.h"
# define SUCCESS 0
# define FATAL_ERROR 1
# define NO_GOOD_PAGE 2
# define MAX_PAGES 1024
# define IS_LARGE_TYPE(r_size, p_size) (LARGE * (r_size >= (LARGE * p_size)));
# define IS_SMALL_TYPE(r_size, p_size) (SMALL * (((r_size >= (TINY * p_size)) * ((r_size < (LARGE * p_size))))));
# define IS_TINY_TYPE(r_size, p_size) (TINY * (r_size < SMALL * p_size));

void    print_page_list(s_page *page_head);
void    print_block_info(void *ptr);
void    ft_print_bits(long nb);
#endif

