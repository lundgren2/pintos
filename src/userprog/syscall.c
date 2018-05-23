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

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
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
    2, 1, 1, 1, 2, 1, 1,
    /* extended */
    0};
static int32_t sys_read_(int32_t FD, char *buf, const unsigned size)
{
  struct thread *tr = thread_current();
  struct map *m = &tr->file_map;
  struct file *file = map_find(m, (int)FD);
  if ((int)file != -1)
  { // Filen Öppnad
    struct file *f = map_find(m, FD);
    if (f != NULL)
    {
      return (int32_t)file_read(f, buf, size);
    }
  }
  // Filen inte Öppnad
  return -1;
}

static int32_t sys_write_(int32_t FD, char *buf, const unsigned size)
{
  struct thread *tr = thread_current();
  struct map *m = &tr->file_map;
  struct file *file = map_find(m, (int)FD);
  if ((int)file != -1)
  { // Filen öppnad FD != STDIN_FILENO
    struct file *f = map_find(m, FD);
    if (f != NULL)
      return (int32_t)file_write(f, buf, size);
  }
  // Filen inte öppnad
  return -1;
}

static int32_t sys_keyboard_read_(char *FD, char *buf, const unsigned size)
{
  unsigned i = 0;
  char c;
  while (i < size)
  {
    c = (char)input_getc();
    if (c == '\r')
    {
      c = '\n';
    }
    *buf = c;
    putbuf(buf, (size_t)1);
    ++buf;
    ++i;
  }
  *buf = '\0';
  return (int32_t)i;
}
static int32_t sys_open_file_(char *filepath)
{
  struct thread *tr = thread_current();
  struct map *m = &tr->file_map;
  struct file *f = filesys_open(filepath);
  if (f != NULL)
  {
    int32_t retval = map_insert(m, f);
    // -1 if map_insert fails
    return retval;
  }
  else
  {
    return (int32_t)-1;
  }
}
static int32_t sys_console_write_(char *FD, char *buf, const unsigned size)
{
  putbuf(buf, (size_t)size);
  return (int32_t)size;
}

void sys_seek_(int FD, unsigned pos)
{
  struct thread *tr = thread_current();
  struct map *m = &tr->file_map;
  struct file *file = map_find(m, FD);
  if ((int)file != -1)
  {
    return file_seek(file, (off_t)pos);
  }
}

unsigned sys_tell_(int FD)
{
  struct thread *tr = thread_current();
  struct map *m = &tr->file_map;
  struct file *file = map_find(m, FD);
  if ((int)file != -1)
  {
    return file_tell(file);
  }
  else
  {
    return (unsigned)-1;
  }
}

unsigned sys_filesize_(int FD)
{
  struct thread *tr = thread_current();
  struct map *m = &tr->file_map;
  struct file *file = map_find(m, FD);
  if ((int)file != -1)
  {
    return file_length(file);
  }
  else
  {
    return (unsigned)-1;
  }
}

void sys_plist_(void)
{
  process_print_list();
}

void sys_exit_(void)
{
  process_exit(-1);
  thread_exit();
}

