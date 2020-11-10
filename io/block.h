#ifndef BLOCK_H
#define BLOCK_H

#include "struct.h"

void load_block(void *buffer);

void init_block();

bool test_block_id(uint16_t block_id);

void set_block_id(uint16_t block_id);

void clear_block_id(uint16_t block_id);

void write_block_bitmap_to_file(FILE *file);

void *create_new_block();

void get_block_buffer(uint16_t block_id, void *buffer, FILE *file);

#endif //BLOCK_H
