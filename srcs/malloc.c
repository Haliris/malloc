// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stdio.h>
#include <string.h>

s_page* page_head;
int     pages_mapped;
int     pages_released;

void    show_alloc_mem()
{};

int     request_page(int type, long page_size)
{
    if (!page_head)
    {
        //ft_printf("Requesting page with size: %d\n", page_size * type);
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
        //ft_printf("Page address at: %p\n", page_head);
        //ft_printf("Page footer set at address: %p\n", page_footer);
        //ft_printf("Page footer at this distance from head: %d\n", (char*)page_footer - (char*)page_head);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
        //ft_printf("Page footer metadata size: %d\n", page_footer->metadata & ~ALLOCATED);
        //ft_print_bits(page_footer->metadata);
        //ft_printf("Page head metadata set at: %p\n", page_head->block_head);
        //ft_printf("Page head metadata value: %d\n", page_head->block_head->metadata);
        ft_printf("------\nRequested page_head: %p\n------\n", page_head);
        pages_mapped++;
    }
    else
    {
        //ft_printf("Requesting page with size: %d\n", page_size * type);
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
        //ft_printf("Page address at: %p\n", new_page);
        //ft_printf("Page footer set at address: %p\n", page_footer);
        //ft_printf("Page footer at this distance from head: %d\n", (char*)page_footer - (char*)new_page);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
        //ft_printf("Page footer metadata size: %d\n", page_footer->metadata & ~ALLOCATED);
        //ft_print_bits(page_footer->metadata);
        //ft_printf("Page head metadata set at: %p\n", new_page->block_head);
        //ft_printf("Page head metadata value: %d\n", new_page->block_head->metadata);
        s_page *page_iterator = page_head;
        while (page_iterator->next != NULL)
            page_iterator = page_iterator->next;
        page_iterator->next = new_page;
        ft_printf("------\nRequested new_page: %p\n------\n", new_page);
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
    //ft_printf("Type requested is: %d\n", type);
    if (request_page(type, *page_size) == FATAL_ERROR)
        return (FATAL_ERROR);
    return (SUCCESS);
}

void    remove_page_node(s_page *released_page)
{
    s_page *page_iterator = page_head;

    if (!released_page)
        return;
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
}

void    *search_address(void *ptr, s_page **page_iterator)
{
    while (*page_iterator)
    {
        s_block_header *header = GET_FIRST_HEADER(*page_iterator);
        int *metadata = &header->metadata;
        while (1)
        {
            void *block = GET_BLOCK_PTR(metadata);
            //ft_printf("search_address: examining block: %p\n", block);
            if (IS_PAGE_FOOTER(*metadata))
            {
                //ft_printf("search_address: Reached page footer, moving on to next page...\n");
                break;
            }
            if (block == ptr)
            {
                ft_printf("Search address: found block %p in page %p\n", block, *page_iterator);
                return block;
            }
            else
            {
                s_block_header* next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
                //ft_printf("search_address: moving from header %p to next header: %p\n", metadata, &next_header->metadata);
                //ft_printf("search_address: Operation to move to next pointer: %d\n",(char*)metadata + ((*metadata & ~ALLOCATED) + sizeof(s_block_header)));
                metadata = &next_header->metadata;
            }
        }
        *page_iterator = (*page_iterator)->next;
    }
    return (NULL);
}

void    *realloc(void *ptr, size_t size)
{
    void    *payload;
    s_page  **page_iterator = &page_head;

    if (size == 0 && ptr)
    {
        free(ptr);
        return (NULL);
    }
    if (size > LLONG_MAX - 7)
        return (NULL); //Add a custom error message and explain in defense the limitation, although why allocated 1048576 teras???
    if (size % 8 != 0)
        size = ROUND_TO_8(size);
    ft_printf("Realloc: Requested size: %d\n", size);
    if (!ptr)
    {
        payload = malloc(size);
        if (!payload)
            return (NULL);
    }
    else
    {
        void *block = search_address(ptr, page_iterator);
        if (!block)
        {
            payload = malloc(size);
            if (!payload)
                return (NULL);
        };
        s_block_header *header = GET_HEADER_FROM_BLOCK(block);
        ft_printf("Realloc: header set at %p, metadata: %d\n", header, header->metadata & ~ALLOCATED);
        int *metadata = &header->metadata;
        size_t block_size = *metadata & ~ALLOCATED;
        ft_printf("Realloc: Current block %p size: %d\n", header, *metadata & ~ALLOCATED);
        if ((long long)size < (*metadata & ~ALLOCATED))
        {
            *metadata = size;
            s_block_header *next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
            *metadata |= ALLOCATED;
            next_header->metadata = block_size - size - sizeof(s_block_header);
            ft_printf("Realloc: page: %p Size requested is SMALLER than existing block, SPLITTING THEN returning new pointer: %p, header: %p with size: %d\n", *page_iterator, ptr, metadata, *metadata & ~ALLOCATED);
            return (ptr);
        }
        while (1)
        {
            if (IS_PAGE_FOOTER(*metadata))
            {
                ft_printf("Realloc: Reached page footer\n");
                break;
            }
            s_block_header *next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
            if ((next_header->metadata & ALLOCATED))
            {
                ft_printf("Realloc: page: %p Next block %p this far from current header %d is allocated\n", *page_iterator, next_header, (char*)next_header - (char*)metadata);
                break;
            }
            else
            {
                (*page_iterator)->free_space -= *metadata;
                if (next_header == (*page_iterator)->block_head)
                    (*page_iterator)->block_head = (s_block_header*) ((char*)metadata);
                ft_printf("Realloc: page: %p Merging block %p and block %p, size %d and %d\n", *page_iterator, metadata, next_header, *metadata & ~ALLOCATED, next_header->metadata & ~ALLOCATED);
                *metadata = *metadata + next_header->metadata + sizeof(s_block_header);
                block_size = *metadata & ~ALLOCATED;
                next_header->metadata = 0;
                (*page_iterator)->free_space += *metadata;
                ft_printf("Realloc: block_size evaluation now at: %d\n", block_size);
            }
            if (block_size == size)
            {
                ft_printf("Realloc: page: %p Managed to extend block to fit EXACTLY new requested size, returning new pointer: %p, header: %p with size: %d\n", *page_iterator, ptr, metadata, *metadata & ~ALLOCATED);
                return (ptr);
            }
            else if (block_size > size)
            {
                *metadata = size;
                next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
                *metadata |= ALLOCATED;
                next_header->metadata = block_size - size - sizeof(s_block_header);
                ft_printf("Realloc: Managed to extend block to fit new requested size, SPLITTING THEN returning new pointer: %p, header: %p with size: %d\n", ptr, metadata, *metadata & ~ALLOCATED);
                return (ptr);
            }
        }
        ft_printf("Realloc: Could not extend block, calling free and malloc\n");
        free(ptr);
        return (malloc(size));
    }
    return (payload);
};

void    coalesce_blocks(s_page* page)
{
    s_block_header *header = GET_FIRST_HEADER(page);
    int *metadata = &header->metadata;
    while (1)
    {
        if (IS_PAGE_FOOTER(*metadata))
            {
                //ft_printf("Coalesce: Reached page footer\n");
                break;
            }
        void *ptr = GET_BLOCK_PTR(metadata);
        s_block_header* next_header = GET_NEXT_HEADER_FROM_BLOCK(ptr, *metadata);
        //ft_printf("Coalesce: Next header set at %p\n", next_header);
        //ft_printf("Coalesce: Next header metadata value at %d\n", next_header->metadata & ~ALLOCATED);
        //ft_printf("Coalesce: Next header allocated status: %d\n", next_header->metadata & ALLOCATED);
        if (*metadata & ALLOCATED)
        {
            ////ft_printf("Coalesce: current block %p marked as allocated, skipping\n", metadata);
            metadata = &next_header->metadata;
            continue;
        }
        if (!(next_header->metadata & ALLOCATED))
        {
            if (next_header == page->block_head)
                page->block_head = (s_block_header*) ((char*)metadata);
            //ft_printf("Coalesce: current block %p and next block %p are free, merging...\n", metadata, next_header);
            page->free_space -= *metadata;
            //ft_printf("Coalesce: metadata used to be %d\n", *metadata);
            //ft_printf("Coalesce: Doing operation : %d + %d + %d\n", *metadata, next_header->metadata, sizeof(s_block_header));
            *metadata = *metadata + next_header->metadata + sizeof(s_block_header);
            //ft_printf("Coalesce: metadata now is %d at adress %p\n", *metadata, metadata);
            next_header->metadata = 0;
            page->free_space += *metadata;
        }
        else
            metadata = &next_header->metadata;
    }
}

int    check_for_page_release(s_page *page)
{
    s_block_header *header = GET_FIRST_HEADER(page);
    int *metadata = &header->metadata;
    while (1)
    {
        if (IS_PAGE_FOOTER(*metadata))
        {
            //ft_printf("Check for page release: Page is empty, releasing...\n");
            return (TRUE);
        }
        else if (*metadata & ALLOCATED)
        {
            //ft_printf("Check for page release: Page is not empty.\n");
            return (FALSE);
        }
        s_block_header* next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
        metadata = &next_header->metadata;
    }
}

void    free(void *ptr)
{
    s_page **page_iterator = &page_head;
    if (ptr == NULL)
    {
        return;
    }
    void *block = search_address(ptr, page_iterator);
    if (!block)
    {
        return;
    }
    s_block_header *header = GET_HEADER_FROM_BLOCK(block);
    int *metadata = &header->metadata;
    ft_printf("Free: Found block %p in page %p\n", block, *page_iterator);
    if (!(*metadata & ~ALLOCATED))
    {
        ft_printf("Free: Found block %p is NOT allocated, aborting\n", block);
        return;
    }
    else
    {
        *metadata ^= ALLOCATED;
        ft_memset(ptr, 0, *metadata);
        coalesce_blocks(*page_iterator); 
        //print_page_memory(*page_iterator);
        if (check_for_page_release(*page_iterator) == TRUE)
        {
            header = GET_FIRST_HEADER(*page_iterator);
            ft_printf("Free: releasing page %p with size %d\n", *page_iterator, header->metadata + sizeof(s_page) + 2 *sizeof(s_block_header));
            remove_page_node(*page_iterator);
            munmap(*page_iterator, header->metadata + sizeof(s_page) + 2 * sizeof(s_block_header));
            *page_iterator = NULL;
            pages_released++;
        }
        return;
    }
}

void*   allocate_memory(long long size, int *error_status)
{
    s_page *page_iterator = page_head;
    void   *ptr;

    //ft_printf("----\n Allocating memory\n----\n");
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
        while (1) 
        {
            if (IS_PAGE_FOOTER(*metadata))
            {
                //ft_printf("Malloc: Reached page footer, moving on to next page...\n");
                break;
            }
            if ((*metadata & ~ALLOCATED) >= size &&
                (*metadata & ALLOCATED) == 0)
            {
                ptr = GET_BLOCK_PTR(metadata);
                //ft_printf("Malloc: ptr: %p\n", ptr);
                long long original_size = *metadata & ~ALLOCATED;
                //ft_printf("Malloc: Original size of page before alloc: %d\n", original_size);
                *metadata = size;
                //ft_printf("Malloc: Metadata of ptr: %d\n", *metadata);
                *metadata |= ALLOCATED;
                //ft_printf("Malloc: Operation to find next header: %p + %d\n", ptr, *metadata & ~ALLOCATED);
                s_block_header* next_header = GET_NEXT_HEADER_FROM_BLOCK(ptr, *metadata);
                //ft_printf("Malloc: Next_header address: %p\n", next_header);
                //ft_printf("Malloc: Next header this far from page_head: %d\n", (char*)next_header - (char*)page_iterator);
                //ft_printf("Malloc: Next header this far from ptr: %d\n", (char*)next_header - (char*)ptr);
                if (IS_PAGE_FOOTER(next_header->metadata))
                {
                    //ft_printf("Malloc: Page footer encountered, resetting page_iterator from: %p to %p\n", page_iterator->block_head, (s_block_header*)((char*)page_iterator + sizeof(s_page) + sizeof(s_block_header)));
                    page_iterator->block_head = GET_FIRST_HEADER(page_iterator);
          //          print_page_memory(page_iterator);
                    return (ptr);
                }
                else if (next_header->metadata & ALLOCATED)
                {
                    page_head->block_head = next_header;
          //          print_page_memory(page_iterator);
                    return (ptr);
                }
                if (size < original_size)
                    next_header->metadata = original_size - size - sizeof(s_block_header);
                page_iterator->block_head = next_header;
                //ft_printf("Malloc: New page_iterator header cursor set at: %p\n", page_iterator->block_head);
                //ft_printf("Malloc: Metadata of next address: %d\n", page_iterator->block_head->metadata);
   //             print_page_memory(page_iterator);
                return (ptr);
            }
            //ft_printf("Malloc: Moving from header %p by: %d\n", metadata, ((*metadata & ~ALLOCATED) + sizeof(s_block_header)));
            s_block_header* next_header = GET_NEXT_HEADER_FROM_HEADER(metadata);
            //ft_printf("Malloc: Header now at: %p\n", next_header);
            metadata = &next_header->metadata;
            //ft_printf("Malloc: Metadata pointer now this far from head: %d\n", (char*)metadata - (char*)page_iterator);
        }
        page_iterator = page_iterator->next;
        //ft_printf("Malloc: Moving on to next page...\n");
    }
    //ft_printf("Malloc: No good page found, returning NO_GOOD_PAGE code\n");
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
    //ft_printf("requested alloc size: %d\n", size);
    //ft_printf("Size of page header: %d\n", sizeof(s_page));
    if (page_head == NULL)
    {
        ft_printf("Initializing page head page\n");
        if (init_pages(&page_size, size) == FATAL_ERROR)
        {
            write(STDERR_FILENO, "Fatal error in init pages\n", 26);
            return (NULL); //wtf do we do when fatal???
        }
    }
    payload = allocate_memory(size, &error_status);
    if (error_status == NO_GOOD_PAGE)
    {
        ft_printf("Malloc Requested new page\n");
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
    ft_printf("------\nPrinting page memory before problem reallocs!\n------\n");
    void *re_ptr = realloc(ptr, 42);
    (void)ptr;
    (void)re_ptr;
    void *big_ass_ptr = realloc(re_ptr, 42000);
    free(big_ass_ptr);
    if (pages_released != pages_mapped)
        ft_printf("Pages still in memory! %d in memory vs %d released\n", pages_mapped, pages_released);
    else
        ft_printf("All pages released!!!\n");
    return (0);
}

