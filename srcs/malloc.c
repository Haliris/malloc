// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stdio.h>

void    free(void *ptr)
{
    if (ptr == NULL)
        return;
};

void init_memory(s_page *page_head)
{
    (void)page_head;
    return ;
}

void    *malloc(size_t size)
{
    (void)size;
    void *p = NULL;
    static s_page* page_head;
    if (size == 0)
        return (NULL);
    if (!page_head)
        init_memory(page_head);
    return (p); //beginning of page actually
};

void    *realloc(void *ptr, size_t size)
{
    (void)ptr;
    (void)size;
    void *p = NULL;
    return (p); //same as malloc
};

void    show_alloc_mem()
{};


int main(void)
{
    printf("Hello world\n");    
    return (0);
}
