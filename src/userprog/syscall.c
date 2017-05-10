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

//NEW
#include "userprog/flist.h"
#include "filesys/directory.h"
#include "filesys/inode.h"
//#include "devices/timer.c"

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

static int32_t sys_open_file_ (const char * file) {
  struct thread* tr = thread_current();
  struct map* m = &tr->file_map;
  struct file * f = filesys_open(file);
  if(f != NULL) {
    int32_t retval = map_insert(m,f);
    return retval;
  }else{
    return (int32_t)-1;
  }
}

static int32_t sys_console_write_ (char * FD, char * buf, const unsigned size) {
  putbuf (buf, (size_t)size);
  return (int32_t)size;
}

unsigned sys_tell_(int FD){
  struct thread* tr = thread_current();
  struct map* m = &tr->file_map;
  struct file* fil = map_find(m, FD);
  int i = 0;
  if(fil != -1){
    return file_tell(fil);
  }else{
    return (unsigned)-1;
  }
}

unsigned sys_filesize_(int FD){
  struct thread* tr = thread_current();
  struct map* m = &tr->file_map;
  struct file* fil = map_find(m, FD);
  if(fil != -1){
    return file_length(fil);
  }else{
    return (unsigned)-1;
  }
}


void sys_seek_(int FD, unsigned pos){
  struct thread* tr = thread_current();
  struct map* m = &tr->file_map;
  struct file* fil = map_find(m, FD);
  if(fil != -1){
    return file_seek(fil, (off_t)pos);
  }

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
    case SYS_OPEN : //Open a file
    NULL;
    {
      int32_t intpath = (int32_t)esp[1];
      int32_t retval = (int32_t)sys_open_file_((char*)intpath);
      f->eax = retval;
      break;
    }
    case SYS_CLOSE :
    NULL;
    {
      int32_t fd = (int32_t)esp[1];
      struct thread *t = thread_current();
      struct map *m = &t->file_map;
      struct file * f = map_remove(m,fd);
      break;
    }
    case SYS_REMOVE :
    NULL;
    {
      int32_t intpath = (int32_t)esp[1];
      f->eax = (int32_t)filesys_remove((char*)intpath);
      break;
    }
    case SYS_CREATE :
    NULL;
    {
      int32_t intpath = (int32_t)esp[1];
      int32_t size = (int32_t)esp[2];
      f->eax = (int32_t)filesys_create ((char*) intpath, (off_t)size);
      break;
    }
    case SYS_TELL :
    NULL;
    {
      int32_t FD = esp[1];
      f->eax = sys_tell_((int)FD);
      break;
    }
    case SYS_FILESIZE :
    NULL;
    {
      int32_t FD = esp[1];
      f->eax = sys_filesize_((int)FD);

      break;
    }
    case SYS_EXEC :
    NULL;
    {
      int32_t cml = esp[1];
      int32_t id = process_execute((const char *)cml);
      break;
    }
    // case SYS_SLEEP :
    // NULL;
    // {
    //   int32_t millis = esp[1];
    //   timer_msleep((int64_t)millis);
    //   break;
    // }
    // case SYS_PLIST :
    // NULL;
    // {
    //   // break for now
    //   break;
    // }


    default:
    {
      printf ("Executed an unknown system calssssl!\n");

      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);

      thread_exit ();
    }
  }
}
