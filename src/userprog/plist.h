#ifndef _PLIST_H_
#define _PLIST_H_
#define MAX_PROCESS 64
#include "../threads/synch.h" // semaphore

struct Process
{
  int process_id;
  char process_name[64];
  int parent_id;
  int exit_status;
  bool free;
  bool process_alive;
  bool parent_alive;
  // new 2018-05-19
  struct semaphore sema;
};
static struct System_process_list
{
  struct Process* plist_[MAX_PROCESS];
  struct lock l;
};

void process_list_init(struct System_process_list *SPL);
int process_list_insert(struct System_process_list *SPL, struct Process *p);
struct Process *process_list_find(struct System_process_list *SPL, int id);
bool process_list_remove(struct System_process_list *SPL, int id);
void process_list_print(struct System_process_list *SPL);

/* Place functions to handle a running process here (process list).

   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.

   - A function that print the entire content of the list in a nice,
     clean, readable format.

 */

#endif
