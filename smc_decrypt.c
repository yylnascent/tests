#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "spc_smc_decrypt.h"   

/* ELF-specific stuff */
#define ELF_ENTRY_OFFSET  24 /* e_hdr e_entry field offset */
#define ELF_PHOFF_OFFSET  28 /* e_hdr e_phoff field offset */
#define ELF_PHESZ_OFFSET  42 /* e_hdr e_phentsize field offset */
#define ELF_PHNUM_OFFSET  44 /* e_hdr e_phnum field offset */
#define ELF_PH_OFFSET_OFF 4  /* p_hdr p_offset field offset */
#define ELF_PH_VADDR_OFF  8  /* p_hdr p_vaddr field offset */
#define ELF_PH_FILESZ_OFF 16 /* p_hdr p_size field offset */
   
static unsigned long elf_get_entry(unsigned char *buf) {
  unsigned long  entry, p_vaddr, p_filesz, p_offset;
  unsigned int   i, phoff;
  unsigned short phnum, phsz;
  unsigned char  *phdr;
   
  entry  = *(unsigned long *) &buf[ELF_ENTRY_OFFSET];
  phoff  = *(unsigned int *) &buf[ELF_PHOFF_OFFSET];
  phnum  = *(unsigned short *) &buf[ELF_PHNUM_OFFSET];
  phsz  = *(unsigned short *) &buf[ELF_PHESZ_OFFSET];
   
  phdr = &buf[phoff];
  /* iterate through program headers */
  for ( i = 0; i < phnum; i++, phdr += phsz ) {
    p_vaddr = *(unsigned long *)&phdr[ELF_PH_VADDR_OFF];
    p_filesz = *(unsigned long *)&phdr[ELF_PH_FILESZ_OFF];
    /* if entry point is in this program segment */
    if ( entry >= p_vaddr && entry < (p_vaddr + p_filesz) ) {
      /* calculate offset of entry point */
      p_offset = *(unsigned long *)&phdr[ELF_PH_OFFSET_OFF];
      return( p_offset + (entry - p_vaddr) );
    }
  }
  return 0;
}
   
int main(int argc, char *argv[  ]) {
  unsigned long entry, offset, len, key_offset, key_len;
  unsigned char *buf;
  struct stat   sb;
  int           fd;
   
  if (argc < 6) {
    printf("Usage: %s filename offset len key_offset key_len\n"
           "       filename:   file to encrypt\n"
           "       offset:     offset in file to start encryption\n"
           "       len:        number of bytes to encrypt\n"
           "       key_offset: offset in file of key\n"
           "       key_len:    number of bytes in key\n"
           "       Values are converted with strtol with base 0\n",
           argv[0]);
    return 1;
  }
   
  /* prepare the parameters */
  offset = strtoul(argv[2], 0, 0);
  len = strtoul(argv[3], 0, 0);
  key_offset = strtoul(argv[4], 0, 0);
  key_len = strtoul(argv[5], NULL, 0);
   
  /* memory map the file so we can access it via pointers */
  if (stat(argv[1], &sb)) {
    fprintf(stderr, "Stat failed: %s\n", strerror(errno));
    return 2;
  }
  if ((fd = open(argv[1], O_RDWR | O_EXCL))  == -1) {
    fprintf(stderr, "Open failed: %s\n", strerror(errno));
    return 3;
  }
  buf = mmap(0, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if ((int)buf == -1) {
    fprintf(stderr, "Open failed: %s\n", strerror(errno));
    close(fd);
    return 4;
  }
   
  /* get entry point : here we assume ELF example */
  entry = elf_get_entry(buf);
  if (!entry) {
    fprintf(stderr, "Invalid ELF header\n");
    munmap(buf, sb.st_size);
    close(fd);
    return 5;
  }
   
  /* these are offsets from the entry point */
  offset += entry;
  key_offset += entry;
   
  printf("Encrypting %ld bytes at 0x%X with %ld bytes at 0x%X\n",
         len, offset, key_len, key_offset);
   
  /* Because we're using RC4, encryption and decryption are the same operation */
  spc_smc_decrypt(buf + offset, len, buf + key_offset, key_len);
   
  /* mem-unmap the file */
  msync(buf, sb.st_size, MS_SYNC);
  munmap(buf, sb.st_size);
  close(fd);
  return 0;
}
