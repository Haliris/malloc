// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

s_page  *page_head;

int     request_page(int type, long page_size)
{
    if (!page_head)
    {
        page_head = (void*)mmap(NULL,
                                (page_size * type) + sizeof(s_page) + 2 * sizeof(s_block_header),
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
        page_head->free_space = (page_size * type) - sizeof(s_page) - 2 * sizeof(s_block_header);
        page_head->block_head = GET_FIRST_HEADER(page_head);
        page_head->block_head->metadata = page_head->free_space;
        s_block_header* page_footer = (s_block_header*)((char*)page_head + sizeof(s_page) + sizeof(s_block_header) + page_head->free_space);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
    }
    else
    {
        s_page *new_page = (void*)mmap(NULL,
                                (page_size * type) + sizeof(s_page) + 2 * sizeof(s_block_header),
                                PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE,
                                -1, 0);
        if (new_page == MAP_FAILED)
        {
            ft_putstr_fd("Fatal error: ", STDERR_FILENO);
            ft_putnbr_fd(errno, STDERR_FILENO);
            write(2, "\n", 1);
            return (FATAL_ERROR);
        }
        new_page->type = type;
        new_page->free_space = (page_size * type) - sizeof(s_page) - 2 * sizeof(s_block_header);
        new_page->block_head = GET_FIRST_HEADER(new_page);
        new_page->block_head->metadata = new_page->free_space;
        new_page->next = NULL;
        s_block_header* page_footer = (s_block_header*)((char*)new_page + sizeof(s_page) + sizeof(s_block_header) + new_page->free_space);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
        s_page *page_iterator = page_head;
        while (page_iterator->next != NULL)
            page_iterator = page_iterator->next;
        page_iterator->next = new_page;
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
    if (request_page(type, *page_size) == FATAL_ERROR)
        return (FATAL_ERROR);
    return (SUCCESS);
}

void*   allocate_memory(long long size, int *error_status)
{
    s_page *page_iterator = page_head;
    void   *ptr;

    while (page_iterator)
    {
//        if (page_iterator->free_space < (long long)size) // add check with largest free block as well
//        {
            //ft_printf("Malloc: Not enough space in page, skipping\n");
//            page_iterator = page_iterator->next;
//            continue;
        //        
//        }
        int *metadata = &page_iterator->block_head->metadata;
        while (!IS_PAGE_FOOTER(*metadata)) 
        {
            if ((*metadata & ~ALLOCATED) >= size &&
                (*metadata & ALLOCATED) == 0)
            {
                ptr = GET_BLOCK_PTR(metadata);
                long long original_size = *metadata & ~ALLOCATED;
                *metadata = size;
                *metadata |= ALLOCATED;
                s_block_header* next_header = GET_NEXT_HEADER_FROM_BLOCK(ptr, *metadata);
                if (IS_PAGE_FOOTER(next_header->metadata))
                {
                    page_iterator->block_head = GET_FIRST_HEADER(page_iterator);
                    return (ptr);
                }
                else if (next_header->metadata & ALLOCATED)
                {
                    page_head->block_head = next_header;
                    return (ptr);
                }
                if (size < original_size)
                    next_header->metadata = original_size - size - sizeof(s_block_header);
                page_iterator->block_head = next_header;
                return (ptr);
            }
            s_block_header* next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
            metadata = &next_header->metadata;
        }
        page_iterator = page_iterator->next;
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
    if (page_head == NULL)
    {
        if (init_pages(&page_size, size) == FATAL_ERROR)
            return (NULL); //wtf do we do when fatal???
    }
    payload = allocate_memory(size, &error_status);
    if (error_status == NO_GOOD_PAGE)
    {
        print_full_heap(page_head);
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
    (void)av;
    char *luna;
    luna = malloc(14000);
    ft_printf("Luna : %p\n");
    luna = malloc(12827);
    ft_printf("Luna : %p\n");
    luna = realloc(luna, 12);
    ft_printf("Luna : %p\n");
    luna = realloc(luna, 130000); //get Rlimit()
    ft_printf("Luna : %p\n");
    char *test = ft_itoa(123);
    for (size_t i = 0; i < strlen(test); i++)
        write(1, &test[i], 1);
    write(1, "\n", 1);
    char *dup = ft_strdup("Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length Since words vary in length ");
    for (size_t i = 0; i < strlen(dup); i++)
        write(1, &dup[i], 1);
    write(1, "\n", 1);
    char **split = ft_split(dup, ' ');
    free(dup);
    free(test);
    int i = 0;
    while (split[i])
    {
        ft_putstr_fd(split[i], 1);
        write(1, "\n", 1);
        free(split[i]);
        i++;
    }
    free(split);
    void *ptr = malloc(100);
    show_alloc_mem();
    void *re_ptr = realloc(ptr, 42);
    (void)ptr;
    (void)re_ptr;
    void *big_ass_ptr = realloc(re_ptr, 42000);
    free(big_ass_ptr);
    return (0);
}

