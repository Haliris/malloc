#ifndef MALLOC_H
# define MALLOC_H
# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <sys/mman.h>
# include <stdlib.h>
# include <limits.h>
# include <pthread.h>
# include "data_structs.h"
# include "../libft/libft.h"
# define SUCCESS 0
# define FAILURE 1
# define FATAL_ERROR 1
# define NO_GOOD_PAGE 2
# define MAX_PAGES 1024
# define ROUND_TO_8(x) ((x + 7) &(-8));

# define IS_PAGE_FOOTER(metadata)  (((metadata & ~ALLOCATED) == 0) && (metadata & ALLOCATED))
# define GET_FIRST_HEADER(page_iterator) ((s_block_header*)((char*)page_iterator + sizeof(s_page)))
# define GET_BLOCK_PTR(metadata) ((void*) ((char*)metadata + sizeof(s_block_header)))
# define GET_HEADER_FROM_BLOCK(ptr) ((s_block_header*)((char*)block - sizeof(s_block_header)))
# define GET_NEXT_HEADER_FROM_BLOCK(ptr, metadata) ((s_block_header*)((char*)ptr + (metadata & ~ALLOCATED)))
# define GET_NEXT_HEADER_FROM_HEADER(metadata) ((s_block_header*)((char*)metadata + ((*metadata & ~ALLOCATED) + sizeof(s_block_header))))

# define MALLOC_ARENA_MAX 1024

typedef enum t_bool
{
    FALSE = 0,
    TRUE = 1
} e_bool;

void    print_page_list(s_page *page_head);
void    print_page_memory(s_page *page);
void    print_full_heap(s_page *page_head);
int     request_page(s_page **page_head, long long type, long page_size);
int     init_pages(s_page **page_head, long* page_size, long requested_size);
void    defragment_page(s_page* page);
int     check_for_page_release(s_page *page);
void    print_block_info(void *ptr);
void    ft_print_bits(long nb);


void    *malloc(size_t size);
void    *allocate_memory(int assigned_arena, long long size, int *error_status);
void    free(void *ptr);
void    *realloc(void *ptr, size_t size);
void    show_alloc_mem();


void    remove_page_node(int assigned_arena, s_page *released_page);
void    *search_address(void *ptr, s_page **page_iterator);
int     *get_assigned_arena(void);
#endif

