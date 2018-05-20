#include <stddef.h>
#include "process.h"
#include "plist.h"
#include "../threads/synch.h"
#include <stdio.h>

void process_list_init(struct System_process_list *SPL)
{
  if (SPL == NULL)
    return -1;
  lock_init(&SPL->l);
  {
    int i = 0;
    for (; i < MAX_PROCESS; ++i)
    {
      SPL->plist_[i] = NULL;
    }
  }
}

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
  struct Process *tmp = NULL;

  for (int i = 0; i < MAX_PROCESS; i++)
  {
    tmp = SPL->plist_[i];
    if (tmp != NULL)
    {
      if (SPL->plist_[i]->process_id == id)
      {
        return SPL->plist_[i];
      }
    }
  }
  return NULL;
}

bool process_list_remove(struct System_process_list *SPL, int id)
{
  if (SPL == NULL)
  {
    return false;
  }
  struct Process *tmp = NULL;
  for (int i = 0; i < MAX_PROCESS; i++)
  {
    tmp = SPL->plist_[i];
    if (tmp != NULL)
    {
      if (tmp->process_id == id)
      {
        lock_acquire(&SPL->l);
        free(SPL->plist_[i]);
        SPL->plist_[i] = NULL;
        lock_release(&SPL->l);
        return true;
      }
    }
  }
  return false;
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
      struct Process *tmp = SPL->plist_[i];
      if (SPL->plist_[i] == NULL)
      {
        break;
      }
      else if (SPL->plist_[i]->process_id == 0)
      {
        break;
      }

      debug("%i\t %i\t\t %s\t\t %i \n",
            tmp->process_id,
            tmp->parent_id,
            tmp->process_name,
            tmp->exit_status);
    }

    debug("\n");
    lock_release(&SPL->l);
  }
}
