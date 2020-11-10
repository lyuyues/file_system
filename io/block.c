#include "block.h"

/*0: occupied  1: assignable */
void init_bitmap(Bitmap *bitmap) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        bitmap->bitmap[i] = 255;
    }
}

bool test_bitmap_pos(Bitmap *bitmap, uint16_t pos) {
    uint16_t i = pos / 8;
    uint8_t j = pos % 8;
    return (bitmap->bitmap[i] & (128u >> j)) != (128u >> j);
}

void set_bitmap_pos(Bitmap *bitmap, uint16_t inode_id) {
    int i = inode_id / 8;
    uint8_t j = inode_id % 8;
    bitmap->bitmap[i] = bitmap->bitmap[i] & (~(128u >> j));
}

void clear_bitmap_pos(Bitmap *bitmap, uint16_t inode_id) {
    int i = inode_id / 8;
    uint8_t j = inode_id % 8;
    bitmap->bitmap[i] = bitmap->bitmap[i] | (128u >> j);
}

Bitmap *block_bitmap;

/* Point the buffer to the bitmap of block */
void load_block(void *buffer) {
    block_bitmap = buffer;
}

/* Initial the bitmap of block, where the first 16 blocks is reserved, and the rest is unoccupied */
void init_block() {
    block_bitmap = malloc(BLOCK_SIZE);
    init_bitmap(block_bitmap);
    block_bitmap->bitmap[0] = block_bitmap->bitmap[1] = 0; // 0:occupied
}

/* Check whether the block is occupied or not base on bitmap */
bool test_block_id(uint16_t block_id) {
    return test_bitmap_pos(block_bitmap, block_id);
}

/* Update the bitmap of block to occupied */
void set_block_id(uint16_t block_id) {
    set_bitmap_pos(block_bitmap, block_id);
}

/* Update the bitmap of block to unoccupied */
void clear_block_id(uint16_t block_id) {
    clear_bitmap_pos(block_bitmap, block_id);
}

/* Write back the updated bitmap of block to vdisk */
void write_block_bitmap_to_file(FILE *file) {
    fseek(file, BLOCK_SIZE, SEEK_SET);
    fwrite(block_bitmap, BLOCK_SIZE, 1, file);
    fflush(file);
}

/* Allocate memory as block, initial to all 0 */
void *create_new_block() {
    void *block = malloc(BLOCK_SIZE);
    memset(block, 0, BLOCK_SIZE);
    return block;
}

/* Get the content of the specific block from vdisk */
void get_block_buffer(uint16_t block_id, void *buffer, FILE *file) {
    fseek(file, BLOCK_SIZE * block_id, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, file);
}
