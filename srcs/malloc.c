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
        page_head = (void*)mmap(NULL,
                                (page_size * type) + sizeof(s_page) + sizeof(int) + sizeof(s_block_header),
                                PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE,
                                -1, 0);
        if (page_head == MAP_FAILED)
        {
            ft_putstr_fd("Fatal error: ", STDERR_FILENO);
            ft_putnbr_fd(errno, STDERR_FILENO);
            write(2, "\n", 1);
            return (FATAL_ERROR);
        }
        page_head->type = type;
        page_head->free_space = (page_size * type) - sizeof(s_page) - sizeof(int) - sizeof(s_block_header);
        page_head->block_head = (s_block_header*) ((char*)page_head + sizeof(s_page));
        page_head->block_head->metadata = page_head->free_space;
        int* page_footer = (int*)(char*)page_head + page_size - sizeof(int);
        *page_footer = 0 & ALLOCATED;
    }
    else
    {
        s_page* new_page = (void*)mmap(NULL,
                                       (page_size * type) + sizeof(s_page) + sizeof(int) + sizeof(s_block_header),
                                       PROT_READ | PROT_WRITE,
                                       MAP_ANONYMOUS | MAP_PRIVATE,
                                       -1, 0);
        if (new_page == MAP_FAILED)
        {
            ft_putstr_fd("Fatal error: ", STDERR_FILENO);
            ft_putnbr_fd(errno, STDERR_FILENO);
            write(2, "\n", 1);
            return (FATAL_ERROR); // need to unmap and free everything, so need to find a code to distinguish. But unmapping memory below the caller is dangerous, so should I just do nothing and wait for the free call??
        }
        new_page->type = type;
        new_page->free_space = (page_size * type) - sizeof(s_page) - sizeof(int) - sizeof(s_block_header);
        new_page->next = NULL;
        new_page->block_head = (s_block_header*)((char*)new_page + sizeof(s_page));
        new_page->block_head->metadata = new_page->free_space;
        int* page_footer = (int*)(char*)page_head + page_size - sizeof(int);
        *page_footer = 0 & ALLOCATED;

        s_page* iterator = page_head;
        while (iterator->next != NULL)
            iterator = iterator->next;
        iterator->next = new_page;
    }
    return (SUCCESS);
}

int    init_pages(long* page_size, long requested_size)
{
    int type = 0;
    if (*page_size == 0)
    {
        #ifdef __APPLE__ // no osX at school??
            p_size = getpagesize(void);
        #elif __linux__
            *page_size = sysconf(_SC_PAGESIZE);
        #endif
        if (*page_size <= 0)
        {
            write(STDERR_FILENO, "Fatal error\n", 12);
            return (FATAL_ERROR);
        }
    }
    type += IS_LARGE_TYPE(requested_size, *page_size);
    type += IS_SMALL_TYPE(requested_size, *page_size);
    type += IS_TINY_TYPE(requested_size, *page_size);
    ft_printf("Type requested is: %d\n", type);
    if (request_page(type, *page_size) == FATAL_ERROR)
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

void*   allocate_memory(long long size, int *error_status)
{
    s_page *iterator = page_head;
    void   *ptr;
    while (iterator)
    {
        if (iterator->free_space < (long long)size) // add check with largest free block as well
        {
            iterator = iterator->next;
            continue;
        }
        int* metadata = &iterator->block_head->metadata;
        while (1) 
        {
            if (((*metadata & ~ALLOCATED) == 0) &&
                *metadata & ALLOCATED)// End of the page if 00000.....001
                break;
            if ((*metadata & ~ALLOCATED) >= size &&
                (*metadata & ALLOCATED) == 0)
            {
                ptr = (void*) ((char*)metadata + sizeof(s_block_header));
                long long original_size = *metadata & ~ALLOCATED;
                *metadata = size;
                *metadata |= ALLOCATED;
                ft_printf("Metadata of ptr: %p\n", metadata);
                ft_printf("ptr: %p\n", ptr);
                s_block_header* next_header = (s_block_header*)((char*)metadata + (*metadata & ~ALLOCATED));
                if ((next_header->metadata & ~ALLOCATED) == 0 &&
                    (next_header->metadata & ALLOCATED))
                {
                    iterator->block_head = (s_block_header*)iterator + sizeof(s_page);
                    return (ptr);
                }
                if (size < original_size)
                    next_header->metadata = original_size - size;
                iterator->block_head = next_header;
                return (ptr);
            }
            metadata += (*metadata & ~ALLOCATED);
        }
        iterator = iterator->next;
    }
    *error_status = NO_GOOD_PAGE;
    return (NULL);
}

void    *malloc(size_t size)
{
    static long page_size;
    void       *payload;
    int         error_status = 0;

    if (size == 0)
        return (NULL);
    if (page_head == NULL)
    {
        if (init_pages(&page_size, size) == FATAL_ERROR)
        {
            write(STDERR_FILENO, "Fatal error in init pages\n", 26);
            return (NULL); //wtf do we do when fatal???
        }
    }
    write(2, "Allocating memory....\n", strlen("Allocating memory....\n"));
    payload = allocate_memory(size, &error_status);
    if (error_status == NO_GOOD_PAGE)
    {
        int type = 0;
        type += IS_LARGE_TYPE((long long)size, page_size);
        type += IS_SMALL_TYPE((long long)size, page_size);
        type += IS_TINY_TYPE((long long)size, page_size);
        if (request_page(type, page_size) == FATAL_ERROR)
            return (NULL);
        error_status = 0;
        payload = allocate_memory(size, &error_status);
    }
    return (payload);
};


int main(int ac, char **av)
{
    if (ac != 2)
    {
        write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
        exit(1);
    }
    size_t  size = atoi(av[1]);
    void    *p;

    p = malloc(size);
    if (p)
    {
   //     print_page_list(page_head);
        s_block_header *header = (s_block_header*)p - 1;
        int metadata = header->metadata;
        write(1, "Metadata of payload: ", strlen("Metadata of payload: "));
        ft_putnbr_fd(metadata, 1);
        write(1, "\n", 1);
        for (size_t i = 0; i < size; i++)
        {
            p++;
            write(1, "Accessing position: ", strlen("Accessing position: "));
            ft_putnbr_fd(i, 1);
            write(1, "\n", 1);
        }
    }
    else
        write(2, "Could not allocate...\n", strlen("Could not allocate...\n"));
    return (0);
}
