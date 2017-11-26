#include <r_lib.h>
#include <r_crypto.h>
#include "serpent_algo.h"

static struct serpent_state st = {{0}};

static bool serpent_set_key (RCrypto *cry, const ut8 *key, int keylen, int mode, int direction) {
	if (!(keylen == 128 / 8 || keylen == 192 / 8 || keylen == 256 / 8)) {
		return false;
	}
	st.key_size_dw = keylen/4;
    // TODO verify indianess
	memcpy (st.key, key, keylen);
	cry->dir = direction;
	return true;
}

static int serpent_get_key_size (RCrypto *cry) {
	return st.key_size_dw;
}

static bool serpent_use (const char *algo) {
	return !strcmp (algo, "serpent-ecb");
}

#define BLOCK_SIZE 16

static bool update (RCrypto *cry, const ut8 *buf, int len) {
	// Pad to the block size, do not append dummy block
	const int diff = (BLOCK_SIZE - (len % BLOCK_SIZE)) % BLOCK_SIZE;
	const int size = len + diff;
	const int blocks = size / BLOCK_SIZE;
	int i;

	ut8 *const obuf = calloc (1, size);
	if (!obuf) {
		return false;
	}
	ut8 *const ibuf = calloc (1, size);
	if (!ibuf) {
		free (obuf);
		return false;
	}

	memset (ibuf, 0, size);
	memcpy (ibuf, buf, len);
	// Padding should start like 100000...
	if (diff) {
		ibuf[len] = 8; //0b1000;
	}

	if (cry->dir == 0) {
		for (i = 0; i < blocks; i++) {
			const int delta = BLOCK_SIZE * i;
			serpent_encrypt (&st, (ut32 *)(ibuf + delta), (ut32 *)(obuf + delta));
		}
	} else if (cry->dir > 0) {
		for (i = 0; i < blocks; i++) {
			const int delta = BLOCK_SIZE * i;
			serpent_decrypt (&st, (ut32 *)(ibuf + delta), (ut32 *)(obuf + delta));
		}
	}

	// printf("%128s\n", obuf);

	r_crypto_append (cry, obuf, size);
	free (obuf);
	free (ibuf);
	return true;
}

static bool final (RCrypto *cry, const ut8 *buf, int len) {
	return update (cry, buf, len);
}

RCryptoPlugin r_crypto_plugin_serpent = { 
	.name = "serpent-ecb",
	.set_key = serpent_set_key,
	.get_key_size = serpent_get_key_size,
	.use = serpent_use,
	.update = update,
	.final = final
};

#ifndef CORELIB
RLibStruct radare_plugin = { 
	.type = R_LIB_TYPE_CRYPTO,
	.data = &r_crypto_plugin_serpent,
	.version = R2_VERSION
};
#endif

