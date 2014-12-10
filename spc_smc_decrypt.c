#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

typedef struct {
  unsigned char sbox[256];
  unsigned char i, j;
} RC4_CTX;
   
void RC4_set_key(RC4_CTX *c, size_t keybytes, unsigned char *key) {
  int           i, j;
  unsigned char keyarr[256], swap;
   
  c->i = c->j = 0;
  for (i = j = 0;  i < 256;  i++, j = (j + 1) % keybytes) {
    c->sbox[i] = i;
    keyarr[i] = key[j];
  }
  for (i = j = 0;  i < 256;  i++) {
    j += c->sbox[i] + keyarr[i];
    j %= 256;
    swap = c->sbox[i];
    c->sbox[i] = c->sbox[j];
    c->sbox[j] = swap;
  }
}


void RC4(RC4_CTX *c, size_t n, unsigned char *in, unsigned char *out) {
  unsigned char swap;
   
  while (n--) {
    c->j += c->sbox[++c->i];
    swap = c->sbox[c->i];
    c->sbox[c->i] = c->sbox[c->j];
    c->sbox[c->j] = swap;
    swap = c->sbox[c->i] + c->sbox[c->j];
    *out++ = *in++ ^ c->sbox[swap];
  }
}
extern void _start(void);
   
/* returns number of bytes encoded */
int spc_smc_decrypt(unsigned char *buf, int buf_len, unsigned char *key, int key_len) {
  RC4_CTX ctx;
  char *p = NULL;
  long    psize = sysconf(_SC_PAGESIZE);
   
  RC4_set_key(&ctx, key_len, key);
   
  /* NOTE: most code segments have read-only permissions, and so must be modified
   * to allow writing to the buffer
   */
  if (mprotect(buf, buf_len, PROT_WRITE | PROT_READ | PROT_EXEC)) {
  //  fprintf(stderr, "mprotect: %s %d\n", strerror(errno), errno);
    for (p = (char *)buf; (unsigned long)p % psize; --p);

    if (mprotect(p, buf_len, PROT_WRITE | PROT_READ | PROT_EXEC)) {
      fprintf(stderr, "mprotect: %s %d\n", strerror(errno), errno);
      return(0);
    }
  } 
   
  /* decrypt the buffer */
  RC4(&ctx, buf_len, buf, buf);
   
  /* restore the original memory permissions */
  if (p) {
    mprotect(p, buf_len, PROT_READ | PROT_EXEC);
  } else {
    mprotect(buf, buf_len, PROT_READ | PROT_EXEC);
  }
   
  return(buf_len);
}
