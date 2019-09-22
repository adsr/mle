#include "test.h"

#define NUM_KINPUTS 256
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

char *str = "";

void test(buffer_t *buf, mark_t *cur) {
    kinput_t garbage[NUM_KINPUTS];
    kinput_t input;
    int result;
    size_t i, j;
    size_t offsetof_mod, offsetof_ch, offsetof_key;

    offsetof_mod = OFFSETOF(kinput_t, mod);
    offsetof_ch  = OFFSETOF(kinput_t, ch);
    offsetof_key = OFFSETOF(kinput_t, key);

    // Fill garbage inputs
    for (i = 0; i < sizeof(kinput_t) * NUM_KINPUTS; i++) {
        *(((uint8_t*)garbage) + i) = (uint8_t)i;
    }

    // Ensure MLE_KINPUT_COPY preserves padding bytes
    result = 0;
    for (i = 0; i < NUM_KINPUTS; i++) {
        MLE_KINPUT_COPY(input, garbage[i]);
        result |= memcmp(&input, &garbage[i], sizeof(kinput_t));
    }
    ASSERT("cmp_kinput_assign", 0, result);

    // Ensure MLE_KINPUT_SET zeroes padding bytes
    result = 0;
    for (i = 0; i < NUM_KINPUTS; i++) {
        MLE_KINPUT_SET(input, garbage[i].mod, garbage[i].ch, garbage[i].key);

        // Ensure fields are equal
        result |= memcmp(&input.mod, &garbage[i].mod, sizeof(input.mod));
        result |= memcmp(&input.ch,  &garbage[i].ch,  sizeof(input.ch));
        result |= memcmp(&input.key, &garbage[i].key, sizeof(input.key));

        // Ensure bytes between mod and ch are zero
        for (j = offsetof_mod + sizeof(input.mod); j < offsetof_ch; j++) {
            result |= *(((uint8_t*)&input) + j) == 0x00 ? 0 : 1;
        }

        // Ensure bytes between ch and key are zero
        for (j = offsetof_ch + sizeof(input.ch); j < offsetof_key; j++) {
            result |= *(((uint8_t*)&input) + j) == 0x00 ? 0 : 1;
        }

        // Ensure bytes between key and end are zero
        for (j = offsetof_key + sizeof(input.key); j < sizeof(kinput_t); j++) {
            result |= *(((uint8_t*)&input) + j) == 0x00 ? 0 : 1;
        }
    }
    ASSERT("cmp_kinput_set", 0, result);
}
