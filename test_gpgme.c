#include <gpgme.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h> 

void
init_gpgme (gpgme_protocol_t proto)
{
  gpgme_error_t err;

  gpgme_check_version (NULL);
  setlocale (LC_ALL, "");
  gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));

  err = gpgme_engine_check_version (proto);
}


void
print_data (const char *tag, gpgme_data_t dh)
{
#define BUF_SIZE 512
  char buf[BUF_SIZE + 1];
  int ret;
  
  fwrite (tag, strlen(tag), 1, stdout);

  ret = gpgme_data_seek (dh, 0, SEEK_SET);
  while ((ret = gpgme_data_read (dh, buf, BUF_SIZE)) > 0)
    fwrite (buf, ret, 1, stdout);
}

void progress(void *opaque, const char *what, int type, int current, int total)
{
	(void)opaque;
	(void)what;
	(void)type;
	(void)current;
	(void)total;
  if (!strcmp (what, "primegen") && !current && !total
      && (type == '.' || type == '+' || type == '!'
	  || type == '^' || type == '<' || type == '>'))
    {
      printf ("%c", type);
      fflush (stdout);
    }
  else
    {
      fprintf (stderr, "unknown progress `%s' %d %d %d\n", what, type,
	       current, total);
      exit (1);
    }
}

int main(void)
{
	gpgme_ctx_t ctx;
	gpgme_data_t in, out, plain;
	gpgme_genkey_result_t result;
	gpgme_encrypt_result_t eresult;
	gpgme_key_t pubkey[2] = {NULL, NULL};
	int rc;
	const char *parms = "<GnupgKeyParms format=\"internal\">\n"
	  "Key-Type: RSA\n"
	  "Key-Length: 1024\n"
	  "Name-Email: dragon@dragontec.com.cn\n"
	  "</GnupgKeyParms>\n";
	in = NULL;
	out = NULL;
	plain = NULL;
	
	init_gpgme (GPGME_PROTOCOL_OpenPGP);
	
	rc = gpgme_new(&ctx);

	rc = gpgme_set_protocol(ctx, GPGME_PROTOCOL_OpenPGP);

	gpgme_set_armor (ctx, 1);

	rc = gpgme_data_new_from_mem(&in, "Hello World\n", 12, 0);

	gpgme_set_progress_cb(ctx, progress, NULL);

	rc = gpgme_op_genkey(ctx, parms, NULL, NULL);
	if (rc) {
		printf("genkey fail\n");
		return 1;
	}

	result = gpgme_op_genkey_result (ctx);
	printf("%s\n", result->fpr);

	gpgme_get_key(ctx, result->fpr, &pubkey[0], 0);

	rc = gpgme_op_encrypt (ctx, pubkey, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);

	eresult = gpgme_op_encrypt_result(ctx);
	
	if (eresult->invalid_recipients) {
		fprintf (stderr, "Invalid recipient encountered: %s\n",
			eresult->invalid_recipients->fpr);
		return 1;
	}	

	print_data("cipher", out);
	

	rc = gpgme_op_decrypt(ctx, out, plain);
	print_data("plain", plain);

	gpgme_key_unref (pubkey[0]);
	gpgme_data_release (in);
	gpgme_data_release (out);
	gpgme_data_release (plain);
	gpgme_release(ctx);
}
