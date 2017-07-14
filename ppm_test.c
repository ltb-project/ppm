#include <stdio.h>
#include <stdlib.h>
#include "ppm.h"

int main(int argc, char *argv[])
{
  int ret = 1;

  if(argc > 1)
  {
    printf("Testing password : %s\n", argv[1]);

    char *errmsg = NULL;
    Entry pEntry;
    pEntry.e_nname.bv_val="user";
    pEntry.e_name.bv_val="user";
  
    ppm_test=1; // enable ppm_test for informing ppm not to use syslog

    ret = check_password(argv[1], &errmsg, &pEntry);
  
    ber_memfree(errmsg);
    return ret;

  }

  return ret;
}



