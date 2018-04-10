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
#include "lib/syscall-nr.h"
#include "userprog/flist.h"
#include "filesys/directory.h"
#include "filesys/inode.h"

#include "devices/timer.h"

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
static int32_t sys_read_(int32_t FD, char * buf , const unsigned size){
  int32_t i = 0;
  struct thread* tr = thread_current();
  struct map* m = &tr->file_map;
  if(map_find(m, (int)FD) != -1){ // Filen Öppnad
    struct file* f = map_find(m, FD);
    if(f != NULL) return (int32_t)file_read(f, buf, size);
    else return -1;

  }else{ // Filen inte Öppnad
    return -1;
  }
}

static int32_t sys_write_(int32_t FD, char * buf , const unsigned size){
  int32_t i = 0;
  struct thread* tr = thread_current();
  struct map* m = &tr->file_map;
  if(map_find(m, (int)FD) != -1){ // Filen öppnad FD != STDIN_FILENO
    struct file* f = map_find(m, FD);
    if (f != NULL) return (int32_t)file_write(f, buf, size);
    else return -1;

  }else{ // Filen inte öppnad
    return -1;
  }

  return (int32_t)i;
}


static int32_t sys_keyboard_read_(char * FD, char * buf , const unsigned size){
  unsigned i = 0;
  char c;
  while(i<size){
    c = (char)input_getc();
    if(c == '\r'){
      c = '\n';
    }
    *buf = c;
    //  int32_t intpath = (int32_t)esp[1];
    putbuf(buf, (size_t)1);
    ++buf;
    ++i;
    //if(c == '\n')
    //break;
  }
  *buf = '\0';
  return (int32_t)i;
}
static int32_t sys_open_file_(char* filepath){
  struct thread* tr = thread_current();
  struct map* m = &tr->file_map;
  struct file * f = filesys_open(filepath);
  if(f != NULL){
    int32_t retval = map_insert(m,f);
    return retval; // -1 if map_insert fails
  }else{
    return (int32_t)-1;
  }
}
static int32_t sys_console_write_(char * FD, char * buf , const unsigned size){
  putbuf(buf, (size_t)size);
  return (int32_t)size;
}


void sys_seek_(int FD, unsigned pos){
  struct thread* tr = thread_current();
  struct map* m = &tr->file_map;
  struct file* fil = map_find(m, FD);
  if(fil != -1){
    return file_seek(fil, (off_t)pos);
  }

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

// NEW 31 aug 2017
void sys_plist_ (void) {
  // Snygg utskrift
  process_print_list();
}


// TODO: Refactor syscall handler to be more readable!!
static void
syscall_handler (struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;

  switch ( *esp /* retrive syscall number */ )
  {
    case SYS_HALT :
    power_off();
    break;
    case SYS_EXIT:
    // TODO: process_cleanup: http://www.ida.liu.se/~TDIU16/2017/lab/pdf/17sysexec.pdf
    printf("thread_exit()... %i\n", esp[1]);
    thread_exit();
    printf("thread_exit() done...\n");
    break;

    case SYS_READ :
    //läs fil
    NULL;
    {
      int32_t FD = (int32_t)esp[1];
      if(FD != STDOUT_FILENO){
        if(FD == STDIN_FILENO){
          //printf("STDIN");
          int32_t buffer = (int32_t)esp[2];
          int32_t len = (int32_t)esp[3];
          int32_t nr_bytes = sys_keyboard_read_((char *)FD, (char *)buffer, (unsigned)len);
          f->eax = nr_bytes;
        }else{ // om fil
          int32_t buffer = (int32_t)esp[2];
          int32_t len = (int32_t)esp[3];
          int32_t nr_bytes = sys_read_(FD, (char *)buffer, (unsigned)len);
          f->eax = nr_bytes;

        }
      }else{
        //return -1
        f->eax = -1;
        printf("NEEEEJ READ\n");
      }
      break;
    }
    case SYS_WRITE :
    //skriv ti
    NULL;
    {
      int32_t FD = (int32_t)esp[1];
      if(FD != STDIN_FILENO){
        if(FD == STDOUT_FILENO){
          int32_t buffer = (int32_t)esp[2];
          int32_t len = (int32_t)esp[3];
          int32_t nr_bytes = sys_console_write_((char *)FD, (char *)buffer, (unsigned)len);
          f->eax = nr_bytes;
        }else{ //Hantera fil istället
          int32_t buffer = (int32_t)esp[2];
          int32_t len = (int32_t)esp[3];
          int32_t nr_bytes = sys_write_(FD, (char *)buffer, (unsigned)len);
          f->eax = nr_bytes;
        }
      }else{
        //return -1
        f->eax = -1;
        printf("NEEEEJ WRITE\n");

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
      f->eax = (int32_t)filesys_create ((char*)intpath, (off_t)size);
      break;
    }
    case SYS_SEEK :
    NULL;
    {
      int32_t FD = esp[1];
      int32_t pos = esp[2];
      sys_seek_((int)FD, (unsigned) pos);
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

      /* Men nu skall den nya processen läggas till i en
      lista över aktiva processer. Ett lämpligt process-id är nu processens index i listan, men
      det går att lösa på många sätt (valfritt). För att kunna hålla reda på relationen mellan
      processer behöver varje process hålla reda på sitt namn, sitt eget process-id, sin
      förälders process-id, samt eventuell extra data som en process behöver skicka tillbaka
      till sin förälderprocess eller tvärtom (jämför med vad du gjorde i uppgift 10). */

      char* cml = (char*)esp[1]; // ESP index?

      // Check if esp[1] is valid pointer.
      if (cml == NULL) {
        printf("SYS_EXEC: ESP1 fails");
        f->eax = -1;
        thread_exit();
        break;
      }
      uint32_t id = process_execute(cml);
      f->eax = id;
      break;
    }
    case SYS_SLEEP :
    NULL;
    {
      int32_t time = esp[1];
      timer_msleep((int64_t)time);
      break;
    }
    case SYS_PLIST :
    NULL;
    {
      sys_plist_();

      break;
    }
    default:
    {
      printf ("Executed an unknown system call!\n");

      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);

      thread_exit ();
    }
  }
}
