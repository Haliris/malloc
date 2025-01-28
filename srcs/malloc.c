// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stdio.h>
#include <string.h>

s_page* page_head = NULL;

void    show_alloc_mem()
{};

void    free(void *ptr)
{
    if (ptr == NULL)
        return;
};

int     request_page(int type, long page_size)
{

    if (!page_head)
    {
        page_head = (void*)mmap(NULL, (page_size * type) + sizeof(s_page), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (page_head == MAP_FAILED)
        {
            ft_putnbr_fd(errno, STDERR_FILENO);
            write(2, "\n", 1);
            return (FATAL_ERROR);
        }
        page_head->block_head = (s_block*) ((char*)page_head + sizeof(s_page));
        page_head->type = type;
        page_head->free_space = (page_size * type) - sizeof(s_page);
    }
    else
    {
        s_page* new_page = (void*)mmap(NULL, (page_size * type) + sizeof(s_page), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (new_page == MAP_FAILED)
            return (FATAL_ERROR); // need to unmap and free everything, so need to find a code to distinguish. But unmapping memory below the caller is dangerous, so should I just do nothing and wait for the free call??
        new_page->block_head = (s_block*) ((char*)new_page + sizeof(s_page));
        new_page->type = type;
        new_page->free_space = page_size - sizeof(s_page);
        new_page->next = NULL;
        s_page* iterator = page_head;
        while (iterator->next != NULL)
            iterator = iterator->next;
        iterator->next = new_page;
    }
    return (SUCCESS);
}

int    init_pages(long* page_size, long requested_size)
{
    int type;
    long p_size = *page_size;
    if (0 == p_size)
    {
        #ifdef __APPLE__ // no osX at school??
            p_size = getpagesize(void);
        #elif __linux__
            p_size = sysconf(_SC_PAGESIZE);
        #endif
        if (p_size <= 0)
        {
            write(STDERR_FILENO, "Fatal error\n", 12);
            return (FATAL_ERROR);
        }
    }
    type = (LARGE * (requested_size >= (LARGE * p_size))
                + SMALL * (requested_size >= (SMALL * p_size))
                + (TINY * (requested_size < SMALL * p_size))
                );
    ft_printf("Type requested: %d\n", type);
    if (FATAL_ERROR == request_page(type, p_size))
        return (FATAL_ERROR);
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

    write(2, "Hi!\n", 4);
    if (size == 0)
        return (NULL);
    if (NULL == page_head)
    {
        if (FATAL_ERROR == init_pages(&page_size, size))
        {
            write(STDERR_FILENO, "Fatal error in init pages\n", 26);
            return (NULL); //wtf do we do when fatal???
        }
        return page_head;
    }
//    else
 //       request_page(size);
    return page_head;
//    void *p = NULL;
//    return (p); //beginning of page actually
};


int main(void)
{
    void *p;
    p = malloc(1);
    if (p)
    {
        write(2, "Successfully got page from kernel!\n", strlen("Successfully got page from kernel!\n"));
        print_page_list(page_head);
    }
    else
    {
        write(2, "Could not get page from kernel...\n", strlen("Could not get page from kernel...\n"));
    } 
    return (0);
}
