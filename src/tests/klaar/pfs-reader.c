/* Part of pfs.c suite.
   
   Reads from the file and checks consistency.
   The buffer should all contain the same character!!
   pintos -v -k -T 120 --fs-disk=2 --qemu \
-p ../examples/pfs -a pfs \
-p ../examples/pfs_writer -a pfs_writer \
-p ../examples/pfs_reader -a pfs_reader \
-g messages -- -f -q run pfs
 */

#include <syscall.h>
#include <stdio.h>

#include "../lib.h"
#include "pfs.h"

static bool check_consistency(char* buf, int size)
{
  for (int i = 1; i < size; ++i)
  {
    if (buf[0] != buf[i])
    {
      /* Ooops, inconsistency */
      return false;
    }
  }
  return true;
}

char buffer[BIG];

int main (int argc UNUSED, char *argv[]) 
{
  test_name = argv[0];
  quiet = true;
  
  for (int i = 0; i < TIMES; ++i)
  {
    int id = open ("file.1");
    CHECK ( id > 1, "open \"file.1\"");    
    int bytes = read(id, buffer, BIG);
    CHECK ( bytes == BIG, "read \"file.1\"");    
    close(id);

    CHECK ( check_consistency(buffer, BIG), "inconsistency");
  }
  return 0;
}
