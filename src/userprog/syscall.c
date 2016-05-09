#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);

  void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:

   int sys_read_arg_count = argc[ SYS_READ ];

   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1, 
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

static int32_t sys_read_ (char * FD, char * buf, const unsigned size) {
  int i = 0;
  while (i<size) {
    *buf = *FD;
    ++FD;
    ++buf;
    ++i;
  }
  return (int32_t)i;
}

static int32_t sys_write_ (char * FD, char * buf, const unsigned size) {
  int i = 0;
  while (i<size) {
    *buf = *FD;
    ++FD;
    ++buf;
    ++i;
  }
  return (int32_t)i;
}

static int32_t sys_keyboard_read_ (char * FD, char * buf, const unsigned size) {
  unsigned i = 0;
  char c;
  while (i<size) {
    c = (char)input_getc();
    if (c == '\r') {
      c = '\n';
    }
    *buf = c;
    putbuf (buf, (size_t)1);
    ++buf;
    ++i;
  }
  *buf = '\0';
  return (int32_t)i;
}

static int32_t sys_console_write_ (char * FD, char * buf, const unsigned size) {
  unsigned i = 0;
  putbuf (buf, (size_t)size);
  return (int32_t)size;
}

  static void
syscall_handler (struct intr_frame *f) 
{
  int32_t *esp = (int32_t*)f->esp;

  switch ( *esp /*i retrive syscall number */ )
  {
  case SYS_HALT : 
    printf ("Kör power_off()\n");
    power_off(); 
    break;

  case SYS_EXIT :
    printf ("thread_exit(), status nr: %i\n", esp[1]);
    thread_exit();
    printf("thread_exit() done...\n");    
    break;

  case SYS_READ : 
    // Läs från keyboard och lägg till i fönster?
    NULL; 
    {
      int32_t FD = (int32_t)esp[1];
      if (FD != STDOUT_FILENO) {
        if (FD == STDIN_FILENO) {
          //printf("STDIN");
          int32_t buffer = (int32_t)esp[2];
          int32_t len = (int32_t)esp[3];
          int32_t nr_bytes = sys_keyboard_read_ ((char *)FD, (char *)buffer, (unsigned)len);
          f->eax = nr_bytes;
        }else{
          // Bara terminalen hanteras 
        }
      }else{
        //return -1
        f->eax = -1;
        printf("NEJ READ \n");
      }
      break;
    }

  case SYS_WRITE : 
  // skriv
NULL; 
    {
      int32_t FD = (int32_t)esp[1];
      if (FD != STDIN_FILENO) {
        if (FD == STDOUT_FILENO) {
          //printf("STDOUT");
          int32_t buffer = (int32_t)esp[2];
          int32_t len = (int32_t)esp[3];
          int32_t nr_bytes = sys_console_write_ ((char *)FD, (char *)buffer, (unsigned)len);
          f->eax = nr_bytes;
        }else{
          // Bara terminalen hanteras 
        }
      }else{
        //return -1
        f->eax = -1;
        printf("NEJ WRITE \n");
      }
      break;
    }



  default:
    {
      printf ("Executed an unknown system calssssl!\n");

      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);

      thread_exit ();
    }
  }
}

