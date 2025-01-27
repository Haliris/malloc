// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stdio.h>

s_page* page_head;

void    show_alloc_mem()
{};

void    free(void *ptr)
{
    if (ptr == NULL)
        return;
};

int     request_page(e_zone_type type, long page_size)
{
    if (!page_head)
    {
        page_head->page = (void*)mmap(NULL, page_size * type, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
        if (page_head == MAP_FAILED)
            return (FATAL_ERROR);
        page_head->type = type;
        page_head->free_space = page_size;
    }
    else
    {
        s_page* iterator = page_head;
        while (iterator->next != NULL)
            iterator = iterator->next;
        iterator->next->page = (void*)mmap(NULL, page_size * type, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
        if (iterator->next == MAP_FAILED)
            return (FATAL_ERROR); // need to unmap and free everything, so need to find a code to distinguish
        iterator->type = type;
        iterator->free_space = page_size;
        iterator->next->next = NULL;
    }
    return (SUCCESS);
}

int    init_pages(long* page_size, size_t requested_size)
{
    if (0 == *page_size)
    {
        #ifdef __APPLE__ // no osX at school??
            *page_size = getpagesize(void);
        #elif __linux__
            *page_size = sysconf(_SC_PAGESIZE);
        #endif
        if (*page_size <= 0)
        {
            write(STDERR_FILENO, "Fatal error\n", 12);
            return (FATAL_ERROR);
        }
    }
    if (requested_size >= LARGE * (*page_size))
    {
        if (FATAL_ERROR == request_page(LARGE, *page_size))
            return (FATAL_ERROR);
    }
    else if (requested_size >= SMALL * (*page_size))
    {
        if (FATAL_ERROR == request_page(SMALL, *page_size))
                return (FATAL_ERROR);
    }
    else
    {
        if (FATAL_ERROR == request_page(TINY, *page_size))
                return (FATAL_ERROR);
    }
    return (SUCCESS);
}

void*    realloc(void *ptr, size_t size)
{
    (void)ptr;
    (void)size;
    void *p = NULL;
    return (p); //same as malloc
};

void*    malloc(size_t size)
{
    static long page_size;

    if (size == 0)
        return (NULL);
    if (NULL == page_head)
        if (FATAL_ERROR == init_pages(&page_size, size))
        {
            write(STDERR_FILENO, "Fatal error\n", 12);
            return (NULL); //wtf do we do when fatal???
        }
    void *p = NULL;
    return (p); //beginning of page actually
};
