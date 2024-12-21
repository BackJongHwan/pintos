#include<hash.h>
#include "vm/page.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <debug.h>
#include <stdbool.h>
#include <stdio.h>

static struct hash spt_table;
static struct lock spt_lock;

void spt_init(struct hash *spt_table){
    hash_init(spt, spt_hash_func, spt_less_func, NULL);
}
