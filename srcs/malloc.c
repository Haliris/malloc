// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
// https://sourceware.org/glibc/wiki/MallocInternals
// man mallopt
#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

s_arena arena_head[MALLOC_ARENA_MAX];

int     request_page(s_page **page_head, long long type, long page_size)
{
    if (!*page_head)
    {
        *page_head = (void*)mmap(NULL,
                                (page_size * type) + sizeof(s_page) + 2 * sizeof(s_block_header),
                                PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE,
                                -1, 0);
        if (*page_head == MAP_FAILED)
        {
            ft_putstr_fd("Fatal error: ", STDERR_FILENO);
            ft_putnbr_fd(errno, STDERR_FILENO);
            write(2, "\n", 1);
            return (FATAL_ERROR);
        }
        if (type > SMALL)
            type = LARGE;
        (*page_head)->type = type;
        size_t free_space = (page_size * type) - sizeof(s_page) - 2 * sizeof(s_block_header);
        (*page_head)->block_head = GET_FIRST_HEADER(*page_head);
        (*page_head)->block_head->metadata = free_space;
        s_block_header* page_footer = (s_block_header*)((char*)(*page_head) + sizeof(s_page) + sizeof(s_block_header) + free_space);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
//        ft_printf("Request page: Mapped new page %p with block_head at %p and metadata %p and page_footer %p\n", *page_head, (*page_head)->block_head, (*page_head)->block_head->metadata, page_footer);
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
        if (type > SMALL)
            type = LARGE;
        new_page->type = type;
        size_t free_space = (page_size * type) - sizeof(s_page) - 2 * sizeof(s_block_header);
        new_page->block_head = GET_FIRST_HEADER(new_page);
        new_page->block_head->metadata = free_space;
        new_page->next = NULL;
        s_block_header* page_footer = (s_block_header*)((char*)new_page + sizeof(s_page) + sizeof(s_block_header) + free_space);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
        s_page *page_iterator = *page_head; // bad??
//        ft_printf("Request page: Mapped new page %p with block_head at %p and metadata %p and page_footer %p\n", new_page, (new_page)->block_head, (new_page)->block_head->metadata, page_footer);
        while (page_iterator->next != NULL)
            page_iterator = page_iterator->next;
        page_iterator->next = new_page; // bad too??
    }
    return (SUCCESS);
}

long long get_page_type(long page_size, long requested_size)
{
    if (requested_size < page_size * TINY)
        return TINY;
    else if (requested_size >= page_size * TINY && requested_size < page_size * SMALL)
        return SMALL;
    else
        return (requested_size);
}

int    init_pages(s_page **page_head, long* page_size, long requested_size)
{
    long long type = 0;
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
    type = get_page_type(*page_size, requested_size);
    if (request_page(page_head, type, *page_size) == FATAL_ERROR)
        return (FATAL_ERROR);
    return (SUCCESS);
}

