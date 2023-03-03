#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef char ALIGN[16];

// header wrapped on a union 'cause union ensure members' alignment
union header {
    struct {
        size_t size;
        unsigned is_free;
        union header* next;
    } md; // metadata
    ALIGN _a;
};

typedef union header header_t;

// memory allocation is a linked list to make possible to traverse
header_t *head, *tail;
pthread_mutex_t global_malloc_lock;

header_t* 
get_free_block(size_t size) {
    header_t* curr = head;

    while (curr) {
        if (curr->md.is_free && curr->md.size >= size) {
            return curr;
        }

        curr = curr->md.next;
    }

    return NULL;
}

void* 
malloc(size_t size) {
    if (!size) {
        return NULL;
    }

    pthread_mutex_lock(&global_malloc_lock);
    header_t* header = get_free_block(size);

    if (header) {
        header->md.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);

        return (void*) (header + 1); // +1 to get the start of the new block
    }

    size_t total_size = sizeof(header_t) + size;
    void* block = sbrk(total_size);

    // error to acquire memory
    if (block == (void*) -1) {
        pthread_mutex_unlock(&global_malloc_lock);

        return NULL;
    }

    header = block;
    header->md.is_free = 0;
    header->md.size = size;
    header->md.next = NULL;

    if (!head) {
        head = header;
    }

    if (tail) {
        tail->md.next = header;
    }

    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);

    return (void*) (header + 1); // +1 to get the start of the new block
}

void
free(void* block) {
    void* brk; // program break
    header_t *header, *tmp;

    if (!block) {
        return;
    }

    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t*) block - 1;
    brk = sbrk(0); // sbrk(0) give the pointer location (end of the heap)

    if ((char*) block + header->md.size == brk) {
        if (head == tail) {
            head = tail = NULL;
        } else {
            tmp = head;

            while (tmp) {
                if(tmp->md.next == tail) {
                    tmp->md.next = NULL;
                    tail = tmp;
                }

                tmp = tmp->md.next;
            }
        }

        sbrk(0 - sizeof(header_t) - header->md.size);
        pthread_mutex_unlock(&global_malloc_lock);

        return;
    }

    header->md.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);

    // debug output on change allocator with LD_PRELOAD environment
    fprintf(stdout, "== allocator ==\n");
}

void* 
calloc(size_t num, size_t nsize) {
	if (!num || !nsize) {
		return NULL;
    }

	size_t size = num * nsize;

    // check multiplication overflow
	if (nsize != size / num) {
		return NULL;
    }

	void* block = malloc(size);

	if (!block) {
		return NULL;
    }

	memset(block, 0, size); // fill-up the block with zeros

	return block;
}

void*
realloc(void* block, size_t size) {
	if (!block || !size) {
		return malloc(size);
    }

	header_t* header = (header_t*) block - 1;

	if (header->md.size >= size) {
		return block;
    }

	void* new = malloc(size);

	if (new) {
		memcpy(new, block, header->md.size);
		free(block);
	}

	return new;
}
