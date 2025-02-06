// https://medium.com/a-42-journey/how-to-create-your-own-malloc-library-b86fedd39b96
// https://my.eng.utah.edu/~cs4400/malloc.pdf
#include "../includes/malloc.h"
#include "../includes/data_structs.h"
#include <stdio.h>
#include <string.h>

s_page* page_head = NULL;

void    show_alloc_mem()
{};

int     request_page(int type, long page_size)
{
    ft_printf("------\nRequest page\n------\n");
    if (!page_head)
    {
        ft_printf("Requesting page with size: %d\n", page_size * type);
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
        page_head->block_head = (s_block_header*) ((char*)page_head + sizeof(s_page));
        page_head->block_head->metadata = page_head->free_space;
        s_block_header* page_footer = (s_block_header*)((char*)page_head + sizeof(s_page) + sizeof(s_block_header) + page_head->free_space);
        ft_printf("Page address at: %p\n", page_head);
        ft_printf("Page footer set at address: %p\n", page_footer);
        ft_printf("Page footer at this distance from head: %d\n", (char*)page_footer - (char*)page_head);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
        ft_printf("Page footer metadata size: %d\n", page_footer->metadata & ~ALLOCATED);
        ft_print_bits(page_footer->metadata);
        ft_printf("Page head metadata set at: %p\n", page_head->block_head);
        ft_printf("Page head metadata value: %d\n", page_head->block_head->metadata);
    }
    else
    {
        ft_printf("Requesting page with size: %d\n", page_size * type);
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
        new_page->block_head = (s_block_header*) ((char*)new_page + sizeof(s_page));
        new_page->block_head->metadata = new_page->free_space;
        new_page->next = NULL;
        s_block_header* page_footer = (s_block_header*)((char*)new_page + sizeof(s_page) + sizeof(s_block_header) + new_page->free_space);
        ft_printf("Page address at: %p\n", new_page);
        ft_printf("Page footer set at address: %p\n", page_footer);
        ft_printf("Page footer at this distance from head: %d\n", (char*)page_footer - (char*)new_page);
        page_footer->metadata = 0;
        page_footer->metadata |= ALLOCATED;
        ft_printf("Page footer metadata size: %d\n", page_footer->metadata & ~ALLOCATED);
        ft_print_bits(page_footer->metadata);
        ft_printf("Page head metadata set at: %p\n", new_page->block_head);
        ft_printf("Page head metadata value: %d\n", new_page->block_head->metadata);
        s_page *iterator = page_head;
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

void    coalesce_blocks(s_page* page)
{
    s_block_header *header = (s_block_header*)((char*)page + sizeof(s_page));
    int *metadata = &header->metadata;
    while (1)
    {
        if (((*metadata & ~ALLOCATED) == 0) &&
                *metadata & ALLOCATED)// End of the page if 00000.....001
            {
                ft_printf("Coalesce: Reached page footer\n");
                break;
            }
        s_block_header* next_header = (s_block_header*)((char*)metadata + ((*metadata & ~ALLOCATED) + sizeof(s_block_header)));
        if (*metadata & ALLOCATED)
        {
            ft_printf("Coalesce: current block %p marked as allocated, skipping\n", metadata);
            metadata = &next_header->metadata;
            continue;
        }
        if (next_header->metadata & ~ALLOCATED)
        {
            ft_printf("Coalesce: current block %p and next block %p are free, merging...\n", metadata, next_header);
            page->free_space -= *metadata;
            ft_printf("Coalesce: metadata used to be %d\n", *metadata);
            *metadata = *metadata + next_header->metadata + sizeof(s_block_header);
            ft_printf("Coalesce: metadata now is %d at adress %p\n", *metadata, metadata);
            next_header->metadata = 0;
            page->free_space += *metadata;
        }
        else
            metadata = &next_header->metadata;
    }
}

int    check_for_page_release(s_page *page)
{
    s_block_header *header = (s_block_header*) page + sizeof(s_page);
    int *metadata = &header->metadata;
    while (1)
    {
        if (((*metadata & ~ALLOCATED) == 0) &&
            *metadata & ALLOCATED)// End of the page if 00000.....001
        {
            ft_printf("Check for page release: Page is empty, releasing...\n");
            return (TRUE);
        }
        else if (*metadata & ALLOCATED)
        {
            ft_printf("Check for page release: Page is not empty.\n");
            return (FALSE);
        }
        s_block_header* next_header = (s_block_header*)((char*)metadata + ((*metadata & ~ALLOCATED) + sizeof(s_block_header)));
        metadata = &next_header->metadata;
    }
}
void    free(void *ptr)
{
    if (ptr == NULL)
        return;
    s_page  *iterator = page_head;
    ft_printf("Free: requesting to free %p\n", ptr);
    while (iterator)
    {
        s_block_header *header = (s_block_header*)((char*)iterator + sizeof(s_page));
        int *metadata = &header->metadata;
        while (1)
        {
            void *block = (void*) ((char*)metadata + sizeof(s_block_header));
            ft_printf("Free: examining block: %p\n", block);
            if (((*metadata & ~ALLOCATED) == 0) &&
                *metadata & ALLOCATED)// End of the page if 00000.....001
            {
                ft_printf("Free: Reached page footer, moving on to next page...\n");
                break;
            }
            if (block == ptr)
            {
                ft_printf("Free: Found block %p in page %p\n", ptr, iterator);
                if (!(*metadata & ~ALLOCATED))
                {
                    ft_printf("Free: Found block %p is NOT allocated, aborting free\n", ptr, iterator);
                    return;
                }
                else
                {
                    *metadata ^= ALLOCATED; // Is the XOR correct?
                    coalesce_blocks(iterator); 
                    if (check_for_page_release(iterator) == TRUE)
                    {
                        ft_printf("Free: releasing page %p with size %d\n", iterator, header->metadata + sizeof(s_page) + 2 *sizeof(s_block_header));
                        ft_printf("Free: released page metadata at address : %p\n", &header->metadata);
                        munmap(iterator, header->metadata + sizeof(s_page) + 2 * sizeof(s_block_header));
                        iterator = NULL;
                    }
                    return;
                }
            }
            else
            {
                s_block_header* next_header = (s_block_header*)((char*)metadata + ((*metadata & ~ALLOCATED) + sizeof(s_block_header)));
                ft_printf("Free: moving from header %p to next header: %p\n", metadata, &next_header->metadata);
                ft_printf("Free: Operation to move to next pointer: %d\n",(char*)metadata + ((*metadata & ~ALLOCATED) + sizeof(s_block_header)));
                metadata = &next_header->metadata;
            }
        }
        iterator = iterator->next;
    }
};

void*   allocate_memory(long long size, int *error_status)
{
    s_page *iterator = page_head;
    void   *ptr;

    ft_printf("----\n Allocating memory\n----\n");
    while (iterator)
    {
        if (iterator->free_space < (long long)size) // add check with largest free block as well
        {
            ft_printf("Malloc: Not enough space in page, skipping\n");
            iterator = iterator->next;
            continue;
        }
        int *metadata = &iterator->block_head->metadata;
        while (1) 
        {
            if (((*metadata & ~ALLOCATED) == 0) &&
                *metadata & ALLOCATED)// End of the page if 00000.....001
            {
                ft_printf("Malloc: Reached page footer, moving on to next page...\n");
                break;
            }
            if ((*metadata & ~ALLOCATED) >= size &&
                (*metadata & ALLOCATED) == 0)
            {
                ptr = (void*) ((char*)metadata + sizeof(s_block_header));
                ft_printf("Malloc: ptr: %p\n", ptr);
                long long original_size = *metadata & ~ALLOCATED;
                ft_printf("Malloc: Original size of page before alloc: %d\n", original_size);
                *metadata = size;
                ft_printf("Malloc: Metadata of ptr: %d\n", *metadata);
                *metadata |= ALLOCATED;
                ft_printf("Malloc: Operation to find next header: %p + %d\n", ptr, *metadata & ~ALLOCATED);
                s_block_header* next_header = (s_block_header*)((char*)ptr + (*metadata & ~ALLOCATED));
                ft_printf("Malloc: Next_header address: %p\n", next_header);
                ft_printf("Malloc: Next header this far from page_head: %d\n", (char*)next_header - (char*)iterator);
                ft_printf("Malloc: Next header this far from ptr: %d\n", (char*)next_header - (char*)ptr);
                if ((next_header->metadata & ~ALLOCATED) == 0 &&
                    (next_header->metadata & ALLOCATED))
                {
                    ft_printf("Malloc: Page footer encountered, resetting iterator from: %p to %p\n", iterator->block_head, (s_block_header*)((char*)iterator + sizeof(s_page) + sizeof(s_block_header)));
                    iterator->block_head = (s_block_header*)((char*)iterator + sizeof(s_page)); // test that the value is right here
                    return (ptr);
                }
                else if (next_header->metadata & ALLOCATED)
                {
                    page_head->block_head = next_header;
                    return (ptr);
                }
                if (size < original_size)
                    next_header->metadata = original_size - size - sizeof(s_block_header);
                iterator->block_head = next_header;
                ft_printf("Malloc: New iterator header cursor set at: %p\n", iterator->block_head);
                ft_printf("Malloc: Metadata of next address: %d\n", iterator->block_head->metadata);
                return (ptr);
            }
            ft_printf("Malloc: Moving from header %p by: %d\n", metadata, ((*metadata & ~ALLOCATED) + sizeof(s_block_header)));
            s_block_header* next_header = (s_block_header*)((char*)metadata + ((*metadata & ~ALLOCATED) + sizeof(s_block_header)));
            ft_printf("Malloc: Header now at: %p\n", next_header);
            metadata = &next_header->metadata;
            ft_printf("Malloc: Metadata pointer now this far from head: %d\n", (char*)metadata - (char*)iterator);
        }
        iterator = iterator->next;
        ft_printf("Malloc: Moving on to next page...\n");
    }
    ft_printf("Malloc: No good page found, returning NO_GOOD_PAGE code\n");
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
    ft_printf("Size of block header: %d\n", sizeof(s_block_header));
    (void)av;

    print_page_list(page_head);
    ft_printf("-----\nPrinting allocated blocks info\n-----\n");
    char *test = ft_itoa(123);
    for (size_t i = 0; i < strlen(test); i++)
        write(1, &test[i], 1);
    write(1, "\n", 1);
    free(test);
//    char *dup = ft_strdup("Since words vary in length, for the purpose of measurement of text entry the definition of each word is often standardized to be five characters or keystrokes long in English,[1] including spaces and punctuation. For example, under such a method applied to plain English text the phrase I run counts as one word, but rhinoceros and let's talk would both count as two. Karat et al. found in one study of average computer users in 1999 that the average rate for transcription was 32.5 words per minute, and 19.0 words per minute for composition.[2] In the same study, when the group was divided into fast, moderate, and slow groups, the average speeds were 40 wpm, 35 wpm, and 23 wpm, respectively. With the onset of the era of desktop computers and smartphones, fast typing skills became much more widespread. As of 2019, the average typing speed on a mobile phone was 36.2 wpm with 2.3% uncorrected errors—there were significant correlations with age, level of English proficiency, and number of fingers used to type.[3] Some typists have sustained speeds over 200 wpm for a 15-second typing test with simple English words.[4] Typically, professional typists type at speeds of 43 to 80 wpm, while some positions can require 80 to 95 (usually the minimum required for dispatch positions and other time-sensitive typing jobs), and some advanced typists work at speeds above 120 wpm.[5] Two-finger typists, sometimes also referred to as hunt and peck typists, commonly reach sustained speeds of about 37 wpm for memorized text and 27 wpm when copying text, but in bursts may be able to reach much higher speeds.[6] From the 1920s through the 1970s, typing speed (along with shorthand speed) was an important secretarial qualification, and typing contests were popular and often publicized by typewriter companies as promotional tools.");
//    for (size_t i = 0; i < strlen(dup); i++)
//        write(1, &dup[i], 1);
//    write(1, "\n", 1);
//    char **split = ft_split(dup, ' ');
//    int i = 0;
//    while (split[i])
//    {
//        ft_putstr_fd(split[i], 1);
//        write(1, "\n", 1);
//        i++;
//    }
    return (0);
}
