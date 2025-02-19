#ifndef DATA_STRUCTS_H
# define DATA_STRUCTS_H
# include "../includes/malloc.h"
# define ALLOCATED 1
# include <pthread.h>
# include <stddef.h>

typedef enum t_zone_type
{
    TINY = 8,
    SMALL = 32,
    LARGE
} e_zone_type;

typedef struct t_block_header
{
    int     metadata;
    char    padding[4];
} s_block_header;

typedef struct t_page
{
    struct t_page     *next;
    s_block_header    *block_head;
    e_zone_type       type;
} s_page;

typedef struct t_arena
{
    s_page          *page_head;
    pthread_mutex_t lock;
    atomic_int      assigned_threads; 
    atomic_bool     arena_initialized;
} s_arena;

#endif
