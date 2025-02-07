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

typedef enum t_bool
{
    FALSE = 0,
    TRUE = 1
} e_bool;

void    print_page_list(s_page *page_head);
int     request_page(int type, long page_size);
int     init_pages(long* page_size, long requested_size);
void    coalesce_blocks(s_page* page);
int     check_for_page_release(s_page *page);
void    print_block_info(void *ptr);
void    ft_print_bits(long nb);
void    free(void *ptr);
void    *allocate_memory(long long size, int *error_status);
void    *malloc(size_t size);

#endif

