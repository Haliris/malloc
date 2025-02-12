#include "../includes/malloc.h"
extern s_page *page_head;

void    *realloc(void *ptr, size_t size)
{
    s_page  **page_iterator = &page_head;
    size_t  ptr_size = 0;
    void    *payload = NULL;

    if (size == 0 && ptr)
    {
        free(ptr);
        return (NULL);
    }
    if (size > LLONG_MAX - 7)
        return (NULL); //Add a custom error message and explain in defense the limitation, although why allocated 1048576 teras???
    if (size % 8 != 0)
        size = ROUND_TO_8(size);
    if (!ptr)
    {
        payload = malloc(size);
        if (!payload)
            return (NULL);
    }
    void *block = search_address(ptr, page_iterator);
    if (!block)
    {
        void *payload = malloc(size);
        if (!payload)
            return (NULL);
    };
    s_block_header *header = GET_HEADER_FROM_BLOCK(block);
    int *metadata = &header->metadata;
    size_t block_size = *metadata & ~ALLOCATED;
    ptr_size = block_size;
    if ((long long)size < (*metadata & ~ALLOCATED))
    {
        *metadata = size;
        s_block_header *next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
        *metadata |= ALLOCATED;
        next_header->metadata = block_size - size - sizeof(s_block_header);
        return (ptr);
    }
    while (1)
    {
        if (IS_PAGE_FOOTER(*metadata))
            break;
        s_block_header *next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
        if ((next_header->metadata & ALLOCATED))
            break;
        else
        {
            (*page_iterator)->free_space -= *metadata;
            if (next_header == (*page_iterator)->block_head)
                (*page_iterator)->block_head = (s_block_header*) ((char*)metadata);
            *metadata = *metadata + next_header->metadata + sizeof(s_block_header);
            block_size = *metadata & ~ALLOCATED;
            next_header->metadata = 0;
            (*page_iterator)->free_space += *metadata;
        }
        if (block_size == size)
            return (ptr);
        else if (block_size > size)
        {
            *metadata = size;
            next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
            *metadata |= ALLOCATED;
            next_header->metadata = block_size - size - sizeof(s_block_header);
            return (ptr);
        }
    }
    payload = malloc(size);
    if (!payload)
    {
        free(ptr);
        return (NULL);
    }
    payload = ft_memmove(payload, ptr, ptr_size);
    free(ptr);
    return (payload);
};
