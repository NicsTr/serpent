#include "serpent.h"
#include "serpent_tables.h"

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

void apply_FP(uint32_t in[DW_BY_BLOCK], uint32_t out[DW_BY_BLOCK]) {
    int index;
    for (int i = 0; i < DW_BY_BLOCK*32; i++) {
        index = FPTable[i];
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

    for (int i = 0; i < st.key_size_dw/32; i++) {
        tmpkeys[i] = st.key[i];
    }
    // Padding key
    if (st.key_size_dw != 256) {
        tmpkeys[st.key_size_dw/32] = 1;
    }

    // TODELETE
    printf("USERKEY=");
    for (int i = DW_BY_USERKEY-1; i >= 0; i--) {
        printf("%08x", tmpkeys[i]); 
    }
    printf("\n");



    for (int i=DW_BY_USERKEY; i < NB_SUBKEYS*DW_BY_BLOCK+DW_BY_USERKEY; i++) {
        tmpkeys[i] = tmpkeys[i-8]^tmpkeys[i-5]^tmpkeys[i-3]^tmpkeys[i-1]
            ^phi^(i-8);
        rotl(&(tmpkeys[i]), 11);
    }

    // Applying sbox for subkey i
    for (int i = 0; i < NB_SUBKEYS; i++) {
        si = (32 + 3 - i) % 8;

        // Iterates over all nibbles of the subkey i
        for (int j = 0; j < NIBBLES_BY_SUBKEY; j++) {
            in = get_bit(j, tmpkeys[0+DW_BY_BLOCK*i+DW_BY_USERKEY])
                | get_bit(j, tmpkeys[1+DW_BY_BLOCK*i+DW_BY_USERKEY]) << 1
                | get_bit(j, tmpkeys[2+DW_BY_BLOCK*i+DW_BY_USERKEY]) << 2
                | get_bit(j, tmpkeys[3+DW_BY_BLOCK*i+DW_BY_USERKEY]) << 3;
            out = apply_sbox(si, in);
            for (int l = 0; l < DW_BY_BLOCK; l++) {
                subkeys[l+DW_BY_BLOCK*i] |= get_bit(l, (uint32_t)out) << j;
            }
        }
        // TODELETE
        printf("SK[%d]=", i);
        printf("%08x", subkeys[3+i*DW_BY_BLOCK]);
        printf("%08x", subkeys[2+i*DW_BY_BLOCK]);
        printf("%08x", subkeys[1+i*DW_BY_BLOCK]);
        printf("%08x", subkeys[0+i*DW_BY_BLOCK]);
        printf("\n");
    }

    // Apply IP on every subkey
    for (int i = 0; i < NB_SUBKEYS; i++) {
        apply_IP(&(subkeys[i*DW_BY_BLOCK]), 
                &(tmpkeys[DW_BY_USERKEY + i*DW_BY_BLOCK]));
    }

    memcpy(subkeys, &(tmpkeys[DW_BY_USERKEY]), 132*sizeof(uint32_t));
}

void apply_xor(uint32_t block[DW_BY_BLOCK], uint32_t subkey[DW_BY_BLOCK]) {
    for (int i = 0; i < DW_BY_BLOCK; i++) {
        block[i] ^= subkey[i];
    }
}

void apply_permut(uint32_t block[DW_BY_BLOCK]) {
    rotl(&block[0], 13);
    rotl(&block[2], 3); 
    block[1] = block[1]^block[0]^block[2];
    block[3] = block[3]^block[2]^(block[0]<<3);
    rotl(&block[1], 1);
    rotl(&block[3], 7);
    block[0] = block[0]^block[1]^block[3];
    block[2] = block[2]^block[3]^(block[1]<<7);
    rotl(&block[0], 5);
    rotl(&block[2], 22);
}

void apply_round(int round, uint32_t block[DW_BY_BLOCK], 
        uint32_t subkeys[DW_BY_BLOCK*NB_SUBKEYS]) {

    apply_xor(block, &(subkeys[round]));

    uint32_t sbox_out = 0;
    for (int i = 0; i < NIBBLES_BY_SUBKEY; i++) {
        sbox_out |= apply_sbox(round%8,
                (block[i/32] >> (4*i%32)) & 0xf) << (4*i%32);
        if ( !(4*(i+1)%32) ) {
            block[i/32] = sbox_out;
            sbox_out = 0;
        }
    }

    if (round == NB_ROUNDS - 1) {
        apply_xor(block, &(subkeys[round+1]));
    } else {
        apply_permut(block);
    }
}

void serpent_encrypt(struct serpent_state *st, uint32_t in[DW_BY_BLOCK], 
    uint32_t out[DW_BY_BLOCK]) {
    
    uint32_t subkeys[DW_BY_BLOCK*NB_SUBKEYS];
    uint32_t tmp_block[DW_BY_BLOCK];
    serpent_keyschedule(*st, subkeys);

    apply_IP(in, tmp_block);
    for (int i = 0; i < NB_ROUNDS; i++) {
        apply_round(i, tmp_block, subkeys);
    }
    apply_FP(tmp_block, out);
}
