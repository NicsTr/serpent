#ifndef __serpent_h_
#define __serpent_h_
#include "serpent.h"
#endif

int main() {
    struct serpent_state st;
    uint32_t *key = calloc(DW_BY_USERKEY, sizeof(uint32_t));
    int key_size_dw = DW_BY_USERKEY;
    st.key = key;
    st.key_size_dw = key_size_dw;
    
    uint32_t subkeys[NB_SUBKEYS*DW_BY_BLOCK];
    serpent_keyschedule(st, subkeys);
    return 0;
}
