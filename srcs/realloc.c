#include "../includes/malloc.h"

extern s_arena arena_head[MALLOC_ARENA_MAX];

void    downsize_block(int* metadata, size_t size, size_t block_size)
{
    *metadata = size;
    s_block_header *next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
    *metadata |= ALLOCATED;
    next_header->metadata = block_size - size - sizeof(s_block_header);
}

int  extend_block(int *metadata, s_page **page_iterator, size_t block_size, size_t size)
{
    while (!IS_PAGE_FOOTER(*metadata))
    {
        s_block_header *next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
        if ((next_header->metadata & ALLOCATED))
            break;
        else
        {
            if (next_header == (*page_iterator)->block_head)
                (*page_iterator)->block_head = (s_block_header*) ((char*)metadata);
            *metadata = *metadata + next_header->metadata + sizeof(s_block_header);
            block_size = *metadata & ~ALLOCATED;
            next_header->metadata = 0;
        }
        if (block_size == size)
            return (SUCCESS);
        else if (block_size > size)
        {
            *metadata = size;
            next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
            *metadata |= ALLOCATED;
            next_header->metadata = block_size - size - sizeof(s_block_header);
            return (SUCCESS);
        }
    }
    return (FAILURE);
}

//void    *find_new_block(void *ptr, size_t size, size_t ptr_size)
//{
//    void *payload = malloc(size);
//    if (!payload)
//    {
//        free(ptr);
//        return (NULL);
//    }
//    payload = ft_memmove(payload, ptr, ptr_size);
//    free(ptr);
//    return (payload);
//}

void    *realloc(void *ptr, size_t size)
{
    int *assigned_arena = get_assigned_arena();
    pthread_mutex_lock(&arena_head[*assigned_arena].lock);
    s_page  **page_iterator = &arena_head[*assigned_arena].page_head;
    size_t  ptr_size = 0;

    if (size == 0 && ptr)
    {
        free(ptr);
        pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
        return (NULL);
    }
    if (size > LLONG_MAX - 7)
    {
        pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
        return (NULL); //Add a custom error message and explain in defense the limitation, although why allocated 1048576 teras???
    }
    if (size % 8 != 0)
        size = ROUND_TO_8(size);
    if (!ptr)
    {
        pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
        return(malloc(size));
    }
    void *block = search_address(ptr, page_iterator);
    if (!block)
    {
        pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
        return (malloc(size));
    }
    s_block_header *header = GET_HEADER_FROM_BLOCK(block);
    int *metadata = &header->metadata;
    size_t block_size = *metadata & ~ALLOCATED;
    ptr_size = block_size;
    if ((long long)size < (*metadata & ~ALLOCATED))
    {
        downsize_block(metadata, size, block_size);
        pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
        return (ptr);
    }
    if (extend_block(metadata, page_iterator, block_size, size) == SUCCESS)
    {
        pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
        return (ptr);
    }
    void *payload = malloc(size);
    if (!payload)
    {
        free(ptr);
        return (NULL);
    }
    pthread_mutex_lock(&arena_head[*assigned_arena].lock);
    payload = ft_memmove(payload, ptr, ptr_size);
    pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
    free(ptr);
    return (payload);
};
