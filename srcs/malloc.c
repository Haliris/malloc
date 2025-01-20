// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
#include "malloc.h"

void    free(void *ptr)
{
    if (ptr == NULL)
        return;
};

void    *malloc(size_t size)
{
    if (size == 0)
        return (NULL);
    void *p;

    return (p); //beginning of page actually
};

void    *realloc(void *ptr, size_t size)
{
    void *p;

    return (p); //same as malloc
};

void    show_alloc_mem()
{};
