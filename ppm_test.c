#include <stdio.h>
#include <stdlib.h>
#include "ppm.h"

int main(int argc, char *argv[])
{

  if(argc > 0)
  {
    printf("Testing password : %s\n", argv[1]);

    char *errmsg = NULL;
    Entry pEntry;
    pEntry.e_name.bv_val="user";
    int ret;
  
    ret = check_password(argv[1], &errmsg, &pEntry);
  
    printf("ret: %d\n", ret);
  
    ber_memfree(errmsg);
    return ret;

  }

  return EXIT_SUCCESS;
}



