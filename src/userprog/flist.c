#include <stddef.h>
#include "flist.h"

void map_init(struct map *m)
{
  int i = 0;
  while (i < MAP_SIZE)
  {
    m->content[i] = NULL;
    i++;
  }
}

key_t map_insert(struct map *m, value_t v)
{
  for (int i = 0; i < MAP_SIZE; i++)
  {
    if (m->content[i] == NULL)
    {
      m->content[i] = v;
      return i + 2;
    }
  }
  free(v); // remove pointer to uninserted file
  return -1;
}

value_t map_find(struct map *m, key_t k)
{
  return m->content[k - 2];
}

value_t map_remove(struct map *m, key_t k)
{
  value_t tmp = m->content[k - 2];
  m->content[k - 2] = NULL;
  return tmp;
}
