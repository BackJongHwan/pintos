#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <list.h>
#include <debug.h>
#include <stdbool.h>
#include <stdio.h>

/* Frame Table */
static struct list frame_table;
static struct lock frame_table_lock;
/* Initialize the frame table */
void frame_table_init(void) {
    list_init(&frame_table);
    lock_init(&frame_table_lock);
}

/* Allocate a frame for the given page */
void *frame_alloc(uint8_t *page, bool zero) {
    lock_acquire(&frame_table_lock);
    void *frame;
    
    /* Allocate a physical frame */
    ASSERT(page != NULL);

    if(zero) {
        frame = palloc_get_page(PAL_USER | PAL_ZERO);
    } else {
        frame = palloc_get_page(PAL_USER);
    }
    if(frame == NULL){  
        // frame = frame_evict();
        // if (frame == NULL) {
        lock_release(&frame_table_lock);
        return NULL;
        // }
    }
    /* Create and initialize a new frame table entry */
    struct frame_table_entry *fte = malloc(sizeof(struct frame_table_entry));
    // frame table entry가 없으면 frame을 free하고 return
    if (fte == NULL) {
        palloc_free_page(frame);
        lock_release(&frame_table_lock);
        return NULL;
    }
    fte->frame = frame;
    fte->page = page;
    fte->owner = thread_current();
    fte->accessed = true;
    list_push_back(&frame_table, &fte->elem);

    lock_release(&frame_table_lock);
    return frame;
}

/* Free the given frame */
void frame_free(uint8_t *frame) {
    lock_acquire(&frame_table_lock);

    struct list_elem *e;
    for (e = list_begin(&frame_table); e != list_end(&frame_table); e = list_next(e)) {
        struct frame_table_entry *fte = list_entry(e, struct frame_table_entry, elem);
        if (fte->frame == frame) {
            list_remove(&fte->elem);
            palloc_free_page(frame);
            free(fte);
            break;
        }
    }
    lock_release(&frame_table_lock);
}