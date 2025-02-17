#include "../includes/malloc.h"

extern s_arena arena_head[MALLOC_ARENA_MAX];
extern atomic_int mapped_mem;

void    defragment_page(s_page* page)
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
        {
            ft_printf("Page still in use: %p!\n", page);
            return (FALSE);
        }
        s_block_header* next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
        metadata = &next_header->metadata;
    }
    ft_printf("Releasing page: %p!\n", page);
    return (TRUE);
}

void    reset_arena(s_arena *arena)
{
    arena->initialized = 0;
    pthread_mutex_unlock(&arena->lock);
    if (pthread_mutex_trylock(&arena->lock) == 0)
    {
        pthread_mutex_unlock(&arena->lock);
        pthread_mutex_destroy(&arena->lock);
        return;
    }
}

void    free(void *ptr)
{
    int    assigned_arena = 0;
    s_page **page_iterator = &arena_head[0].page_head;

    if (ptr == NULL)
        return;
    void *block = search_address(ptr, page_iterator, &assigned_arena);
    if (!block)
    {
        pthread_mutex_unlock(&arena_head[assigned_arena].lock);
        return;
    }
    s_block_header *header = GET_HEADER_FROM_BLOCK(block);
    int *metadata = &header->metadata;
    if (!(*metadata & ~ALLOCATED))
    {
        pthread_mutex_unlock(&arena_head[assigned_arena].lock);
        return;
    }
    else
    {
        *metadata ^= ALLOCATED;
        ft_memset(ptr, 0, *metadata);
        defragment_page(*page_iterator); 
        if (check_for_page_release(*page_iterator) == TRUE)
        {
            header = GET_FIRST_HEADER(*page_iterator);
            s_page *page_to_remove = remove_page_node(assigned_arena, *page_iterator);
            munmap(page_to_remove, header->metadata + sizeof(s_page) + 2 * sizeof(s_block_header));
            atomic_fetch_sub(&mapped_mem, 1);
            page_to_remove = NULL; // Likely does not write into page_head correctly
            if (!arena_head[assigned_arena].page_head)
            {
                arena_head[assigned_arena].assigned_threads--;
                if (!arena_head[assigned_arena].assigned_threads)
                    return (reset_arena(&arena_head[assigned_arena]));
                pthread_mutex_unlock(&arena_head[assigned_arena].lock);
                return;
            }
        }
        pthread_mutex_unlock(&arena_head[assigned_arena].lock);
        return;
    }
}
