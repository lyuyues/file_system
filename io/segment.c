#include "segment.h"

#include "block.h"
#include "inode.h"

#define BLOCK_PER_SEGMENT   16
#define SEGMENT_SIZE        (BLOCK_SIZE * BLOCK_PER_SEGMENT)
#define SEGMENT_NUM         255  // the #segments on disk
#define INVALID_SEGMENT     255

void *segment_buffer;
uint8_t current_segment_id; // store next assignable segment on disk
uint8_t inner_block_id;  // store next assignable block in segment

bool segment_map[SEGMENT_NUM]; // bitmap of segment, True: occupied; False: avaible to write;

/* Get the information of each segment of on disk is occupied or empty. store the info in array segment_map of size 255 */
void load_segment() {
    segment_buffer = malloc(BLOCK_SIZE * BLOCK_PER_SEGMENT);

    current_segment_id = INVALID_SEGMENT;
    inner_block_id = 0;
    // check which segment on disk is available to write next
    // if all the block in a segment is with no valid data, then the segment is considered as empty, which is valid to write new data
    for (int i = 0; i < SEGMENT_NUM; i++) {
        for (int j = 0; j < BLOCK_PER_SEGMENT; j++) {
            uint16_t block_id = (i + 1) * BLOCK_PER_SEGMENT + j;
            if (test_block_id(block_id)) { // check if the block is occupied
                segment_map[i] = true;
                goto continue_outer_loop; // go to check next segment
            }
        }
        segment_map[i] = false;
        if (current_segment_id == INVALID_SEGMENT)
            current_segment_id = i;
        continue_outer_loop:
        ;
    }
}

void init_segment() {
    segment_buffer = malloc(BLOCK_SIZE * BLOCK_PER_SEGMENT);

    current_segment_id = 0;
    inner_block_id = 0;

    for (int i = 0; i < SEGMENT_NUM; i++)
        segment_map[i] = false;
}

uint8_t get_next_available_segment() {
    for (int i = 0; i < SEGMENT_NUM; i++) {
        if (!segment_map[i])
            return i;
    }

    // TODO
    return INVALID_SEGMENT;
}

/* Check if the segment is valid to write new data in*/
bool test_segment_id(uint8_t segment_id) {
    return segment_map[segment_id];
}

/* Update segment status to be occupied */
void set_segment_id(uint8_t segment_id) {
    segment_map[segment_id] = true;
}

/* Update segment status to be unoccupied  */
void clear_segment_id(uint8_t segment_id) {
    segment_map[segment_id] = false;
}

/* Get the next assignable block, return the number of the block */
uint16_t get_current_block_id() {
    return (current_segment_id + 1) * BLOCK_PER_SEGMENT + inner_block_id;
}

uint16_t write_block_to_segment(void *block_buffer, FILE *file) {
    uint16_t current_block_id = get_current_block_id();

    memcpy(segment_buffer + inner_block_id * BLOCK_SIZE, block_buffer, BLOCK_SIZE);
    inner_block_id++;

    if (inner_block_id == BLOCK_PER_SEGMENT)
        write_segment_to_file(file);

    return current_block_id;
}

/* Help function called by write_segment_to_file() */
void write_segment_to_file_(FILE *file) {
    fseek(file, (current_segment_id + 1) * SEGMENT_SIZE, SEEK_SET);
    fwrite(segment_buffer, SEGMENT_SIZE, 1, file);
    fflush(file);
}

/* Write whole segment and bitmap of block and inode map to vdisk */
void write_segment_to_file(FILE *file) {
    if (0 == inner_block_id)
        return;

    set_segment_id(current_segment_id);

    // TODO CHECK SEQUENCE
    write_segment_to_file_(file);
    write_block_bitmap_to_file(file);
    write_inode_map_to_file(file);

    current_segment_id = get_next_available_segment();
    inner_block_id = 0;
}
