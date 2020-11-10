#ifndef SEGMENT_H
#define SEGMENT_H

#include "struct.h"

void load_segment();

void init_segment();

uint16_t get_current_block_id();

uint16_t write_block_to_segment(void *block_buffer, FILE *file);

void write_segment_to_file(FILE *file);

#endif //SEGMENT_H
