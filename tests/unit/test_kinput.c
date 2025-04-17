#include "test.h"

#define NUM_KINPUTS 256
#define OFFSETOF(PTR, ELEMENT) ((ptrdiff_t)&(((PTR))->ELEMENT) - (ptrdiff_t)(PTR))

char *str = "";

void test(buffer_t *buf, mark_t *cur) {
    kinput_t weird_input[NUM_KINPUTS];
    kinput_t input;
    int result;
    size_t i, j;
    size_t offsetof_mod, offsetof_ch, offsetof_key;
    kinput_t *input_ptr = &input;

    offsetof_mod = OFFSETOF(input_ptr, mod);
    offsetof_ch  = OFFSETOF(input_ptr, ch);
    offsetof_key = OFFSETOF(input_ptr, key);

    // Fill weird_input values
    for (i = 0; i < sizeof(kinput_t) * NUM_KINPUTS; i++) {
        *(((uint8_t*)weird_input) + i) = (uint8_t)i;
    }

    // Ensure MLE_KINPUT_COPY *preserves* padding bytes
    result = 0;
    for (i = 0; i < NUM_KINPUTS; i++) {
        MLE_KINPUT_COPY(input, weird_input[i]);
        result |= memcmp(&input, &weird_input[i], sizeof(kinput_t));
    }
    ASSERT("cmp_kinput_assign", 0, result);

    // Ensure MLE_KINPUT_SET *zeroes* padding bytes
    result = 0;
    for (i = 0; i < NUM_KINPUTS; i++) {
        // Fill input with non-sense; 42 is a good choice
        memset(&input, 42, sizeof(input));

        // Set input to weird_input[i]
        MLE_KINPUT_SET(input, weird_input[i].mod, weird_input[i].ch, weird_input[i].key);

        // Ensure all fields are equal
        result |= memcmp(&input.mod, &weird_input[i].mod, sizeof(input.mod));
        result |= memcmp(&input.ch,  &weird_input[i].ch,  sizeof(input.ch));
        result |= memcmp(&input.key, &weird_input[i].key, sizeof(input.key));

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
