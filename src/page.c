#include "include/page.h"
#include "include/main.h"

void*
alloc_page(size_t size) {
    return (void*) 1;
}

void 
dalloc_page(page_t* page) {
    if (page == NULL) {
        return;
    }

    pthread_mutex_lock(&GLOBAL_MUTEX);

    // action 

    pthread_mutex_unlock(&GLOBAL_MUTEX);
}
