#include "typedefs.h"

size_t max_token_len(size_t num_fields, char32_t **tokens)
{
    size_t max_len = 0;
    size_t last_len;
    char32_t *last_token;

    for (size_t i = 0; i < num_fields - 1; ++i) {
        // This relies on knowing that the pointers in tokens point
        // to consecutive words in a contiguous buffer.
        size_t token_len = tokens[i+1] - tokens[i] - 1;
        if (token_len > max_len) {
            max_len = token_len;
        }
    }
    last_token = tokens[num_fields - 1];
    last_len = 0;
    while (*last_token) {
        ++last_token;
        ++last_len;
    }
    if (last_len > max_len) {
        max_len = last_len;
    }
    return max_len;
}
