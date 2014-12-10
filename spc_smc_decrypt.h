#include <stdio.h>

#define SPC_SMC_START_BLOCK(label)  void label(void) {  }
#define SPC_SMC_END_BLOCK(label)    void _##label(void) {  }
#define SPC_SMC_BLOCK_LEN(label)    (int)_##label - (int)label
#define SPC_SMC_BLOCK_ADDR(label)   (unsigned char *)label
#define SPC_SMC_START_KEY(label)    void key_##label(void) {  }
#define SPC_SMC_END_KEY(label)      void _key_##label(void) {  }
#define SPC_SMC_KEY_LEN(label)      (int)_key_##label - (int)key_##label
#define SPC_SMC_KEY_ADDR(label)     (unsigned char *)key_##label
#define SPC_SMC_OFFSET(label)       (long)label - (long)_start


typedef struct {
  unsigned char sbox[256];
  unsigned char i, j;
} RC4_CTX;
   
extern void _start(void);

void RC4_set_key(RC4_CTX *c, size_t keybytes, unsigned char *key);

void RC4(RC4_CTX *c, size_t n, unsigned char *in, unsigned char *out);

/* returns number of bytes encoded */
int spc_smc_decrypt(unsigned char *buf, int buf_len, unsigned char *key, int key_len);
