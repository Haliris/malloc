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

int     request_map(e_zone_type type, long page_size)
{

}

int    init_pages(long* page_size, size_t requested_size)
{
    if (0 == *page_size)
    {
        #ifdef __APPLE__
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
    if (requested_size >= LARGE * *page_size)
    {
        if (FATAL_ERROR == request_map(LARGE, *page_size))
            return (FATAL_ERROR);
    }
    else if (requested_size >= SMALL * *page_size)
    {
        if (FATAL_ERROR == request_map(SMALL, *page_size))
                return (FATAL_ERROR);
    }
    else
    {
        if (FATAL_ERROR == request_map(TINY, *page_size))
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
            return (NULL); //wtf do we do when fatal???

    void *p = NULL;
    return (p); //beginning of page actually
};
