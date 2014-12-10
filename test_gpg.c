#include <gpgme.h>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* 
 * 1. gen a symmetric key 
 * 2. encrypt data with the symmetric key
 * 3. encrypt the symmetric key with a peer's public key
 * 4. send the encrypted symmetric to peer (virtual)
 * 5. peer decrypt the symmetric key with his secret key
 * 6. peer use the decrypted symmetric key to decrypt the encrypted data
 * 7. display the data 
 */


#define fail_if_err(err)					\
  do								\
    {								\
      if (err)							\
        {							\
          fprintf (stderr, "%s:%d: %s: %s\n",			\
                   __FILE__, __LINE__, gpgme_strsource (err),	\
		   gpgme_strerror (err));			\
          exit (1);						\
        }							\
    }								\
  while (0)

void
init_gpgme (gpgme_protocol_t proto)
{
  gpgme_error_t err;

  gpgme_check_version (NULL);
  setlocale (LC_ALL, "");
  gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
#ifndef HAVE_W32_SYSTEM
  gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
#endif

  err = gpgme_engine_check_version (proto);
  fail_if_err (err);
}

void
print_data (gpgme_data_t dh)
{
#define BUF_SIZE 512
  char buf[BUF_SIZE + 1];
  int ret;
  
  ret = gpgme_data_seek (dh, 0, SEEK_SET);
  if (ret)
    fail_if_err (gpgme_err_code_from_errno (errno));
  while ((ret = gpgme_data_read (dh, buf, BUF_SIZE)) > 0)
    fwrite (buf, ret, 1, stdout);
  if (ret < 0)
    fail_if_err (gpgme_err_code_from_errno (errno));
}

void
write_data (gpgme_data_t dh, FILE *fd)
{
#define BUF_SIZE 512
  char buf[BUF_SIZE + 1];
  int ret;
  
  ret = gpgme_data_seek (dh, 0, SEEK_SET);
  if (ret)
    fail_if_err (gpgme_err_code_from_errno (errno));
  while ((ret = gpgme_data_read (dh, buf, BUF_SIZE)) > 0)
    fwrite (buf, ret, 1, fd);
  if (ret < 0)
    fail_if_err (gpgme_err_code_from_errno (errno));
}

gpgme_error_t
passphrase_cb (void *opaque, const char *uid_hint, const char *passphrase_info,
	       int last_was_bad, int fd)
{
#ifdef HAVE_W32_SYSTEM
  DWORD written;
  WriteFile ((HANDLE) fd, "", 4, &written, 0);
#else
  int res;
  char *pass = "";
  int passlen = strlen (pass);
  int off = 0;

  do
    {
      res = write (fd, &pass[off], passlen - off);
      if (res > 0)
	off += res;
    }
  while (res > 0 && off != passlen);

  return off == passlen ? 0 : gpgme_error_from_errno (errno);
#endif

  return 0;
}

char *
make_filename (const char *fname)
{
  const char *srcdir = getenv ("srcdir");
  char *buf;

  if (!srcdir)
    srcdir = ".";
  buf = malloc (strlen(srcdir) + strlen(fname) + 2);
  if (!buf) 
    exit (8);
  strcpy (buf, srcdir);
  strcat (buf, "/");
  strcat (buf, fname);
  return buf;
}

void encryptdata(const char *data, gpgme_data_t out)
{
	gpgme_ctx_t ctx;
	gpgme_data_t in;
	gpgme_encrypt_result_t result;
	gpgme_error_t err;
	gpgme_key_t key[2] = {NULL, NULL};

	init_gpgme (GPGME_PROTOCOL_OpenPGP);
	
	err = gpgme_new (&ctx);
	fail_if_err (err);

	gpgme_set_armor (ctx, 1);
	
	err = gpgme_data_new_from_mem (&in, data, strlen(data), 0);
	fail_if_err (err);
	
	err = gpgme_get_key (ctx, "BB421B1352D338A17F498EE42C6AC400A37827FD",
	      	       &key[0], 0);
	fail_if_err (err);
	
	err = gpgme_op_encrypt (ctx, key, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);
	fail_if_err (err);
	result = gpgme_op_encrypt_result (ctx);
	if (result->invalid_recipients) {
	    fprintf (stderr, "Invalid recipient encountered: %s\n",
	             result->invalid_recipients->fpr);
	    exit (1);
	}
	
	gpgme_key_unref (key[0]);
	gpgme_data_release (in);
	gpgme_release (ctx);
}

void decryptcipher(gpgme_data_t cipher, const char *filename,  gpgme_data_t plain)
{
	gpgme_ctx_t ctx;
	gpgme_decrypt_result_t result;
	gpgme_error_t err;
	const char *cipher_1_asc = make_filename (filename);
	char *agent_info;

	init_gpgme (GPGME_PROTOCOL_OpenPGP);
	
	err = gpgme_new (&ctx);
	fail_if_err (err);

	agent_info = getenv("GPG_AGENT_INFO");
	if (!(agent_info && strchr (agent_info, ':')))
	  gpgme_set_passphrase_cb (ctx, passphrase_cb, NULL);

	err = gpgme_data_new_from_file (&cipher, cipher_1_asc, 1);
	fail_if_err (err);

	err = gpgme_op_decrypt (ctx, cipher, plain);
	fail_if_err (err);
	result = gpgme_op_decrypt_result (ctx);
	if (result->unsupported_algorithm) {
	    fprintf (stderr, "%s:%i: unsupported algorithm: %s\n",
	             __FILE__, __LINE__, result->unsupported_algorithm);
	    exit (1);
	}
	gpgme_release (ctx);
}

int main()
{
	gpgme_error_t err;
	gpgme_data_t cipher, plain;
	FILE *fd;
	const char *data = "This is a test text for gpg         \n";

	err = gpgme_data_new (&cipher);
	fail_if_err (err);

	err = gpgme_data_new (&plain);
	fail_if_err (err);

	encryptdata(data, cipher);
	print_data(cipher);
	fd = fopen("cipher-1.asc", "w");
	write_data(cipher, fd);
	fclose(fd);

	decryptcipher(cipher, "cipher-1.asc", plain);
	print_data(plain);
	
	gpgme_data_release (cipher);
	gpgme_data_release (plain);
}
