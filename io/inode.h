#ifndef INODE_H
#define INODE_H

#include "struct.h"

void load_inode_map(void *buffer);

void init_inode_map();

void write_inode_map_to_file(FILE *file);

uint8_t get_available_inode_id();

void add_inode_block_id_relation(uint8_t inode_id, uint16_t block_id);

void clear_inode_id(uint8_t inode_id);

uint16_t get_block_id(uint8_t inode_id);

#endif //INODE_H
