#include<hash.h>
#include "vm/page.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <debug.h>
#include <stdbool.h>
#include <stdio.h>
#include "threads/thread.h"


bool spt_insert(struct spt_entry *spte){
    struct thread *t = thread_current();
    struct hash *spt = &t->spt;
    struct hash_elem *e = hash_insert(spt, &spte->elem);
    return e == NULL;
}

struct spt_entry *spt_find(void *upage){
    struct thread *t = thread_current();
    struct hash *spt = &t->spt;
    struct spt_entry temp_spte;
    temp_spte.upage = upage;
    struct hash_elem *e = hash_find(spt, &temp_spte.elem);
    return e != NULL ? hash_entry(e, struct spt_entry, elem) : NULL;
}

void spt_destroy(struct hash *spt){

    ASSERT(spt != NULL);
    hash_destroy(spt, spt_free_entry);
}


void spt_free_entry(struct hash_elem *e, void *aux UNUSED){
    struct spt_entry *spte = hash_entry(e, struct spt_entry, elem);
    
    //entry가 swap에 있는 경우 swap slot을 free해준다.
    if(spte->status == SWAP){
        // swap_free(spte->swap_slot);
    }
    //entry가 file에 있는 경우 file을 close해준다.
    else if(spte->status == FILE){
        file_close(spte->file);
    }
    //entry가 load된 경우 page를 free해준다.
    else if(spte->status == LOAD){
        palloc_free_page(pagedir_get_page(thread_current()->pagedir, spte->upage));
    }

    free(spte);
}



