#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DW_BY_BLOCK 4
#define DW_BY_USERKEY 8
#define NB_ROUNDS 32
#define NB_SUBKEYS 33
#define NIBBLES_BY_SUBKEY 32
#define eprintf printf

struct serpent_state {
    uint32_t *key;
    int key_size_dw;
};


/*
 * st: A pointer to a serpent_state structure containing the key and the key size.
 * in: A block of data to be encrypted.
 * out: When the function returns, the block of data encrypted by serpent
 *      with the key contained in st.
 */
void serpent_encrypt(struct serpent_state *st, uint32_t in[DW_BY_BLOCK], uint32_t out[DW_BY_BLOCK]);

/*
 * st: A pointer to a serpent_state structure containing the key and the key size.
 * in: A block of data to be decrypted.
 * out: When the function returns, the block of data decrypted by serpent
 *      with the key contained in st.
 */
void serpent_decrypt(struct serpent_state *st, uint32_t in[DW_BY_BLOCK], uint32_t out[DW_BY_BLOCK]);


/*
 * st: A serpent_state structure containing the key and the key size.
 * subkeys: When the function returns, an array of double words containings
 *          all the subkeys needed for the encryptio/dcryption with serpent.
 */
void serpent_keyschedule(struct serpent_state st,
        uint32_t subkeys[NB_SUBKEYS*DW_BY_BLOCK]);

//RCryptoPlugin r_crypto_plugin_serpent = { 
//	.name = "serpent",
//	.set_key = serpent_set_key,
//	.get_key_size = serpent_get_key_size,
//	.use = serpent_use,
//	.update = update,
//	.final = final
//};
