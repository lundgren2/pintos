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
      SPL->plist_[i].free = true;
    }
  }
}

int process_list_insert(struct System_process_list *SPL, struct Process p)
{
  if (SPL == NULL)
  {
    return -1;
  }
  lock_acquire(&SPL->l);

  for (int i = 0; i < MAX_PROCESS; ++i)
  {
    if (SPL->plist_[i].free)
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
  for (int i = 0; i < MAX_PROCESS; i++)
  {
    if (SPL->plist_[i].process_id == id)
    {
      return &SPL->plist_[i];
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
  for (int i = 0; i < MAX_PROCESS; i++)
  {
    if (SPL->plist_[i].process_id == id)
    {
      lock_acquire(&SPL->l);
      SPL->plist_[id].free = true;
      lock_release(&SPL->l);
      return true;
    }
  }
  return false;
}

void process_list_print(struct System_process_list *SPL)
{
  if (SPL == NULL)
  {
    return NULL;
  }
  lock_acquire(&SPL->l);
  printf("\n\t\t==== PROCESS LIST ====\n");
  printf("ID\t PARENT ID\t INDEX\t\t EXIT_STATUS\n");

  int i = 0;
  for (; i < MAX_PROCESS; i++)
  {
    if (SPL->plist_[i].process_id == 0)
    {
      break;
    }
    printf("%i\t %i\t\t %i\t\t %i \n",
           SPL->plist_[i].process_id,
           SPL->plist_[i].parent_id,
           i,
           SPL->plist_[i].exit_status);
  }

  printf("\n");
  lock_release(&SPL->l);
}
