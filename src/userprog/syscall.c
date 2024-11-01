#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
//my
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"  // shutdown_power_off 함수 선언을 위한 헤더 파일
#include "devices/input.h" //for input_getc()
#include "userprog/pagedir.h"
// struct lock print_lock;
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
// #include"filesys/file.c"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

//TODO: syscall_handler implement
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //implement exit, halt, exec, wait
  //read, write standard input/output!!
  //exit -> shutdown_power_off()
  //exit -> terminate current user program, returning status to the kernel
  //exec() -> 1. create child process
  // 2. refer to process_execute() in userprog/proceess.c
  //wait() -> 

  //valid
  if(!is_valid_user_pointer(f->esp)){
    exit(-1);
  } 
  int syscall_num = *(int*)f->esp;
  switch (syscall_num)
  {
  case SYS_HALT:
    halt();
    break;
  case SYS_EXIT:
    if(!is_valid_user_pointer(f->esp + 4)){
      exit(-1);
    }
    int status = *((int *)(f->esp + 4));
      exit(status);
    break;
  case SYS_EXEC:
    if(!is_valid_user_pointer(f->esp + 4)){
      exit(-1);
    }
    f->eax = exec(*(const char**)(f->esp + 4));
    break;
  case SYS_WAIT:
    if (!is_valid_user_pointer(f->esp + 4)) {
        exit(-1);  
    }
    tid_t tid = *(tid_t *)(f->esp + 4);
    f->eax = wait(tid);
    break;
  case SYS_WRITE:
    if(!is_valid_user_pointer(f->esp + 4) || !is_valid_user_pointer(f->esp + 8) || !is_valid_user_pointer(f->esp + 12) ){
      exit(-1);
    }
    f->eax = write(*(int*)(f->esp + 4), *(void**)(f->esp + 8), *(unsigned *)(f->esp + 12));
    break;
  case SYS_READ:
    if(!is_valid_user_pointer(f->esp + 4) || !is_valid_user_pointer(f->esp + 8) || !is_valid_user_pointer(f->esp + 12) ){
      exit(-1);
    }
    f->eax = read(*(int*)(f->esp + 4), *(void**)(f->esp + 8), *(unsigned *)(f->esp + 12));
    break;
  case SYS_FIBO:
    if(!is_valid_user_pointer(f->esp + 4)){
      exit(-1);
    }
    f->eax = Fibonacci(*(int*)(f->esp + 4));
    break;
  case SYS_MAX_FOUR_INT:
    if(!is_valid_user_pointer(f->esp + 4) ||!is_valid_user_pointer(f->esp + 8) ||!is_valid_user_pointer(f->esp + 12) | !is_valid_user_pointer(f->esp + 16) ){
      exit(-1);
    }
    f->eax = Max_of_four_int(*(int*)(f->esp + 4),*(int*)(f->esp + 8), *(int*)(f->esp + 12), *(int*)(f->esp + 16) );
    break;
  case SYS_CREATE:
    if(!is_valid_user_pointer(f->esp+4) || !is_valid_user_pointer(f->esp + 8)){
      exit(-1);
    }
    f->eax = create(*(const char**)(f->esp + 4), *(unsigned *)(f->esp + 8));
    break;
  case SYS_REMOVE:
    if(!is_valid_user_pointer(f->esp+4)){
      exit(-1);
    }
    f->eax = remove(*(const char**)(f->esp + 4));
    break;
  case SYS_OPEN:
    if(!is_valid_user_pointer(f->esp+4)){
      exit(-1);
    }
    f->eax = open(*(const char**)(f->esp + 4));
    break;
  case SYS_CLOSE:
    if(!is_valid_user_pointer(f->esp + 4)){
      exit(-1);
    }
    close(*(int*)(f->esp + 4));
    break;
  case SYS_FILESIZE:
    if(!is_valid_user_pointer(f->esp + 4)){
      exit(-1);
    }
    f->eax = filesize(*(int*)(f->esp + 4));
    break;
  case SYS_SEEK:
    if(!is_valid_user_pointer(f->esp+4) || !is_valid_user_pointer(f->esp + 8)){
      exit(-1);
    }
    seek(*(int*)(f->esp + 4),*(unsigned *)(f->esp + 8));
    break;
  case SYS_TELL:
    if(!is_valid_user_pointer(f->esp + 4)){
      exit(-1);
    }
    f->eax = tell(*(int*)(f->esp + 4));
    break;
  default:
    printf("unknown system call!!\n");
    exit(-1);
  }
}

