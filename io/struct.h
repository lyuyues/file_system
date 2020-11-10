#ifndef STRUCT_H
#define STRUCT_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE          512
#define NUM_BLOCK           4096
#define NUM_INODE           256
#define NUM_ENTRY           16

#define FILE_DEPTH_LEVEL    4
#define FILENAME_SIZE       31

#define INVALID_INODE_ID    0
#define INVALID_BLOCK_ID    0

struct LFile {
    uint8_t inode_id;
};

typedef struct SuperBlock {
    uint32_t magic_number;
    uint32_t num_block;
    uint32_t num_inode;
} SuperBlock;

typedef struct INode {
    uint32_t file_size;
    uint32_t flag;
    uint16_t direct_block[10];
    uint16_t single_block;
    uint16_t double_block;
} INode;

typedef struct Bitmap {
    uint8_t bitmap[NUM_BLOCK / sizeof(uint8_t)];
} Bitmap;

typedef struct Entry {
    uint8_t inode_id;
    char filename[FILENAME_SIZE];
} Entry;

typedef struct Directory {
    struct Entry entries[NUM_ENTRY];
} Directory;

typedef struct INodeMap {
    uint16_t imap[NUM_INODE];
} INodeMap;

#endif //STRUCT_H
