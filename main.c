#include "serpent.h"

int main() {
    struct serpent_state st;
    uint32_t *key = calloc(DW_BY_USERKEY, sizeof(uint32_t));
    int key_size_dw = 128;
    for (int i = 0; i < 4; i++) {
        key[i] = -1;
    }
    st.key = key;
    st.key_size_dw = key_size_dw;
    
    uint32_t subkeys[NB_SUBKEYS*DW_BY_BLOCK] = {0};
    serpent_keyschedule(st, subkeys);
    return 0;
}
