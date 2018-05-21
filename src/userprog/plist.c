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
  struct Process *process = NULL;
  for (int i = 0; i < MAX_PROCESS; ++i)
  {
    process = SPL->plist_[i];
    process->free = true;
    sema_init(&process->sema, 0);
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
  struct Process *process = NULL;
  for (int i = 0; i < MAX_PROCESS; ++i)
  {
    process = SPL->plist_[i];
    if (process != NULL && process->free)
    {
      process = p;
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
    if (process != NULL)
    {
      if (process->id == id)
      {
        return process;
      }
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
  struct Process *process = NULL;
  for (int i = 0; i < MAX_PROCESS; i++)
  {
    process = SPL->plist_[i];
    if (process != NULL)
    {
      if (process->id == id)
      {
        lock_acquire(&SPL->l);
        exit_status = process->exit_status;
        free(process->name);
        process->free = true;
        process->alive = false;
        lock_release(&SPL->l);
        return exit_status;
      }
    }
  }
  return -2;
}

void process_list_print(struct System_process_list *SPL)
{
  if (SPL != NULL)
  {
    lock_acquire(&SPL->l);
    debug("\n\t\t==== PROCESS LIST ====\n");
    debug("ID\t PARENT ID\t NAME\t\t EXIT_STATUS\n");

    for (int i = 0; i < MAX_PROCESS; i++)
    {
      struct Process *process = SPL->plist_[i];
      if (process == NULL)
      {
        break;
      }
      else if (process->id == 0)
      {
        break;
      }

      debug("%i\t %i\t\t %s\t\t %s\t\t %i \n",
            process->id,
            process->parent_id,
            process->name,
            process->free ? "FREE" : "BUSY",
            process->exit_status);
    }

    debug("\n");
    lock_release(&SPL->l);
  }
}
