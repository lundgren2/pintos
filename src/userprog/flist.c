#include <debug.h>
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
  if(k > MAP_SIZE) {
    return -1;
  }
  if (m->content[k - 2] == NULL)
  {
    return -1;
  }
  return m->content[k - 2];
}

value_t map_remove(struct map *m, key_t k)
{
  if(k > MAP_SIZE) {
    return NULL;
  }
  value_t tmp = m->content[k - 2];
  m->content[k - 2] = NULL;
  return tmp;
}

void map_remove_if(struct map *m, value_t v, int aux)
{
  if (v != NULL && m != NULL)
  {
    for (int key = 0; key < MAP_SIZE; key++)
    {
      if (m->content[key] != NULL && v == map_find(m, key))
      {
        debug("map_remove_if map[%d]: %d\n", key, v);
          map_remove(m, key);
      }
    }
   } else {
    debug("# v is null\n");
  }
}

void map_for_each(struct map *m,
                  void (*exec)(key_t k, value_t v, int aux),
                  int aux)
{
  for (int key = 0; key < MAP_SIZE; key++)
  {
    if (m->content[key] != NULL)
    {
      debug("for_each map[%d]: %d", key, m->content[key]);
      exec(key, m->content[key], aux);
    }
  }
}