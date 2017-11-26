#include "serpent_algo.h"

int main() {
    struct serpent_state st;
    uint32_t *key = calloc(DW_BY_USERKEY, sizeof(uint32_t));
    uint32_t pt[DW_BY_BLOCK] = {0};
    uint32_t pt2[DW_BY_BLOCK] = {0};
    uint32_t ct[DW_BY_BLOCK] = {0};
    int key_size_dw = 128;
    
    for (int i = 0; i < 4; i++) {
        key[i] = 0;
    }
    pt[3] = 0x8ED77392;
    pt[2] = 0xF29990ED;
    pt[1] = 0xA7A3A3CE;
    pt[0] = 0x6F579DD2;
    st.key = key;
    st.key_size_dw = key_size_dw;
    
    
    serpent_encrypt(&st, pt, ct);
    
    printf("Cipher=");
    printf("%08x", ct[3]);
    printf("%08x", ct[2]);
    printf("%08x", ct[1]);
    printf("%08x\n", ct[0]);

    serpent_decrypt(&st, ct, pt2);
    
    printf("Plain=");
    printf("%08x", pt2[3]);
    printf("%08x", pt2[2]);
    printf("%08x", pt2[1]);
    printf("%08x\n", pt2[0]);

    free(key);
    return 0;
}
