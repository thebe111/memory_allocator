#ifndef PAGE_H
#define PAGE_H

#include <stddef.h>

typedef struct page_s {
    size_t size;
} page_t;

void* alloc_page(size_t size);
void dalloc_page(page_t* page);

#endif // PAGE_H
