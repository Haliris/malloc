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
        s_block_header* page_footer = (s_block_header*)((char*)page_head + page_head->free_space - sizeof(s_block_header));
        ft_printf("Page address at: %p\n", page_head);
        ft_printf("Page footer set at address: %p\n", page_footer);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
        ft_printf("Page footer metadata size: %d\n", page_footer->metadata & ~ALLOCATED);
        ft_print_bits(page_footer->metadata);
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
        s_block_header* page_footer = (s_block_header*)((char*)page_head + new_page->free_space - sizeof(s_block_header)));
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
        ft_printf("Page footer set at address: %p\n", page_footer);
        ft_printf("Page footer metadata size: %d\n", page_footer->metadata & ~ALLOCATED);
        ft_print_bits(page_footer->metadata);
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
    type += IS_LARGE_TYPE(requested_size,(long) (*page_size - sizeof(s_page)));
    type += IS_SMALL_TYPE(requested_size,(long) (*page_size - sizeof(s_page)));
    type += IS_TINY_TYPE(requested_size,(long) (*page_size - sizeof(s_page)));
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
    // Make sure that the size of the block is always a multiple of 8 to keep last bit free
    s_page *iterator = page_head;
    void   *ptr;

    ft_printf("----\n Allocating memory\n----\n");

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
                ft_printf("ptr: %p\n", ptr);
                long long original_size = *metadata & ~ALLOCATED;
                ft_printf("Original size of page before alloc: %d\n", original_size);
                *metadata = size;
                ft_printf("Metadata of ptr: %d\n", *metadata);
                *metadata |= ALLOCATED;
                s_block_header* next_header = (s_block_header*)((char*)metadata + (*metadata & ~ALLOCATED));
                ft_printf("Operation to find next header: metadata + %d\n", *metadata & ~ALLOCATED);
                ft_printf("Next_header address: %p\n", next_header);
                if ((next_header->metadata & ~ALLOCATED) == 0 &&
                    (next_header->metadata & ALLOCATED))
                {
                    iterator->block_head = (s_block_header*)((char*)page_head + sizeof(s_page)); // test that the value is right here
                    return (ptr);
                }
                else if (next_header->metadata & ALLOCATED)
                {
                    // call method to iterate through block headers until one is found free
                    continue;
                }
                if (size < original_size)
                    next_header->metadata = original_size - size;
                iterator->block_head = next_header;
                ft_printf("Metadata of next address: %d\n", next_header->metadata);
                return (ptr);
            }
            ft_printf("Moving metadata pointer by: %d\n", *metadata & ~ALLOCATED);
            metadata = metadata + (*metadata & ~ALLOCATED);
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
    if (size > LLONG_MAX - 7)
        return (NULL); //Add a custom error message and explain in defense the limitation, although why allocated 1048576 teras???
    if (size % 8 != 0)
        size = ROUND_TO_8(size);
    ft_printf("requested alloc size: %d\n", size);
    ft_printf("Size of page header: %d\n", sizeof(s_page));
    if (page_head == NULL)
    {
        if (init_pages(&page_size, size) == FATAL_ERROR)
        {
            write(STDERR_FILENO, "Fatal error in init pages\n", 26);
            return (NULL); //wtf do we do when fatal???
        }
    }
    payload = allocate_memory(size, &error_status);
    if (error_status == NO_GOOD_PAGE)
    {
        int type = 0;
        type += IS_LARGE_TYPE((long long)size, (long)(page_size - sizeof(s_page)));
        type += IS_SMALL_TYPE((long long)size, (long)(page_size - sizeof(s_page)));
        type += IS_TINY_TYPE((long long)size, (long)(page_size - sizeof(s_page)));
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
    void    **p[100];

    (void)size;
    (void)p;
    void *big = malloc(1);
    (void)big;
//    for (int i = 0; i < 100; i++)
//        p[i] = malloc(size * (i + 1));
//    print_page_list(page_head);
//    ft_printf("-----\nPrinting allocated blocks info\n-----\n");
//    for (int k = 0; k < 100; k++)
//        print_block_info(p[k]);
    return (0);
}
