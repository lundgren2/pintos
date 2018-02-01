#include <debug.h>
#include <stdio.h>
#include <string.h>

#include "userprog/gdt.h"      /* SEL_* constants */
#include "userprog/process.h"
#include "userprog/load.h"
#include "userprog/pagedir.h"  /* pagedir_activate etc. */
#include "userprog/tss.h"      /* tss_update */
#include "filesys/file.h"
#include "threads/flags.h"     /* FLAG_* constants */
#include "threads/thread.h"
#include "threads/vaddr.h"     /* PHYS_BASE */
#include "threads/interrupt.h" /* if_ */
#include "threads/init.h"      /* power_off() */

/* Headers not yet used that you may need for various reasons. */
#include "threads/synch.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"

#include "userprog/flist.h"
#include "userprog/plist.h"

#define HACK
static struct System_process_list SPL;

/*
 pintos -p ../../examples/sumargv -a sumargv -v -k --fs-disk=2 -- -f -q run 'sumargv 1 2 3 4'
*/


/* FROM setup-argv.c */


bool exists_in(char c, const char* d)
{
  int i = 0;
  while (d[i] != '\0' && d[i] != c)
  ++i;
  return (d[i] == c);
}

/* Return the number of words in 'buf'. A word is defined as a
* sequence of characters not containing any of the characters in
* 'delimeters'.
* NOTE: arguments must be '\0'-terminated c-strings
*/
int count_args(const char* buf, const char* delimeters)
{
  int i = 0;
  bool prev_was_delim;
  bool cur_is_delim = true;
  int argc = 0;

  while (buf[i] != '\0')
  {
    prev_was_delim = cur_is_delim;
    cur_is_delim = exists_in(buf[i], delimeters);
    argc += (prev_was_delim && !cur_is_delim);
    ++i;
  }
  return argc;
}

/* Replace calls to STACK_DEBUG with calls to printf. All such calls
* easily removed later by replacing with nothing. */
#define STACK_DEBUG(...) printf(__VA_ARGS__)

void* setup_main_stack(const char* command_line, void* stack_top)
{
  /* Variable "esp" stores an address, and at the memory loaction
  * pointed out by that address a "struct main_args" is found.
  * That is: "esp" is a pointer to "struct main_args" */
  struct main_args* esp;
  int argc;
  int total_size;
  int line_size;
  /* "cmd_line_on_stack" and "ptr_save" are variables that each store
  * one address, and at that address (the first) char (of a possible
  * sequence) can be found. */
  char* cmd_line_on_stack;
  char* ptr_save;
  int i = 0;

  /* calculate the bytes needed to store the command_line */
  line_size = strlen(command_line)+1;
  STACK_DEBUG("# line_size = %d\n", line_size);

  /* round up to make it even divisible by 4 */
  while (line_size % 4 != 0) {
    line_size += 1;
  }

  STACK_DEBUG("# line_size (aligned) = %d\n", line_size);

  /* calculate how many words the command_line contain */
  {
    const char * tmp = " ";
    argc = count_args(command_line, tmp);
    STACK_DEBUG("# argc = %d\n", argc);
  }
  /* calculate the size needed on our simulated stack */
  total_size = line_size + (argc+4)*4;
  STACK_DEBUG("# total_size = %d\n", total_size);


  /* calculate where the final stack top will be located */
  esp = stack_top - total_size;

  /* setup return address and argument count */
  esp->ret = 0;
  esp->argc = argc;
  /* calculate where in the memory the argv array starts */
  esp->argv = &esp->argc+2;

  /* calculate where in the memory the words is stored */
  cmd_line_on_stack = &esp->argv[argc+1];

  /* copy the command_line to where it should be in the stack */
  for (size_t i = 0; i < line_size; i++) {
    cmd_line_on_stack[i] = command_line[i];
  }
  /* build argv array and insert null-characters after each word */
  esp->argv[0] = &cmd_line_on_stack[0];
  int argv_count = 1;
  for (size_t i = 1; i < line_size; i++) {
    if (cmd_line_on_stack[i] == ' ') {
      cmd_line_on_stack[i] = '\0'; // Insert null-character
      esp->argv[argv_count++] = &cmd_line_on_stack[i+1];

      if(argv_count >= argc)
        break;
    }
  }

  return esp; /* the new stack top */
}

/* END setup-argv.c */


/* This function is called at boot time (threads/init.c) to initialize
 * the process subsystem. */
void process_init(void)
{
  process_list_init(&SPL);
}

/* This function is currently never called. As thread_exit does not
 * have an exit status parameter, this could be used to handle that
 * instead. Note however that all cleanup after a process must be done
 * in process_cleanup, and that process_cleanup are already called
 * from thread_exit - do not call cleanup twice! */
void process_exit(int status UNUSED)
{
}

/* Print a list of all running processes. The list shall include all
 * relevant debug information in a clean, readable format. */
void process_print_list()
{
  process_list_print(&SPL);
}


struct parameters_to_start_process
{
  char* command_line;
  struct semaphore sema;
  bool init_ok;
};

static void
start_process(struct parameters_to_start_process* parameters) NO_RETURN;

/* Starts a new proccess by creating a new thread to run it. The
   process is loaded from the file specified in the COMMAND_LINE and
   started with the arguments on the COMMAND_LINE. The new thread may
   be scheduled (and may even exit) before process_execute() returns.
   Returns the new process's thread id, or TID_ERROR if the thread
   cannot be created. */
