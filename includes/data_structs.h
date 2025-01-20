#ifndef DATA_STRUCTS_H
# define DATA_STRUCTS_H
# include <stdlib.h>

typedef struct s_heap_list
{
    s_heap_list *prev;    
    s_heap_list *next;

} t_heap_list;

typedef struct s_block
{
    s_block *prev;
    s_block *next;
} t_block;

typedef struct s_block_header
{
    size_t  size;
    char    allocated;
} t_block_header;

# define PAYLOAD_HEADER(block_ptr) ((char *) (block_ptr) - sizeof(t_block_header))
# define GET_SIZE(ptr) ((t_block_header *) (ptr)->size);
# define GET_ALLOC(ptr) ((t_block_header *) (ptr)->allocated);
# define GET_NEXT_HEADER(block_ptr) ((char *) (block_ptr) + (block_ptr)->size)
//

#endif
