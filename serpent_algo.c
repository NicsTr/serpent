#include "serpent_algo.h"
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
/* Apply SBox to the four least significant bits */
static inline uint8_t apply_sbox_inv(int si, uint8_t x) {
    x = Sinv[si][x & 0xf];
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
    //printf("USERKEY=");
    for (int i = DW_BY_USERKEY-1; i >= 0; i--) {
        //printf("%08x", tmpkeys[i]); 
    }
    //printf("\n");



    for (int i=DW_BY_USERKEY; i < NB_SUBKEYS*DW_BY_BLOCK+DW_BY_USERKEY; i++) {
        tmpkeys[i] = tmpkeys[i-8]^tmpkeys[i-5]^tmpkeys[i-3]^tmpkeys[i-1]
            ^phi^(i-8);
        rotl(&(tmpkeys[i]), 11);
    }
    for (int i = 0; i < NB_SUBKEYS*DW_BY_BLOCK; i++) {
        //printf("%x", subkeys[i]);
    }
    //printf("\n");

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
        //printf("SK[%d]=", i);
        //printf("%08x", subkeys[3+i*DW_BY_BLOCK]);
        //printf("%08x", subkeys[2+i*DW_BY_BLOCK]);
        //printf("%08x", subkeys[1+i*DW_BY_BLOCK]);
        //printf("%08x", subkeys[0+i*DW_BY_BLOCK]);
        //printf("\n");
    }

    // Apply IP on every subkey
    for (int i = 0; i < NB_SUBKEYS; i++) {
        apply_IP(&(subkeys[i*DW_BY_BLOCK]), 
                &(tmpkeys[DW_BY_USERKEY + i*DW_BY_BLOCK]));
    }

    memcpy(subkeys, &(tmpkeys[DW_BY_USERKEY]), 132*sizeof(uint32_t));
    for (int i = 0; i < NB_SUBKEYS; i++) {
        //printf("SK^[%d]=", i);
        //printf("%08x", subkeys[3+i*DW_BY_BLOCK]);
        //printf("%08x", subkeys[2+i*DW_BY_BLOCK]);
        //printf("%08x", subkeys[1+i*DW_BY_BLOCK]);
        //printf("%08x", subkeys[0+i*DW_BY_BLOCK]);
        //printf("\n");
    }
}

void apply_xor(uint32_t block[DW_BY_BLOCK], uint32_t subkey[DW_BY_BLOCK]) {
    for (int i = 0; i < DW_BY_BLOCK; i++) {
        //printf("%08x %08x ", block[i], subkey[i]);
        block[i] ^= subkey[i];
        //printf("%08x\n", block[i]);
    }
}

void apply_permut(uint32_t block[DW_BY_BLOCK]) {
    uint32_t tmp_block[DW_BY_BLOCK] = {0};
    apply_FP(block, tmp_block);
    rotl(&tmp_block[0], 13);
    rotl(&tmp_block[2], 3); 
    tmp_block[1] ^= tmp_block[0]^tmp_block[2];
    tmp_block[3] ^= tmp_block[2]^(tmp_block[0]<<3);
    rotl(&tmp_block[1], 1);
    rotl(&tmp_block[3], 7);
    tmp_block[0] ^= tmp_block[1]^tmp_block[3];
    tmp_block[2] ^= tmp_block[3]^(tmp_block[1]<<7);
    rotl(&tmp_block[0], 5);
    rotl(&tmp_block[2], 22);
    apply_IP(tmp_block, block);
}

void apply_permut_inv(uint32_t block[DW_BY_BLOCK]) {
    uint32_t tmp_block[DW_BY_BLOCK] = {0};
    apply_FP(block, tmp_block);
    rotr(&tmp_block[0], 5);
    rotr(&tmp_block[2], 22);
    tmp_block[2] ^= tmp_block[3]^(tmp_block[1]<<7);
    tmp_block[0] ^= tmp_block[1]^tmp_block[3];
    rotr(&tmp_block[3], 7);
    rotr(&tmp_block[1], 1);
    tmp_block[3] ^= tmp_block[2]^(tmp_block[0]<<3);
    tmp_block[1] ^= tmp_block[0]^tmp_block[2];
    rotr(&tmp_block[2], 3); 
    rotr(&tmp_block[0], 13);
    apply_IP(tmp_block, block);
}

