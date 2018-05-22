#include <stddef.h>
#include "process.h"
#include "plist.h"
#include "../threads/synch.h"
#include <stdio.h>

void process_list_init(struct System_process_list *SPL)
{
  if (SPL == NULL)
  {
    return -1;
  }
  lock_init(&SPL->l);
  for (int i = 0; i < MAX_PROCESS; ++i)
  {
    SPL->plist_[i] = NULL;
  }
}

// TODO: Check if insert works after NULL fix
int process_list_insert(struct System_process_list *SPL, struct Process *p)
{
  if (SPL == NULL)
  {
    return -1;
  }
  lock_acquire(&SPL->l);
  for (int i = 0; i < MAX_PROCESS; ++i)
  {
    if (SPL->plist_[i] == NULL)
    {
      SPL->plist_[i] = p;
      lock_release(&SPL->l);
      return i;
    }
  }
  lock_release(&SPL->l);
  return -1;
}

struct Process *process_list_find(struct System_process_list *SPL, int id)
{
  if (SPL == NULL)
  {
    return NULL;
  }
  struct Process *process = NULL;
  for (int i = 0; i < MAX_PROCESS; i++)
  {
    process = SPL->plist_[i];
    if (process != NULL && process->id == id)
    {
      return process;
    }
  }
  return NULL;
}

// remove process from process list and return exit_status for the process
int process_list_remove(struct System_process_list *SPL, int id)
{
  if (SPL == NULL)
  {
    return false;
  }
  int exit_status;
  for (int i = 0; i < MAX_PROCESS; i++)
  {
    if (SPL->plist_[i] != NULL && SPL->plist_[i]->id == id)
    {
      lock_acquire(&SPL->l);
      exit_status = SPL->plist_[i]->exit_status;
      free(SPL->plist_[i]);
      SPL->plist_[i] = NULL;
      lock_release(&SPL->l);
      return exit_status;
    }
  }
  return -2;
}

void process_list_print(struct System_process_list *SPL)
{
  if (SPL != NULL)
  {
    lock_acquire(&SPL->l);
    debug("==== PROCESS LIST ====\n");
    debug("ID\t PARENT ID\t NAME\t\t EXIT_STATUS\n");

    struct Process *process = NULL;
    for (int i = 0; i < MAX_PROCESS; i++)
    {
      process = SPL->plist_[i];
      if (process != NULL)
      {
        debug("%i\t %i\t\t %s\t\t  %i\n",
              process->id,
              process->parent_id,
              process->name,
              process->exit_status);
      }
    }
    debug("\n");
    lock_release(&SPL->l);
  }
}
