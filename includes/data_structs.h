#ifndef DATA_STRUCTS_H
# define DATA_STRUCTS_H
# include "../includes/malloc.h"
# define ALLOCATED 1
# include <pthread.h>
# include <stddef.h>
//Align memory to 8 so that the last 3 bits are always free to be written into. Need to mask them to get the size however
//Need to setup headers and footers so that we can use then footers to go back through the list instead of being stuck going forward

typedef enum t_zone_type
{
    TINY = 8,
    SMALL = 32,
    LARGE
} e_zone_type;

typedef struct t_block_header
{
    int     metadata;
    char    padding[4]; //TO make it 8??
} s_block_header;

typedef struct t_page
{
    struct t_page*     next;
    s_block_header*    block_head;
    e_zone_type        type;
} s_page;

typedef struct t_arena
{
    s_page          *page_head;
    size_t          size;
    pthread_mutex_t lock;
    int             initialized;
    int             assigned_threads;
} s_arena;

#endif