static void syscall_handler(struct intr_frame *f)
{
  int32_t *esp = (int32_t *)f->esp;
  int32_t FD = (int32_t)esp[1];
  int32_t buffer = (int32_t)esp[2];
  int32_t len = (int32_t)esp[3];
  char *cml = (char *)esp[1];
  struct thread *t = thread_current();

  if(esp == NULL || !verify_fix_length(esp, sizeof(esp))) {
    process_exit(-1);
    thread_exit();
  }

  switch (*esp /* retrive syscall number */)
  {

  case SYS_HALT:
    power_off();
    break;
  case SYS_EXIT:
    // TODO: CHECK kernel addr
    process_exit((int)FD); // Update exit_status
    thread_exit();
    break;

  case SYS_READ:
    if (FD != STDOUT_FILENO)
    {
      if(cml == NULL || !verify_fix_length(buffer, len)) {
        sys_exit_();
      }

      if (FD == STDIN_FILENO)
      {
        int32_t nr_bytes = sys_keyboard_read_((char *)FD, (char *)buffer, (unsigned)len);
        f->eax = nr_bytes;
      }
      else
      { // om file
        int32_t nr_bytes = sys_read_(FD, (char *)buffer, (unsigned)len);
        f->eax = nr_bytes;
      }
    }
    else
    {
      f->eax = -1;
    }
    break;
  case SYS_WRITE:
    if (FD != STDIN_FILENO)
    {
      if(!verify_fix_length(buffer, sizeof(buffer))) {
        sys_exit_();
        f->eax = -1;
        break;
      }

      if (FD == STDOUT_FILENO)
      {
        int32_t nr_bytes = sys_console_write_((char *)FD, (char *)buffer, (unsigned)len);
        f->eax = nr_bytes;
      }
      else
      { // Hantera file istället
        int32_t nr_bytes = sys_write_(FD, (char *)buffer, (unsigned)len);
        f->eax = nr_bytes;
      }
    }
    else
    {
      f->eax = -1;
    }
    break;
  case SYS_OPEN: // Open a file
   if( cml == NULL || !verify_variable_length(cml)) {
      sys_exit_();
      break;
    } 
    f->eax = (int32_t)sys_open_file_(cml);
    break;
  case SYS_CLOSE:
    map_remove(&t->file_map, FD);
    break;
  case SYS_REMOVE:
    if(cml == NULL || !verify_variable_length(cml)) {
      f->eax = 0;
      break;
    }
    f->eax = (int32_t)filesys_remove(cml);
    break;
  case SYS_CREATE:
    if (cml == NULL || !verify_variable_length(cml)) 
    {
      sys_exit_();
    }
    f->eax = (int32_t)filesys_create(cml, (off_t)buffer);
    break;
  case SYS_SEEK:
    sys_seek_((int)FD, (unsigned)buffer);
    break;
  case SYS_TELL:
    f->eax = sys_tell_((int)FD);
    break;
  case SYS_FILESIZE:
    f->eax = sys_filesize_((int)FD);
    break;
  case SYS_EXEC:
    /* Men nu skall den nya processen läggas till i en
      lista över aktiva processer. Ett lämpligt process-id är nu processens index i listan, men
      det går att lösa på många sätt (valfritt). För att kunna hålla reda på relationen mellan
      processer behöver varje process hålla reda på sitt namn, sitt eget process-id, sin
      förälders process-id, samt eventuell extra data som en process behöver skicka tillbaka
      till sin förälderprocess eller tvärtom (jämför med vad du gjorde i uppgift 10). */

    // Check if esp[1] is valid pointer.
    if (cml == NULL || !verify_variable_length(cml))
    {
      f->eax = -1;
      thread_exit();
      break;
    }
    uint32_t id = process_execute(cml);
    f->eax = id;
    break;
  case SYS_SLEEP:
    timer_msleep((int64_t)FD);
    break;
  case SYS_PLIST:
    sys_plist_();
    break;
  case SYS_WAIT:
    if(is_kernel_vaddr(cml)) {
      break;
    }
    f->eax = process_wait((int)FD);
    break;
  default:
    printf("# Executed an unknown system call!\n");
    printf("# Stack top + 0: %d\n", esp[0]);
    printf("# Stack top + 1: %d\n", esp[1]);

    thread_exit();
  }
}

/* Verify all addresses from and including 'start' up to but excluding
 * (start+length). */
bool verify_fix_length(void *start, int length)
{
  char *addr = (char *)pg_round_down(start);
  char *end_addr = (char *)(start + length);
  while (addr < end_addr)
  {
    bool check = pagedir_get_page(thread_current()->pagedir, (void *)addr) == NULL;
    if (check)
    {
      return false;
    }
    else
    {
      addr = addr + PGSIZE;
    }
  }
  return true; // If no problems return true
}

/* Verify all addresses from and including 'start' up to and including
 * the address first containg a null-character ('\0'). (The way
 * C-strings are stored.)
 */
bool verify_variable_length(char *start)
{
  bool check = pagedir_get_page(thread_current()->pagedir, (void *)start) == NULL;
  if (check)
  {
    return false;
  }
  else
  {
    char *addr = start;
    unsigned pagenum = pg_no(start);
    unsigned prevpage;
    while (true)
    {
      prevpage = pg_no(addr);
      if (pagenum != prevpage)
      {
        bool addr_check = pagedir_get_page(thread_current()->pagedir, (void *)addr) == NULL;
        if (addr_check)
        {
          return false;
        }
        prevpage = pg_no(addr);
      }
      if (*addr == '\0') // is_end_of_string
      {
        return true;
      }
      addr++;
    }
  }
}
