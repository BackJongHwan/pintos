#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
enum page_status{
    LOAD,  // memory에 load되어 있는 상태
    SWAP,  // swap disk 에 있는 상태
    UNALLOCATED, // mapping된 page가 없는 상태
    FILE   // file에 mapping된 상태
};

/* Supplemental Page Table Entry */
struct spt_entry{

    void *upage;            // User virtual page address
    void *kpage;            // Kernel virtual page address
    
    enum page_status status; // Page status

    bool writable;          // Writable or not

    struct file *file;      // File pointer
    off_t offset;           // Offset

    size_t read_bytes;      // Read bytes
    size_t zero_bytes;      // Zero bytes
    
    size_t swap_index;      // Swap index

    struct hash_elem elem;  // Hash element
};

/* Functions to manage the supplemental page table */
void spt_init(struct hash *spt);
bool spt_insert(struct hash *spt, void *upage, void *kpage, bool writable, bool swapped, bool loaded, struct file *file, off_t ofs, size_t read_bytes, size_t zero_bytes, size_t swap_index);
bool spt_delete(struct hash *spt, void *upage);
struct spt *spt_find(struct hash *spt, void *upage);

bool spt_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux);

#endif