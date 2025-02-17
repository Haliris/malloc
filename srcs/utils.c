#include "../includes/malloc.h"

extern s_arena arena_head[MALLOC_ARENA_MAX];

int *get_assigned_arena(void)
{
    static __thread int assigned_arena = -1;

    return (&assigned_arena);
}

void    show_alloc_mem()
{
    size_t  total_bytes = 0;
    for (int i = 0; i < MALLOC_ARENA_MAX && arena_head[i].initialized; i++)
    {
//        pthread_mutex_lock(&arena_head[i].lock);
        s_page  *page_iterator = arena_head[i].page_head;
        ft_printf("Arena: %d\n", i);
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
                ft_printf("%p - %p: %d bytes ", block_ptr, next_header, (*metadata & ~ALLOCATED));
                if (*metadata & ALLOCATED)
                    ft_printf("| Allocated\n");
                else
                    ft_printf("| Free\n");
                total_bytes += *metadata & ~ALLOCATED;
                metadata = &next_header->metadata;
            }
            page_iterator = page_iterator->next;
        }
 //       pthread_mutex_unlock(&arena_head[i].lock);
    }
    ft_printf("Total: ");
    ft_putnbr_fd(total_bytes, STDOUT_FILENO);
    ft_putendl_fd(" bytes", STDOUT_FILENO);
}

s_page  *remove_page_node(int assigned_arena, s_page *released_page)
{
    s_page *page_iterator = arena_head[assigned_arena].page_head;

    if (!released_page)
        return (NULL);
    if (arena_head[assigned_arena].page_head == released_page)
    {
        if (page_iterator->next)
            arena_head[assigned_arena].page_head = page_iterator->next;
        else
            arena_head[assigned_arena].page_head = NULL;
        return (released_page);
    }
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
    return (released_page);
}

void    *search_address(void *ptr, s_page **page_iterator, int *arena_nb)
{
    int    i = 0;
    s_page *current_page;

    while (i < MALLOC_ARENA_MAX)
    {
        if (pthread_mutex_lock(&arena_head[i].lock) == EINVAL)
            break;
        current_page = arena_head[i].page_head;
        if (!current_page)
            return (NULL);
        while (current_page)
        {
            s_block_header *header = GET_FIRST_HEADER(current_page);
            int *metadata = &header->metadata;
            while (!IS_PAGE_FOOTER(*metadata))
            {
                void *block = GET_BLOCK_PTR(metadata);
                if (block == ptr)
                {
                    arena_nb = &i;
                    *page_iterator = current_page;
                    return (block);
                }
                else
                {
                    s_block_header* next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
                    metadata = &next_header->metadata;
                }
            }
            current_page = current_page->next;
        }
        pthread_mutex_unlock(&arena_head[i].lock);
        i++;
    }
    return (NULL);
};
