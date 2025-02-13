#include "../includes/malloc.h"

extern s_arena arena_head[MALLOC_ARENA_MAX];

void    show_alloc_mem()
{
    size_t  total_bytes = 0;
    s_page  *page_iterator = arena_head[0].page_head;
    while (page_iterator) // fix loop by iterating over arena head using define and i
    {
        switch (page_iterator->type)
        {
            case TINY:
                ft_printf("TINY : ");
                break;
            case SMALL:
                ft_printf("SMALL : ");
                break;
            case LARGE:
                ft_printf("LARGE : ");
                break;
            default:
                break;
        }
        ft_printf("%p\n", page_iterator);
        s_block_header  *header = GET_FIRST_HEADER(page_iterator);
        int             *metadata = &header->metadata;
        while (!(IS_PAGE_FOOTER(*metadata)))
        {
            void           *block_ptr = GET_BLOCK_PTR(metadata);
            s_block_header *next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
            ft_printf("%p - %p: %d bytes\n", block_ptr, next_header, (*metadata & ~ALLOCATED) / 8);
            total_bytes += *metadata & ~ALLOCATED;
            metadata = &next_header->metadata;
        }
        page_iterator = page_iterator->next;
    }
    ft_printf("Total: ");
    ft_putnbr_fd(total_bytes / 8, STDOUT_FILENO);
    ft_putendl_fd(" bytes", STDOUT_FILENO);
}

void    remove_page_node(s_page **page, s_page *released_page)
{
    s_page *page_iterator = *page;

    if (!released_page)
        return;
    while (page_iterator)
    {
        if (page_iterator->next && page_iterator->next == released_page)
        {
            if (page_iterator->next->next)
                page_iterator->next = page_iterator->next->next;
            else
                page_iterator->next = NULL;
            break;
        }
        page_iterator = page_iterator->next;
    }
}

void    *search_address(void *ptr, s_page **page_iterator)
{
    while (*page_iterator)
    {
        s_block_header *header = GET_FIRST_HEADER(*page_iterator);
        int *metadata = &header->metadata;
        while (!IS_PAGE_FOOTER(*metadata))
        {
            void *block = GET_BLOCK_PTR(metadata);
            if (block == ptr)
                return block;
            else
            {
                s_block_header* next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
                metadata = &next_header->metadata;
            }
        }
        *page_iterator = (*page_iterator)->next;
    }
    return (NULL);
};
