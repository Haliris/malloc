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


#endif
