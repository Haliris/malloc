#ifndef DATA_STRUCTS_H
# define DATA_STRUCTS_H
# define ALLOCATED 1

//Align memory to 8 so that the last 3 bits are always free to be written into. Need to mask them to get the size however
//Need to setup headers and footers so that we can use then footers to go back through the list instead of being stuck going forward

typedef enum t_zone_type
{
    TINY = 32,
    SMALL = 64,
    LARGE = 128
} e_zone_type;

typedef struct t_block_header
{
    int     metadata;
    char    padding[4]; //TO make it 8??
} s_block_header;

typedef struct t_page
{
    struct t_page*     next;
    s_block_header*           block_head;
    e_zone_type        type;
    long long          free_space; // Have some way to 'preview' the page so that we do not always need to scan it??
} s_page;

# define PAYLOAD_HEADER(block_ptr) ((char *) (block_ptr) - sizeof(s_block_header));
# define GET_SIZE(ptr) ((s_block_header *) (ptr)->size); // Need to mask off the tree bits of the header word
# define GET_ALLOC(ptr) ((s_block_header *) (ptr)->allocated); // Same but the opposite
# define GET_NEXT_HEADER(block_ptr) ((char *) (block_ptr) + (block_ptr)->size);
# define PAYLOAD_FOOTER(block_ptr) ((char *) (block_ptr) + GET_SIZE(PAYLOAD_HEADER(block_tr)) - sizeof(s_block));
# define ROUND_TO_8(x) ((x + 7) &(-8));

#endif
