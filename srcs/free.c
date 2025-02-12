#include "../includes/malloc.h"

extern s_page *page_head;

void    coalesce_blocks(s_page* page)
{
    s_block_header *header = GET_FIRST_HEADER(page);
    int *metadata = &header->metadata;
    while (!IS_PAGE_FOOTER(*metadata))
    {
        void *ptr = GET_BLOCK_PTR(metadata);
        s_block_header* next_header = GET_NEXT_HEADER_FROM_BLOCK(ptr, *metadata);
        if (*metadata & ALLOCATED)
        {
            metadata = &next_header->metadata;
            continue;
        }
        if (!(next_header->metadata & ALLOCATED))
        {
            if (next_header == page->block_head)
                page->block_head = (s_block_header*) ((char*)metadata);
            *metadata = *metadata + next_header->metadata + sizeof(s_block_header);
            next_header->metadata = 0;
        }
        else
            metadata = &next_header->metadata;
    }
}

int    check_for_page_release(s_page *page)
{
    s_block_header *header = GET_FIRST_HEADER(page);
    int *metadata = &header->metadata;
    while (!IS_PAGE_FOOTER(*metadata))
    {
        if (*metadata & ALLOCATED)
            return (FALSE);
        s_block_header* next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
        metadata = &next_header->metadata;
    }
    return (TRUE);
}

void    free(void *ptr)
{
    s_page **page_iterator = &page_head;
    if (ptr == NULL)
        return;
    void *block = search_address(ptr, page_iterator);
    if (!block)
        return;
    s_block_header *header = GET_HEADER_FROM_BLOCK(block);
    int *metadata = &header->metadata;
    if (!(*metadata & ~ALLOCATED))
        return;
    else
    {
        *metadata ^= ALLOCATED;
        ft_memset(ptr, 0, *metadata);
        coalesce_blocks(*page_iterator); 
        if (check_for_page_release(*page_iterator) == TRUE)
        {
            header = GET_FIRST_HEADER(*page_iterator);
            remove_page_node(*page_iterator);
            munmap(*page_iterator, header->metadata + sizeof(s_page) + 2 * sizeof(s_block_header));
            *page_iterator = NULL;
        }
        return;
    }
}
