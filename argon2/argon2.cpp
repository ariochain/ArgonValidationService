//
// Created by Haifa Bogdan Adnan on 05/08/2018.
//

#include "../common.h"
#include "base64.h"

#include "blake2/blake2.h"
#include "argon2.h"
#include "defs.h"

extern "C" {
    void *fill_memory_blocks(void *memory, argon2profile *profile);
}

argon2::argon2(void *seed_memory) {
    __output_memory = __seed_memory = (uint8_t*)seed_memory;
    __lane_length = -1;
}

string argon2::generate_hash(const argon2profile &profile, const string &base, string salt) {
    string hash;

    uint8_t blockhash[ARGON2_PREHASH_SEED_LENGTH];
    uint8_t raw_hash[ARGON2_RAW_LENGTH];

    __initial_hash(profile, blockhash, base, salt);

    memset(blockhash + ARGON2_PREHASH_DIGEST_LENGTH, 0,
           ARGON2_PREHASH_SEED_LENGTH -
           ARGON2_PREHASH_DIGEST_LENGTH);

    __fill_first_blocks(profile, blockhash);

    __output_memory = (uint8_t *)fill_memory_blocks (__seed_memory, (argon2profile*)&profile);

    if(__output_memory != NULL) {
        blake2b_long((void *) raw_hash, ARGON2_RAW_LENGTH,
                     (void *) (__output_memory), ARGON2_BLOCK_SIZE);

        hash = __encode_string(profile, salt, raw_hash);
    }

    return hash;
}

void argon2::__initial_hash(const argon2profile &profile, uint8_t *blockhash, const string &base, const string &salt) {
    blake2b_state BlakeHash;
    uint32_t value;

    blake2b_init(&BlakeHash, ARGON2_PREHASH_DIGEST_LENGTH);

    value = profile.thr_cost;
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    value = ARGON2_RAW_LENGTH;
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    value = profile.mem_cost;
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    value = profile.tm_cost;
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    value = ARGON2_VERSION;
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    value = ARGON2_TYPE_VALUE;
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    value = (uint32_t)base.length();
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    blake2b_update(&BlakeHash, (const uint8_t *)base.c_str(),
                   base.length());

    value = (uint32_t)salt.length();
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    blake2b_update(&BlakeHash, (const uint8_t *)salt.c_str(),
                   salt.length());

    value = 0;
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));
    blake2b_update(&BlakeHash, (const uint8_t *)&value, sizeof(value));

    blake2b_final(&BlakeHash, blockhash, ARGON2_PREHASH_DIGEST_LENGTH);
}

void argon2::__fill_first_blocks(const argon2profile &profile, uint8_t *blockhash) {
    block *blocks = (block *)(__seed_memory);

    size_t lane_length;
    if(__lane_length == -1) {
        lane_length = profile.mem_cost / profile.thr_cost;
    }
    else {
        lane_length = __lane_length;
    }

    for (uint32_t l = 0; l < profile.thr_cost; ++l) {
        *((uint32_t*)(blockhash + ARGON2_PREHASH_DIGEST_LENGTH)) = 0;
        *((uint32_t*)(blockhash + ARGON2_PREHASH_DIGEST_LENGTH + 4)) = l;

        blake2b_long((void *)(blocks + l * lane_length), ARGON2_BLOCK_SIZE, blockhash,
                     ARGON2_PREHASH_SEED_LENGTH);

        *((uint32_t*)(blockhash + ARGON2_PREHASH_DIGEST_LENGTH)) = 1;

        blake2b_long((void *)(blocks + l * lane_length + 1), ARGON2_BLOCK_SIZE, blockhash,
                     ARGON2_PREHASH_SEED_LENGTH);
    }
}

string argon2::__encode_string(const argon2profile &profile, const string &salt, uint8_t *hash) {
    char salt_b64[50];
    char hash_b64[50];

    base64::encode(salt.c_str(), (int)salt.length(), salt_b64);
    base64::encode((char *)hash, ARGON2_RAW_LENGTH, hash_b64);

    salt_b64[22] = 0;
    hash_b64[43] = 0;

    stringstream ss;
    ss << "$argon2i$v=19$m=" << profile.mem_cost << ",t=" << profile.tm_cost << ",p=" << profile.thr_cost << "$" << salt_b64 << "$" << hash_b64;
    return ss.str();
}

void argon2::set_seed_memory(uint8_t *memory) {
    __seed_memory = memory;
}

uint8_t *argon2::get_output_memory() {
    return __output_memory;
}

void argon2::set_lane_length(int length) {
    if(length > 0)
        __lane_length = length;
}

