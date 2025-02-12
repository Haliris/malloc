#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stdio.h>
#include <string.h>

void    ft_print_bits(long nb) // chat gpt shit, remove aaah
{
    unsigned long mask = 1UL << (sizeof(long) * 8 - 1); // Get the most significant bit
    int started = 0; // To remove leading zeros

    while (mask > 0)
    {
        if (nb & mask)
        {
            started = 1; // Start printing once we hit the first '1'
            ft_printf("1");
        }
        else if (started) // Skip leading zeros
        {
            ft_printf("0");
        }
        mask >>= 1;
    }
    if (!started) // If nb was 0, at least print a single 0
        ft_printf("0");
    ft_printf("\n");
}

void    print_page_memory(s_page *page)
{
    ft_printf("-----\nPrinting page %p state\n-----\n", page);
    s_block_header *header = GET_FIRST_HEADER(page);
    int *metadata = &header->metadata;
    int block_num = 1;
    while (1)
    {
        if (IS_PAGE_FOOTER(*metadata))
            break;
        ft_printf("Print page memory: Block number %d of size %d at address %p\n", block_num, *metadata & ~ALLOCATED, metadata);
        s_block_header *next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
        metadata = &next_header->metadata;
        block_num++;
    }
    ft_printf("Print page memory: total blocks number: %d\n", block_num - 1);
}

void    print_page_list(s_page *page_head)
{
    s_page* iterator;
    iterator = page_head;
    ft_printf("-----------\n Printing Pages\n-----------\n");
    while (iterator)
    {
        ft_printf("page_head: %p\n", iterator);
        ft_printf("type: %d\n", iterator->type);
        ft_printf("block_head: %p\n", iterator->block_head);
        ft_printf("block metadata: ");
        ft_print_bits(iterator->block_head->metadata);
        ft_printf("block_head metadata: %d\n", iterator->block_head->metadata);
        iterator = iterator->next;
    }
    ft_printf("----------------------\n");
}

void    print_full_heap(s_page *page_head)
{
     s_page* iterator;
    iterator = page_head;
    ft_printf("-----------\n Printing Full heap\n-----------\n");
    while (iterator)
    {
        ft_printf("page_head: %p\n", iterator);
        ft_printf("type: %d\n", iterator->type);
        ft_printf("block_head: %p\n", iterator->block_head);
        ft_printf("block metadata: ");
        ft_print_bits(iterator->block_head->metadata);
        ft_printf("block_head metadata: %d\n", iterator->block_head->metadata);
        print_page_memory(iterator);
        iterator = iterator->next;
    }
    ft_printf("----------------------\n");       
}

void    print_block_info(void *ptr)
{
    if (!ptr)
        write(2, "Null pointer passed to print_block_info!\n", strlen("Null pointer passed to print_block_info!\n"));
    ft_printf("Printing info for block: %p\n", ptr);
    s_block_header *header = (s_block_header*)ptr - 1;
    int metadata = header->metadata;
    write(1, "Metadata of payload: ", strlen("Metadata of payload: "));
    ft_putnbr_fd(metadata & ~ALLOCATED, 1);
    write(1, "\n", 1);
    ft_printf("Metadata bits: ");
    ft_print_bits(metadata);
}


