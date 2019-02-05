//
// Created by Haifa Bogdan Adnan on 05/08/2018.
//

#ifndef ARIOMINER_ARGON2_H
#define ARIOMINER_ARGON2_H

#include "defs.h"

class argon2 {
public:
    argon2(void *seed_memory);

    string generate_hash(const argon2profile &profile, const string &base, string salt);

    void set_seed_memory(uint8_t *memory);
    uint8_t *get_output_memory();
    void set_lane_length(int length); // in blocks
private:
    void __initial_hash(const argon2profile &profile, uint8_t *blockhash, const string &base, const string &salt);
    void __fill_first_blocks(const argon2profile &profile, uint8_t *blockhash);
    string __encode_string(const argon2profile &profile, const string &salt, uint8_t *hash);

    uint8_t *__seed_memory;
	uint8_t *__output_memory;
    int __lane_length;
};

#endif //ARIOMINER_ARGON2_H