int is_valid_user_pointer(const void * uaddr){
    if (uaddr == NULL) {
        return false;
    }
    //is user_vadder
    if (!is_user_vaddr(uaddr)) {
        return false;
    }
    void *ptr = pagedir_get_page(thread_current()->pagedir, uaddr);
    if(ptr == NULL){
      return false;
    }

    return true;
}
/*shut down system*/
void halt(void){
  shutdown_power_off();
}

/*Terminate current thread*/
void exit(int status)
{
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status = status;
  thread_exit();
}

tid_t exec(const char* file){
  // printf("Exec called with file_name: %s\n", file);  // file 값 확인
  if (!is_valid_user_pointer(file)) {
      exit(-1);  
  }
  return process_execute(file);
}

int wait(tid_t pid){
  return process_wait(pid);
}


int Fibonacci(int n){
  if(n < 0){
    printf("invalid input argument!\n");
    exit(-1);
  }
  if(n == 0){
    return 0;
  }
  if(n == 1){
    return 1;
  }
  int fn_first = 0;
  int fn_second = 1;
  int result;
  for(int i = 2; i <= n; i++){
    result = fn_first + fn_second;
    fn_first= fn_second;
    fn_second = result; 
  }
  return result;
}



int Max_of_four_int(int a, int b, int c, int d){
  int max = a;
  if(b > max) max = b;
  if(c > max) max = c;
  if(d > max) max = d;
  return max;
}

int open (const char *file){

  if(!is_valid_user_pointer(file)){
    exit(-1);
  }

  //no availble file descriptor
  if(thread_current()->fd_num >= FD_MAX){
    return -1;
  }
  struct file *f = filesys_open(file);
  //if false to open
  if(f == NULL){
    return -1;
  }
  // if(thread_current()->exec_file == f){
  //   file_deny_write(f);
  //   printf("deny write!!");
  // }else{
  //   printf("not the same!!");

  // }

  for(int i = 2; i < FD_MAX; i++){
    if(thread_current()->fd_table[i] == NULL){
      thread_current()->fd_table[i] = f;
      thread_current()->fd_num++;
      return i;
    }
  }
  return -1;
}

bool create (const char *file, unsigned initial_size)
{
  if(!is_valid_user_pointer(file)) {
    exit(-1);  
  }
  bool success = filesys_create(file, initial_size);
  return success;
}

bool remove (const char *file)
{
  if (!is_valid_user_pointer(file)) {
      exit(-1);  
  }
  bool success = filesys_remove(file);
  return success;
}
int filesize (int fd)
{
  if(!thread_current()->fd_table[fd]){
    exit(-1);
  }
  return file_length(thread_current()->fd_table[fd]);
}
void seek (int fd, unsigned position)
{
  if(!thread_current()->fd_table[fd]){
    exit(-1);
  }
  file_seek(thread_current()->fd_table[fd], position);
}
unsigned tell (int fd)
{
  if(!thread_current()->fd_table[fd]){
    exit(-1);
  }
  return(file_tell(thread_current()->fd_table[fd]));
}

void close (int fd)
{
  if(fd < 2 || fd >= FD_MAX){
    return;
  }
  if(!thread_current()->fd_table[fd]){
    return;
  }
  thread_current()->fd_num--;
  thread_current()->fd_table[fd] = NULL;
  file_close(thread_current()->fd_table[fd]);
}

int write(int fd, void *buffer, unsigned size){
  if(fd < 1 || fd >= FD_MAX || !is_valid_user_pointer(buffer)){
    exit(-1);
  }
  //lock 필요함
  //if stdout
  if(fd == 1){
    putbuf(buffer, size);
    return size;
  }

  struct file *f = thread_current()->fd_table[fd];
  if(f == NULL || f->deny_write){
    return 0;
  }

  return (file_write(f, buffer, size));
  
}

int read(int fd, void *buffer, unsigned size){

  if(fd < 0 || fd >= FD_MAX || !is_valid_user_pointer(buffer)){
    exit(-1);
  }

  if(fd == 0){
    for(unsigned i = 0; i < size; i++){
      *((char *)buffer + i) = input_getc();
    }
    return size;
  }

  struct file *f = thread_current()->fd_table[fd];
  if(f == NULL){
    return 0;
  }

  return(file_read(thread_current()->fd_table[fd], buffer, size));
}