void apply_round(int round, uint32_t block[DW_BY_BLOCK], 
        uint32_t subkeys[DW_BY_BLOCK*NB_SUBKEYS]) {


    //printf("P[%d]=", round);
    //printf("%08x", block[3]);
    //printf("%08x", block[2]);
    //printf("%08x", block[1]);
    //printf("%08x\n", block[0]);
    apply_xor(block, &(subkeys[4*round]));
    //printf("X[%d]=", round);
    //printf("%08x", block[3]);
    //printf("%08x", block[2]);
    //printf("%08x", block[1]);
    //printf("%08x\n", block[0]);

    uint32_t res;
    for (int i = 0; i < DW_BY_BLOCK; i++) {
        res = 0; 
        for (int j = 0; j < 8; j++) {
            res |= apply_sbox(round%8, (block[i] >> 4*j) & 0xf) << 4*j;
        }
        block[i] = res;
    }
    //printf("S[%d]=", round);
    //printf("%08x", block[3]);
    //printf("%08x", block[2]);
    //printf("%08x", block[1]);
    //printf("%08x\n", block[0]);

    if (round == NB_ROUNDS - 1) {
        apply_xor(block, &(subkeys[4*(round+1)]));
    } else {
        apply_permut(block);
    }
    //printf("P[%d]=", round);
    //printf("%08x", block[3]);
    //printf("%08x", block[2]);
    //printf("%08x", block[1]);
    //printf("%08x\n", block[0]);

}

void apply_round_inv(int round, uint32_t block[DW_BY_BLOCK], 
        uint32_t subkeys[DW_BY_BLOCK*NB_SUBKEYS]) {

    //printf("C[%d]=", round);
    //printf("%08x", block[3]);
    //printf("%08x", block[2]);
    //printf("%08x", block[1]);
    //printf("%08x\n", block[0]);

    if (round == NB_ROUNDS - 1) {
        apply_xor(block, &(subkeys[4*(round+1)]));
    } else {
        apply_permut_inv(block);
    }

    //printf("Pinv[%d]=", round);
    //printf("%08x", block[3]);
    //printf("%08x", block[2]);
    //printf("%08x", block[1]);
    //printf("%08x\n", block[0]);

    uint32_t res;
    for (int i = 0; i < DW_BY_BLOCK; i++) {
        res = 0; 
        for (int j = 0; j < 8; j++) {
            res |= apply_sbox_inv(round%8, (block[i] >> 4*j) & 0xf) << 4*j;
        }
        block[i] = res;
    }
    //printf("Sinv[%d]=", round);
    //printf("%08x", block[3]);
    //printf("%08x", block[2]);
    //printf("%08x", block[1]);
    //printf("%08x\n", block[0]);
    apply_xor(block, &(subkeys[4*round]));
    //printf("X[%d]=", round);
    //printf("%08x", block[3]);
    //printf("%08x", block[2]);
    //printf("%08x", block[1]);
    //printf("%08x\n", block[0]);



}

void serpent_encrypt(struct serpent_state *st, uint32_t in[DW_BY_BLOCK], 
        uint32_t out[DW_BY_BLOCK]) {

    uint32_t subkeys[DW_BY_BLOCK*NB_SUBKEYS] = {0};
    uint32_t tmp_block[DW_BY_BLOCK] = {0};

    serpent_keyschedule(*st, subkeys);

    apply_IP(in, tmp_block);
    for (int i = 0; i < NB_ROUNDS; i++) {
        apply_round(i, tmp_block, subkeys);
    }
    apply_FP(tmp_block, out);
}



void serpent_decrypt(struct serpent_state *st, uint32_t in[DW_BY_BLOCK],
        uint32_t out[DW_BY_BLOCK]) {
    
    uint32_t subkeys[DW_BY_BLOCK*NB_SUBKEYS] = {0};
    uint32_t tmp_block[DW_BY_BLOCK] = {0};


    serpent_keyschedule(*st, subkeys);

    //printf("Cipher=");
    //printf("%08x", in[3]);
    //printf("%08x", in[2]);
    //printf("%08x", in[1]);
    //printf("%08x\n", in[0]);

    apply_IP(in, tmp_block);
    for (int i = NB_ROUNDS - 1; i >= 0; i--) {
        apply_round_inv(i, tmp_block, subkeys);
    }
    apply_FP(tmp_block, out);
}
