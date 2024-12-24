#include "frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <list.h>
#include <debug.h>
#include <stdbool.h>
#include <stdio.h>
#include  "swap.h"

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

    void *frame;
    lock_acquire(&frame_table_lock);
    /* Allocate a physical frame */
    ASSERT(page != NULL);

    frame = zero ? palloc_get_page(PAL_USER | PAL_ZERO) : palloc_get_page(PAL_USER);
    if(frame == NULL){  
        frame = frame_evict();
        if (frame == NULL) {
        lock_release(&frame_table_lock);
        return NULL;
        }
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
    fte->pinned = false;
    list_push_back(&frame_table, &fte->elem);
    lock_release(&frame_table_lock);
    return frame;
}

/* Free the given frame */
void frame_free(uint8_t *frame) {
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
}

void *frame_evict(void) {
    // printf("frame_evict\n");
    static struct list_elem *victim = NULL; // 원형 큐의 "현재 위치"
    // lock_acquire(&frame_table_lock);
    if (victim == NULL || victim == list_end(&frame_table)) {
        victim = list_begin(&frame_table); // 첫 번째 프레임에서 시작
    }
    while (true) {
        struct frame_table_entry *fte = list_entry(victim, struct frame_table_entry, elem);

        // Pinned 프레임은 건너뜀
        if (fte->pinned) {
            victim = list_next(victim);
            if (victim == list_end(&frame_table)) {
                victim = list_begin(&frame_table);
            }
            continue;
        }

        // 참조되지 않은 프레임을 찾음
        if (!pagedir_is_accessed(fte->owner->pagedir, fte->page)) {
            struct spt_entry *spte = spt_find(fte->page);  // SPT에서 해당 페이지 찾기
            if (spte == NULL) {
                PANIC("SPT entry not found for evicted page!");
            }
            // Swap Out 처리
            size_t swap_slot = swap_out(fte->page);
            spte->status = SWAP;         // SPT 상태를 SWAP으로 변경
            spte->swap_slot = swap_slot; // 스왑 슬롯 정보 저장
            // 페이지 테이블에서 제거
            pagedir_clear_page(fte->owner->pagedir, fte->page);
            // 프레임 테이블에서 제거
            victim = list_remove(victim); // 현재 프레임 제거 후 다음 프레임으로 이동
            // lock_release(&frame_table_lock);
            // printf("frame_evict end\n");
            return fte->frame;
        } else {
            // 참조 비트를 초기화
            pagedir_set_accessed(fte->owner->pagedir, fte->page, false);
        }

        // 다음 프레임으로 이동 (원형 큐 방식)
        victim = list_next(victim);
        if (victim == list_end(&frame_table)) {
            victim = list_begin(&frame_table); // 원형 큐처럼 처음으로 돌아감
        }
    }
    // lock_release(&frame_table_lock);
}
