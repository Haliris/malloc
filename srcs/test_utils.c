#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stdio.h>
#include <string.h>

void print_page_list(s_page *page_head)
{
    s_page* iterator;
    iterator = page_head;
    while (iterator)
    {
        ft_printf("page_head: %p\n", iterator);
        ft_printf("type: %d\n", iterator->type);
        ft_printf("block_head: %p\n", iterator->block_head);
        ft_printf("free_space: %d\n", iterator->free_space);
        iterator = iterator->next;
    }
}
