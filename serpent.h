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


void serpent_encrypt(struct serpent_state *st, uint32_t in[DW_BY_BLOCK], uint32_t out[DW_BY_BLOCK]);

void serpent_decrypt(struct serpent_state *st, uint32_t in[DW_BY_BLOCK], uint32_t out[DW_BY_BLOCK]);

void serpent_keyschedule(struct serpent_state st,
        uint32_t subkeys[NB_SUBKEYS*DW_BY_BLOCK]);