void   *allocate_memory(int assigned_arena, long long size, int *error_status)
{
    s_page *page_iterator = arena_head[assigned_arena].page_head;
    void   *ptr;

    if (!page_iterator)
    {
        *error_status = NO_GOOD_PAGE;
        return (NULL);
    }
    while (page_iterator)
    {
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
                    page_iterator->block_head = next_header; // check that this writes correctly into the page
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

int init_arena(s_arena *arena, long *page_size, long requested_size)
{
    pthread_mutex_lock(&arena->lock);
    if (init_pages(&arena->page_head, page_size, requested_size) == FATAL_ERROR)
    {
        pthread_mutex_destroy(&arena->lock);
        return (FATAL_ERROR);
    }
    //arena_head->size =  //will need to reimplement free_space tracking per pages
    arena->initialized = TRUE;
    pthread_mutex_unlock(&arena->lock);
    return (SUCCESS);
}

int    assign_arena(int *assigned_arena, long *page_size, long requested_size)
{
    int i = 0;

    while (i < MALLOC_ARENA_MAX)
    {
        int ret = pthread_mutex_lock(&arena_head[i].lock);
        if (ret != 0)
        {
            if(ret == EINVAL)
            {
                if (pthread_mutex_init(&arena_head[i].lock, NULL) != 0)
                    return (FATAL_ERROR);
            } 
        }
        if(arena_head[i].assigned_threads > 5)
        {
            pthread_mutex_unlock(&arena_head[i].lock);
            i++;
        }
        else
            break;
    }
    *assigned_arena = i;
    if (arena_head[i].initialized == FALSE)
        return (init_arena(&arena_head[i], page_size, requested_size));
    pthread_mutex_unlock(&arena_head[i].lock);
    return (SUCCESS);
}

void    *malloc(size_t size)
{
    static          long page_size;
    int             *assigned_arena = get_assigned_arena();
    void            *payload;
    int             error_status = 0;

    if (size == 0)
        return (NULL);
    if (size > LLONG_MAX - 7)
        return (NULL); //Add a custom error message and explain in defense the limitation, although why allocated 1048576 teras???
    if (size % 8 != 0)
        size = ROUND_TO_8(size);
    if (arena_head[0].initialized == FALSE)
    {
        if (pthread_mutex_init(&arena_head[0].lock, NULL) != 0)
            return (NULL);
        if (init_arena(&arena_head[0], &page_size, size) == FATAL_ERROR)
            return (NULL);
        *assigned_arena = 0;
    }
    else if (*assigned_arena == -1)
    {
        if (assign_arena(assigned_arena, &page_size, size) == FATAL_ERROR)
            return (NULL); // Not correct, look into what needs to be done
    }
    pthread_mutex_lock(&arena_head[*assigned_arena].lock);
    payload = allocate_memory(*assigned_arena, size, &error_status);
    pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
    if (error_status == NO_GOOD_PAGE)
    {
        long long type = get_page_type(page_size, size);
        pthread_mutex_lock(&arena_head[*assigned_arena].lock);
        if (request_page(&arena_head[*assigned_arena].page_head, type, page_size) == FATAL_ERROR)
            return (NULL);
        error_status = 0;
        payload = allocate_memory(*assigned_arena, size, &error_status);
        pthread_mutex_unlock(&arena_head[*assigned_arena].lock);
    }
    return (payload);
};

#include <assert.h>

#define NUM_THREADS 10
#define NUM_ALLOCS  10

pthread_mutex_t print_stick;
int thread_nb = 10;

void *thread_func(void *arg) {
    void *ptrs[NUM_ALLOCS];
    
    // Allocate memory
    (void)arg;
    for (int i = 0; i < NUM_ALLOCS; i++) {
        pthread_mutex_lock(&print_stick);
        ft_printf("Doing allocations!\n");
        pthread_mutex_unlock(&print_stick);
        ptrs[i] = malloc(64);
        assert(ptrs[i] != NULL);
    }
    
    // Reallocate memory
    //for (int i = 0; i < NUM_ALLOCS; i++) {
    //    pthread_mutex_lock(&print_stick);
    //    ft_printf("Doing reallocations!\n");
    //    pthread_mutex_unlock(&print_stick);
    //    ptrs[i] = realloc(ptrs[i], 128);
    //    assert(ptrs[i] != NULL);
    //}
    
    // Free memory
    //for (int i = 0; i < NUM_ALLOCS; i++) {
    //    pthread_mutex_lock(&print_stick);
    //    ft_printf("Doing frees!\n");
    //    pthread_mutex_unlock(&print_stick);
    //    free(ptrs[i]);
    //}
    
    pthread_mutex_lock(&print_stick);
    ft_printf("Thread exiting#\n");
    ft_printf("Threads left: %d\n", thread_nb--);
    pthread_mutex_unlock(&print_stick);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    
    // Spawn threads
    pthread_mutex_init(&print_stick, NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        assert(pthread_create(&threads[i], NULL, thread_func, NULL) == 0);
    }
    
    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&print_stick);
    printf("Thread-safe malloc test passed!\n");
    return 0;
}
