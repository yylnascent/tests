#include <gcrypt.h>
#include <stdio.h>


static void
show_sexp(gcry_sexp_t key)
{
	size_t size;
	char *buf;

	size = gcry_sexp_sprint (key, GCRYSEXP_FMT_ADVANCED, NULL, 0);
	fprintf (stderr, "%d: ", size);
	buf = gcry_xmalloc(size);
	size = gcry_sexp_sprint (key, GCRYSEXP_FMT_ADVANCED, buf, size);

	fprintf (stderr, "%.*s", (int)size, buf);
	gcry_free(buf);

}

static void
check_generated_rsa_key (gcry_sexp_t key, unsigned long expected_e)
{
  gcry_sexp_t skey, pkey, list;

  pkey = gcry_sexp_find_token (key, "public-key", 0);
  if (!pkey) {
    printf ("public part missing in return value\n");
    return;
  } else {
      show_sexp(pkey);
      gcry_mpi_t e = NULL;

      list = gcry_sexp_find_token (pkey, "e", 0);
      if (!list || !(e=gcry_sexp_nth_mpi (list, 1, 0)) ) {
        printf ("public exponent not found\n");
        return;
      } else if (!expected_e);
      else if ( gcry_mpi_cmp_ui (e, expected_e))
        {
          printf ("public exponent is not %lu\n", expected_e);
          return;
        }
      gcry_sexp_release (list);
      gcry_mpi_release (e);
    }

  skey = gcry_sexp_find_token (key, "private-key", 0);
  if (!skey) {
    printf ("private part missing in return value\n");
    return;
  }
  else
    {
      show_sexp(skey);
      int rc = gcry_pk_testkey (skey);
      if (rc) {
        printf ("gcry_pk_testkey failed: %d\n", rc);
        return;
      }
    }
}

int main(void)
{
	gcry_sexp_t keyparm, key;
	gcry_sexp_t pkey, skey;
	gcry_sexp_t data_t, ciph, plain;
	int rc;
	size_t size;
	char *buf;
	char *data = "test\nencrypt\n";

	rc = gcry_sexp_new(&keyparm,
			"(genkey\n"
			" (rsa\n"
			"  (nbits 4:1024)\n"
			"  (rsa-use-e 3:257)\n"
			" ))", 0, 1);
	if (rc) {
		printf("something goes wrong\n");
		return 1;
	}
	gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
	gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
	gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
	
	rc = gcry_pk_genkey(&key, keyparm);
	gcry_sexp_release(keyparm);
	if (rc) {
		printf("something goes wrong\n");
		return 1;
	}

	check_generated_rsa_key (key, 257);
	rc = gcry_sexp_sscan (&data_t, NULL, data, strlen(data));
	if (rc) {
		printf("get data_t fail\n");
		return 1;
	}
	show_sexp(data_t);
  	pkey = gcry_sexp_find_token (key, "public-key", 0);
	if (!pkey) {
		printf("There is no pkey\n");
		return 1;
	}
	show_sexp(pkey);
  	skey = gcry_sexp_find_token (key, "private-key", 0);
	if (!skey) {
		printf("There is no skey\n");
		return 1;
	}
	show_sexp(skey);
#if 1
	rc = gcry_pk_encrypt (&ciph, data_t, pkey);
	if (rc) {
		printf("encryption failed:%d %s\n", rc, gcry_strerror(rc));
		return 1;
	}
	show_sexp(ciph);
	rc = gcry_pk_decrypt (&plain, ciph, skey);
	if (rc) {
		printf("decryption failed:%d %s\n", rc, gcry_strerror(rc));
		return 1;
	}
	show_sexp(plain);
	gcry_sexp_release(plain);
#else
	rc = gcry_pk_sign(&ciph, data_t, skey);
	if (rc) {
		printf("sign fail:%d %s\n", rc, gcry_strerror(rc));
		return 1;
	}
	show_sexp(ciph);
	rc = gcry_pk_verify(ciph, data_t, pkey);
	if (rc) {
		printf("verify fail:%d %s\n", rc, gcry_strerror(rc));
		return 1;
	}
#endif
	
	gcry_sexp_release(ciph);
	gcry_sexp_release(data_t);
	gcry_sexp_release(key);
	
}