int
process_execute (const char *command_line)
{
  char debug_name[64];
  int command_line_size = strlen(command_line) + 1;
  tid_t thread_id = -1;
  int  process_id = -1;

  /* LOCAL variable will cease existence when function return! */
  struct parameters_to_start_process arguments;

  debug("%s#%d: process_execute(\"%s\") ENTERED\n",
        thread_current()->name,
        thread_current()->tid,
        command_line);

  /* COPY command line out of parent process memory */
  arguments.command_line = malloc(command_line_size);
  strlcpy(arguments.command_line, command_line, command_line_size);


  strlcpy_first_word (debug_name, command_line, 64);

  /* SCHEDULES function `start_process' to run (LATER) */
  thread_id = thread_create (debug_name, PRI_DEFAULT,
                             (thread_func*)start_process, &arguments);

  process_id = thread_id;

  /* AVOID bad stuff by turning off. YOU will fix this! */
  power_off();


  /* WHICH thread may still be using this right now? */
  free(arguments.command_line);

  debug("%s#%d: process_execute(\"%s\") RETURNS %d\n",
        thread_current()->name,
        thread_current()->tid,
        command_line, process_id);

  /* MUST be -1 if `load' in `start_process' return false */
  return process_id;
}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (struct parameters_to_start_process* parameters)
{
  /* The last argument passed to thread_create is received here... */
  struct intr_frame if_;
  bool success;

  char file_name[64];
  debug("======== parameters->command_line: %s\n END OF PARAMETERS!\n", parameters->command_line);
  strlcpy_first_word (file_name, parameters->command_line, 64);

  // debug("======== File_name: %s\n", file_name);


  debug("%s#%d: start_process(\"%s\") ENTERED\n",
        thread_current()->name,
        thread_current()->tid,
        parameters->command_line);

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  // c
  success = load (file_name, &if_.eip, &if_.esp);

  debug("%s#%d: start_process(...): load returned %d\n",
        thread_current()->name,
        thread_current()->tid,
        success);

  if (success)
  {
    /* We managed to load the new program to a process, and have
       allocated memory for a process stack. The stack top is in
       if_.esp, now we must prepare and place the arguments to main on
       the stack. */

    /* A temporary solution is to modify the stack pointer to
       "pretend" the arguments are present on the stack. A normal
       C-function expects the stack to contain, in order, the return
       address, the first argument, the second argument etc. */

    // HACK if_.esp -= 12; /* Unacceptable solution. */
    parameters->init_ok = true;
    if_.esp = setup_main_stack(parameters->command_line, if_.esp);

    /* The stack and stack pointer should be setup correct just before
       the process start, so this is the place to dump stack content
       for debug purposes. Disable the dump when it works. */

//    dump_stack ( PHYS_BASE + 15, PHYS_BASE - if_.esp + 16 );

  }

  debug("%s#%d: start_process(\"%s\") DONE\n",
        thread_current()->name,
        thread_current()->tid,
        parameters->command_line);


  /* If load fail, quit. Load may fail for several reasons.
     Some simple examples:
     - File doeas not exist
     - File do not contain a valid program
     - Not enough memory
  */
  if ( ! success )
  {
    thread_exit ();
  }

  /* Start the user process by simulating a return from an interrupt,
     implemented by intr_exit (in threads/intr-stubs.S). Because
     intr_exit takes all of its arguments on the stack in the form of
     a `struct intr_frame', we just point the stack pointer (%esp) to
     our stack frame and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Wait for process `child_id' to die and then return its exit
   status. If it was terminated by the kernel (i.e. killed due to an
   exception), return -1. If `child_id' is invalid or if it was not a
   child of the calling process, or if process_wait() has already been
   successfully called for the given `child_id', return -1
   immediately, without waiting.

   This function will be implemented last, after a communication
   mechanism between parent and child is established. */
int
process_wait (int child_id)
{
  int status = -1;
  struct thread *cur = thread_current ();

  debug("%s#%d: process_wait(%d) ENTERED\n",
        cur->name, cur->tid, child_id);
  /* Yes! You need to do something good here ! */
  debug("%s#%d: process_wait(%d) RETURNS %d\n",
        cur->name, cur->tid, child_id, status);

  return status;
}

/* Free the current process's resources. This function is called
   automatically from thread_exit() to make sure cleanup of any
   process resources is always done. That is correct behaviour. But
   know that thread_exit() is called at many places inside the kernel,
   mostly in case of some unrecoverable error in a thread.

   In such case it may happen that some data is not yet available, or
   initialized. You must make sure that nay data needed IS available
   or initialized to something sane, or else that any such situation
   is detected.
*/

void
process_cleanup (void)
{
  struct thread  *cur = thread_current ();
  uint32_t       *pd  = cur->pagedir;
  int status = -1;

  debug("%s#%d: process_cleanup() ENTERED\n", cur->name, cur->tid);

  /* Later tests DEPEND on this output to work correct. You will have
   * to find the actual exit status in your process list. It is
   * important to do this printf BEFORE you tell the parent process
   * that you exit.  (Since the parent may be the main() function,
   * that may sometimes poweroff as soon as process_wait() returns,
   * possibly before the printf is completed.)
   */
  printf("%s: exit(%d)\n", thread_name(), status);

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  if (pd != NULL)
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
  debug("%s#%d: process_cleanup() DONE with status %d\n",
        cur->name, cur->tid, status);
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}
