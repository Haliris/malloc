#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stdio.h>
#include <string.h>

void print_page_list(s_page *page_head)
{
    ft_printf("page_head: %p\n", page_head);
    ft_printf("type: %d\n", page_head->type);
    ft_printf("block_head: %p\n", page_head->block_head);
    ft_printf("free_space: %d\n", page_head->free_space);
}
