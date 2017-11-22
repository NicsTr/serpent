#ifndef __serpent_h_
#define __serpent_h_
#include "serpent.h"
#endif


static inline void rotr(uint32_t *x, int s) {
	*x = (*x >> s) | (*x << (32 - s));
}

static inline void rotl(uint32_t *x, int s) {
	*x = (*x << s) | (*x >> (32 - s));
}

/* Apply SBox to the four least significant bits */
static inline uint8_t apply_sbox(int si, uint8_t x) {
    x = S[si][x & 0xf];
    return x;
}

static inline uint8_t get_bit(int i, uint32_t input) {
    if (i >= 32) {
        eprintf("Wrong bit asked");
        exit(1);
    }
    return (input >> i) & 1;
}

void apply_IP(uint32_t in[DW_BY_BLOCK], uint32_t out[DW_BY_BLOCK]) {
    int index;
    for (int i = 0; i < DW_BY_BLOCK*32; i++) {
        index = IPTable[i];
        out[i/32] ^= (-(uint32_t)get_bit(index%32, in[index/32])^out[i/32])
                & (1 << i);
    }
}

void serpent_keyschedule(struct serpent_state st,
        uint32_t subkeys[NB_SUBKEYS*DW_BY_BLOCK]) {
    if (st.key_size_dw != 128 && st.key_size_dw != 192 
            && st.key_size_dw != 256) {
        eprintf("Invalid key size");
        exit(1);
    }

    uint32_t tmpkeys[DW_BY_BLOCK*NB_SUBKEYS+DW_BY_USERKEY] = {0};
    const uint32_t phi = 0x9e3779b9;
    int si;
    uint8_t in, out;

    for (int i = 0; i < st.key_size_dw; i++) {
        tmpkeys[i] = st.key[i];
    }
    // Padding key
    tmpkeys[st.key_size_dw] = 1;



    for (int i = DW_BY_USERKEY; i < NB_SUBKEYS*DW_BY_BLOCK; i++) {
        tmpkeys[i] = tmpkeys[i-8]^tmpkeys[i-5]^tmpkeys[i-3]^tmpkeys[i-1]
            ^phi^(i-8);
        rotl(&(tmpkeys[i]), 11);
    }
    
    // Applying sbox for subkey i
    for (int i = 0; i < NB_SUBKEYS; i++) {
        si = (3 - i) % 8;

        // Iterates over all nibbles of the subkey i
        for (int j = 0; j < NIBBLES_BY_SUBKEY; j++) {
            in = get_bit(j, tmpkeys[0+DW_BY_BLOCK*i])
                | get_bit(j, tmpkeys[1+DW_BY_BLOCK*i]) << 1
                | get_bit(j, tmpkeys[2+DW_BY_BLOCK*i]) << 2
                | get_bit(j, tmpkeys[3+DW_BY_BLOCK*i]) << 3;
            out = apply_sbox(si, in);
            for (int l = 0; l < DW_BY_BLOCK; l++) {
                subkeys[l+DW_BY_BLOCK*i] |= get_bit(l, (uint32_t)out) << j;
            }
        }
        printf("%08x", subkeys[0+i*DW_BY_BLOCK]);
        printf("%08x", subkeys[1+i*DW_BY_BLOCK]);
        printf("%08x", subkeys[2+i*DW_BY_BLOCK]);
        printf("%08x\n", subkeys[3+i*DW_BY_BLOCK]);
    }
    
    // Apply IP on every subkey
    for (int i = 0; i < NB_SUBKEYS; i++) {
        apply_IP(subkeys, &(tmpkeys[DW_BY_USERKEY]));
    }

    memcpy(subkeys, &(tmpkeys[DW_BY_USERKEY]), 132*sizeof(uint32_t));
}
