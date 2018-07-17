#include <stdio.h>
#include <stdlib.h>
#include "ppm.h"

int main(int argc, char *argv[])
{
  int ret = 1;

  if(argc > 2)
  {
    printf("Testing password : %s\n", argv[1]);

    char *errmsg = NULL;
    Entry pEntry;
    pEntry.e_nname.bv_val=argv[1];
    pEntry.e_name.bv_val=argv[1];
  
    ppm_test=1; // enable ppm_test for informing ppm not to use syslog

    ret = check_password(argv[2], &errmsg, &pEntry);
  
    if(ret == 0)
    {
      printf("Password is OK!\n");
    }
    else
    {
      printf("Password failed checks : %s\n", errmsg);
    }

    ber_memfree(errmsg);
    return ret;

  }

  return ret;
}



