#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "spc_smc_decrypt.h"
   
SPC_SMC_START_BLOCK(test)
int test_routine(void) {
  int x;
   
  for (x = 0;  x < 10;  x++) printf("decrpyted!\n");
  return x;
}
SPC_SMC_END_BLOCK(test)
   
SPC_SMC_START_KEY(test)
int bogus_routine(void) {
  int x, y;
   
  for (x = 0;  x < y;  x++) {
    y = x + 256;
    y /= 32;
    x = y * 2 / 24;
  }
  return 1;
} 
SPC_SMC_END_KEY(test)
   
int main(int argc, char *argv[  ]) {
  int rt = -1;
  int decrypt = 1;
  while((rt = getopt(argc, argv, "d")) != -1) {
	switch (rt) {
		case 'd':
			decrypt = 0;
			break;
		default:
		break;
	}
  }
  if (decrypt != 0) {
  	spc_smc_decrypt(SPC_SMC_BLOCK_ADDR(test), SPC_SMC_BLOCK_LEN(test),
        	          SPC_SMC_KEY_ADDR(test), SPC_SMC_KEY_LEN(test));
  }
   
#ifdef UNENCRYPTED_BUILD
  /* This printf(  ) displays the parameters to pass to the smc_encrypt utility on
   * stdout.  The printf(  ) must be removed, and the program recompiled before
   * running smc_encrypt.  Having the printf(  ) at the end of the file prevents
   * the offsets from changing after recompilation.
   */
  printf("(offsets from _start)offset: 0x%X len 0x%X key 0x%X len 0x%X\n",
         SPC_SMC_OFFSET(SPC_SMC_BLOCK_ADDR(test)), SPC_SMC_BLOCK_LEN(test),
         SPC_SMC_OFFSET(SPC_SMC_KEY_ADDR(test)), SPC_SMC_KEY_LEN(test));
  exit(0);
#endif
   
  test_routine(  );
  return 0;
}
