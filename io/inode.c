#include "inode.h"

#include "block.h"

INodeMap *inode_map; // #inode & #block; when #block = -1 means the #inode is unoccupied.

/* Caller have the pointer of the inode map*/
void load_inode_map(void *buffer) {
    inode_map = buffer;
}

/* Initial the inode_map, clean the memory to all 0 */
void init_inode_map() {
    inode_map = malloc(sizeof(INodeMap));
    memset(inode_map->imap, INVALID_BLOCK_ID, sizeof(INodeMap));
}

/* Write inode_map to vdisk which is Block 2*/
void write_inode_map_to_file(FILE *file) {
    fseek(file, BLOCK_SIZE * 2, SEEK_SET);
    fwrite(inode_map, sizeof(INodeMap), 1, file);
    fflush(file);
}

/* Get next assignable #inode */
uint8_t get_available_inode_id() {
    for (int i = INVALID_INODE_ID + 1; i < NUM_INODE; i++) {
        if (inode_map->imap[i] == INVALID_BLOCK_ID)
            return i;
    }
    return INVALID_INODE_ID;
}

/* Update one entry of inode_map, #inode and corresponding #block */
void add_inode_block_id_relation(uint8_t inode_id, uint16_t block_id) {
    inode_map->imap[inode_id] = block_id;
}

void clear_inode_id(uint8_t inode_id) {
    inode_map->imap[inode_id] = INVALID_BLOCK_ID;
}

/* Get the #block of corresponding #inode */
uint16_t get_block_id(uint8_t inode_id) {
    return inode_map->imap[inode_id];
}
