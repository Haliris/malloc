// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
#include "../includes/malloc.h"
#include "../includes/data_structs.h"

void    free(void *ptr)
{
    if (ptr == NULL)
        return;
};

void init_memory(t_heap *heap_head)
{
    heap_head = mmap(NULL, );
}

void    *malloc(size_t size)
{
    static t_heap *heap_head;
    if (size == 0)
        return (NULL);
    if (!heap_head)
        init_memory(heap_head);

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
