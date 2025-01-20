#ifndef DATA_STRUCTS_H
# define DATA_STRUCTS_H
# include <stdlib.h>

typedef struct s_block
{
    size_t  size;
    char    allocated;
    s_block *prev;
    s_block *next;
} t_block;

typedef struct s_heap
{
    s_heap *prev;    
    s_heap *next;
    t_block     block;
    size_t      group_size;
} t_heap;

# define PAYLOAD_HEADER(block_ptr) ((char *) (block_ptr) - sizeof(t_block))
# define GET_SIZE(ptr) ((t_block_header *) (ptr)->size);
# define GET_ALLOC(ptr) ((t_block_header *) (ptr)->allocated);
# define GET_NEXT_HEADER(block_ptr) ((char *) (block_ptr) + (block_ptr)->size)

#endif
