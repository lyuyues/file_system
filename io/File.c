#include <unistd.h>

#include "File.h"

#include "struct.h"
#include "block.h"
#include "inode.h"
#include "segment.h"

#define DISK_FILE_NAME      "disk/vdisk"

#define RESERVED_BLOCK_NUM  16
#define MAGIC_NUM           2738
#define ROOT_INODE_ID       1

FILE *file; //vdisk

int check_block(bool block_map[NUM_BLOCK], uint16_t block_id) {
    if (block_id < RESERVED_BLOCK_NUM) {
        printf("Block Id [%d] is smaller than Reserved block num [%d]\n", block_id, RESERVED_BLOCK_NUM);
        return -1;
    } else if (block_id >= NUM_BLOCK) {
        printf("Block Id [%d] is larger than or equal to the total number of blocks [%d]\n", block_id, NUM_BLOCK);
        return -1;
    } else {
        if (test_block_id(block_id)) {
            if (block_map[block_id]) {
                block_map[block_id] = false;
            } else {
                printf("Block Id [%d] should not be occupied\n", block_id);
                return -1;
            }
        } else {
            printf("Block Id [%d] should not be occupied\n", block_id);
            return -1;
        }
    }

    return 0;
}

/* check block*/
void i_check() {
    bool block_map[NUM_BLOCK];
    for (int i = RESERVED_BLOCK_NUM; i < NUM_BLOCK; i++)
        block_map[i] = test_block_id(i);

    for (int i = ROOT_INODE_ID; i < NUM_INODE; i++) {
        uint16_t block_id = get_block_id(i);
        if (INVALID_BLOCK_ID == block_id)
            continue;

        if (0 != check_block(block_map, block_id))
            return;

        INode *inode_buffer = malloc(BLOCK_SIZE);
        get_block_buffer(block_id, inode_buffer, file);

        for (int j = 0; j < 10; j++) {
            uint16_t data_block_id = inode_buffer->direct_block[j];
            if (INVALID_BLOCK_ID == data_block_id)
                break;

            if (0 != check_block(block_map, data_block_id))
                return;
        }

        free(inode_buffer);
    }

    for (int i = RESERVED_BLOCK_NUM; i < NUM_BLOCK; i++)
        if (block_map[i])
            printf("Block Id [%d] should not be occupied\n", i);
}

int check_directory(const uint8_t inode_parent_map[NUM_INODE], uint8_t inode_id) {
    uint8_t child_inode_id = inode_id;
    while (true) {
        uint8_t parent_inode_id = inode_parent_map[child_inode_id];
        if (INVALID_INODE_ID == parent_inode_id) {
            printf("Inode Id [%d] does not connect to root directory\n", inode_id);
            return -1;
        } else if (ROOT_INODE_ID == parent_inode_id) {
            return 0;
        }

        child_inode_id = parent_inode_id;
    }
}

/* directory check*/
void d_check() {
    uint8_t inode_parent_map[NUM_INODE];
    memset(inode_parent_map, INVALID_INODE_ID, NUM_INODE);

    for (int i = ROOT_INODE_ID; i < NUM_INODE; i++) {
        uint16_t block_id = get_block_id(i);
        if (INVALID_BLOCK_ID == block_id)
            continue;

        INode *inode_buffer = malloc(BLOCK_SIZE);
        get_block_buffer(block_id, inode_buffer, file);

        if (0 == inode_buffer->flag) {
            for (int j = 0; j < 10; j++) {
                uint16_t data_block_id = inode_buffer->direct_block[j];
                if (INVALID_BLOCK_ID == data_block_id)
                    break;

                Directory *directory = malloc(BLOCK_SIZE);
                get_block_buffer(data_block_id, directory, file);

                for (int k = 0; k < NUM_ENTRY; k++) {
                    uint8_t sub_dir_inode_id = directory->entries[k].inode_id;
                    if (INVALID_INODE_ID == sub_dir_inode_id)
                        break;

                    if (INVALID_INODE_ID != inode_parent_map[sub_dir_inode_id]) {
                        printf("Inode Id [%d] has a second parent Inode Id [%d], while the previous one is [%d]\n",
                                sub_dir_inode_id, i, inode_parent_map[sub_dir_inode_id]);
                    } else {
                        inode_parent_map[sub_dir_inode_id] = i;
                    }
                }

                free(directory);
            }
        }

        free(inode_buffer);
    }

    for (int i = ROOT_INODE_ID + 1; i < NUM_INODE; i++) {
        uint16_t block_id = get_block_id(i);
        if (INVALID_BLOCK_ID == block_id)
            continue;

        check_directory(inode_parent_map, i);
    }
}

/* Initial the super block*/
void init_super_block(SuperBlock *super_block) {
    super_block->magic_number = MAGIC_NUM;
    super_block->num_block = NUM_BLOCK;
    super_block->num_inode = NUM_INODE;
}

/* Create root directory, write back to disk immediately*/
void create_root_dir() {
    uint16_t block_id = get_current_block_id();

    INode *inode = create_new_block();
    inode->flag = 0;

    set_block_id(block_id);
    add_inode_block_id_relation(ROOT_INODE_ID, block_id);

    write_block_to_segment(inode, file);

    write_segment_to_file(file);
}

/* Check if file vdisk is exist, if exist then load the disk, if not creat a new disk*/
int INITLLFS() {
    void *buffer = malloc(BLOCK_SIZE * RESERVED_BLOCK_NUM);
    memset(buffer, 0, BLOCK_SIZE * RESERVED_BLOCK_NUM);

    // point all the init data head to their reserved blocks
    // Block 0: superblock; Block 1: bitmap of blocks; Blcok 2: inode map;

    SuperBlock *super_block = (SuperBlock *) buffer;
    init_super_block(super_block);

    file = fopen(DISK_FILE_NAME, "w+");
    fwrite(buffer, BLOCK_SIZE, RESERVED_BLOCK_NUM, file); // write superblock into vdisk

    init_block();  // initial bitmap of block
    write_block_bitmap_to_file(file); // write bitmap of block into vdisk

    init_inode_map();// initial bitmap of inode
    write_inode_map_to_file(file); // write bitmap of inode into vdisk

    init_segment(); // initial bitmap of segment

    create_root_dir();

    return 0;
}

/* Initial load the reserved disk.*/
int load_disk() {
    if(access(DISK_FILE_NAME, F_OK) == -1)
        return -1;

    file = fopen(DISK_FILE_NAME, "r+"); //

    void *buffer = malloc(BLOCK_SIZE * RESERVED_BLOCK_NUM);
    fread(buffer, BLOCK_SIZE, RESERVED_BLOCK_NUM, file);

    // Check if the disk is the one we want
    SuperBlock *super_block = buffer;
    if (MAGIC_NUM != super_block->magic_number
        || NUM_BLOCK != super_block->num_block
        || NUM_INODE != super_block->num_inode) {
        free(buffer);
        return -1;
    }

    // load bitmap of block occupied info
    load_block(buffer + BLOCK_SIZE);
    // load bitmap of inode occupied info
    load_inode_map(buffer + 2 * BLOCK_SIZE);
    // load bitmap of segments occupied info
    load_segment();

    i_check();
    d_check();

    return 0;
}

uint16_t write_block_buffer_to_segment(uint16_t block_id, void *block_buffer) {
    if (INVALID_BLOCK_ID != block_id)
        clear_block_id(block_id);

    block_id = get_current_block_id();
    set_block_id(block_id);

    write_block_to_segment(block_buffer, file);

    return block_id;
}

uint16_t write_inode_buffer_to_segment(uint8_t inode_id, INode *inode_buffer) {
    uint16_t old_inode_block_id = get_block_id(inode_id);
    uint16_t new_inode_block_id = get_current_block_id();

    if (INVALID_BLOCK_ID != old_inode_block_id)
        clear_block_id(old_inode_block_id);
    set_block_id(new_inode_block_id);

    add_inode_block_id_relation(inode_id, new_inode_block_id);

    write_block_to_segment(inode_buffer, file);

    return new_inode_block_id;
}

/* Break the whole string of pathname into seperate names with terminate */
char *divide_pathname(char *pathname, int *level) {
    int length = strlen(pathname) + 1;
    char *buffer = malloc(length);
    strncpy(buffer, pathname, length);

    char *path = malloc(FILENAME_SIZE * FILE_DEPTH_LEVEL);
    memset(path, 0, FILENAME_SIZE * FILE_DEPTH_LEVEL);

    char *temp = buffer;
    int i = 0;

    *(temp++) = 0;
    while (1) {
        (*level)++;

        char *p = strchr(temp, '/');
        if (p != NULL)
            *(p) = 0;

        strncpy(path + i, temp, strlen(temp));
        i += FILENAME_SIZE;

        if (p == NULL)
            break;

        temp = p + 1;
    }
    free(buffer);
    return path;
}

/* Get the #inode of the file name in argument
    @block_id : the #block of current directory
    @filename : the file waited to locate
    @*is_dir: the container to store if the file is flat or directroy
    @return: #inode of the file wait to locate, 0: not found
 */
uint16_t find_file_inode_in_block(uint16_t block_id, char *filename, bool *is_dir) {
    Directory *directory = malloc(BLOCK_SIZE);
    get_block_buffer(block_id, directory, file);
    uint8_t res = 0;
    for (int i = 0; i < NUM_ENTRY; i++) {
        uint8_t inode_id = directory->entries[i].inode_id;
        if (0 == inode_id)
            break;

        if (0 == strcmp(filename, directory->entries[i].filename)) {
            res = inode_id;

            INode *inode_buffer = malloc(BLOCK_SIZE);
            get_block_buffer(get_block_id(inode_id), inode_buffer, file);

            *is_dir = inode_buffer->flag == 0;

            break;
        }
    }
    free(directory);
    return res;
}

/* Locate corresponding #inode of the file name in argument
 @dir_inode_id: the #inode of current directory
 @filename : the file waited to locate
 @*is_dir: the container to store if the file is flat or directroy
 @return: #inode of the file wait to locate, 0: not found
 */
uint16_t locate_inode_via_filename(uint16_t dir_inode_id, char *filename, bool *is_dir) {
    INode *inode_buffer = malloc(BLOCK_SIZE);
    get_block_buffer(get_block_id(dir_inode_id), inode_buffer, file);
    uint8_t res = 0;
    for (int i = 0; i < 10; i++) { // only consider the direct blocks to make it simple
        int block_id = inode_buffer->direct_block[i];
        if (0 == block_id)
            break;

        int file_inode_id = find_file_inode_in_block(block_id, filename, is_dir);
        if (0 != file_inode_id) {
            res = file_inode_id;
            break;
        }
    }
    free(inode_buffer);
    return res;
}

bool rm_file_content(uint8_t file_inode_id) {
    uint16_t block_id = get_block_id(file_inode_id);

    INode *file_inode_buffer = malloc(BLOCK_SIZE);
    get_block_buffer(block_id, file_inode_buffer, file);

    // release the data block of the file
    for (int i = 0; i < 10; i++)
        clear_block_id(file_inode_buffer->direct_block[i]);

    free(file_inode_buffer);

    // release the block which is used to store the inode, and the inode_id as well.
    clear_block_id(block_id);
    clear_inode_id(file_inode_id);

    return true;
}

bool rm_dir_content(uint8_t dir_inode_id) {
    uint16_t block_id = get_block_id(dir_inode_id);

    // release the block which is used to store the inode, and the inode_id as well.
    clear_block_id(block_id);
    clear_inode_id(dir_inode_id);

    return true;
}

bool rm_entry_in_dir(uint8_t dir_inode_id, uint8_t file_inode_id) {
    uint16_t dir_inode_block_id = get_block_id(dir_inode_id);

    INode *dir_inode_buffer = malloc(BLOCK_SIZE);
    get_block_buffer(dir_inode_block_id, dir_inode_buffer, file);

    // get the block index and the entry index for the file which needs to be deleted
    // get the last block index and the entry index as well
    uint8_t file_block_index = INVALID_BLOCK_ID;
    uint8_t file_entry_index = NUM_ENTRY;
    uint8_t last_block_index = INVALID_BLOCK_ID;
    uint8_t last_entry_index = NUM_ENTRY;
    for (int i = 0; i < 10; i++) {
        int dir_data_block_id = dir_inode_buffer->direct_block[i];
        if (INVALID_BLOCK_ID == dir_data_block_id)
            break;

        last_block_index = i;

        Directory *directory = malloc(BLOCK_SIZE);
        get_block_buffer(dir_data_block_id, directory, file);

        for (int j = 0; j < NUM_ENTRY; j++) {
            if (INVALID_INODE_ID == directory->entries[j].inode_id)
                break;

            last_entry_index = j;

            if (file_inode_id == directory->entries[j].inode_id) {
                file_block_index = i;
                file_entry_index = j;
            }
        }

        free(directory);
    }

    //
    // delete the entry in the directory
    //
    uint16_t file_block_id = dir_inode_buffer->direct_block[file_block_index];
    uint16_t last_block_id = dir_inode_buffer->direct_block[last_block_index];

    // load the directory
    Directory *file_directory = malloc(BLOCK_SIZE);
    get_block_buffer(file_block_id, file_directory, file);
    Directory *last_directory;
    if (file_block_index == last_block_index) {
        last_directory = file_directory;
    } else {
        last_directory = malloc(BLOCK_SIZE);
        get_block_buffer(last_block_id, last_directory, file);
    }

    // copy the last entry to the file entry and remove the last entry
    if (file_block_index != last_block_index || file_entry_index != last_entry_index) {
        file_directory->entries[file_entry_index].inode_id = last_directory->entries[last_entry_index].inode_id;
        memcpy(file_directory->entries[file_entry_index].filename, last_directory->entries[last_entry_index].filename, FILENAME_SIZE);
    }
    last_directory->entries[last_entry_index].inode_id = INVALID_INODE_ID;
    memset(last_directory->entries[last_entry_index].filename, 0, FILENAME_SIZE);

    // write the data buffer to the disk
    if (last_entry_index == 0) {
        // if last entry index equals to 0, the last directory needs to be deleted as well.
        clear_block_id(last_block_id);
        dir_inode_buffer->direct_block[last_block_index] = INVALID_BLOCK_ID;

        if (last_block_index != 0)
            dir_inode_buffer->direct_block[file_block_index] = write_block_buffer_to_segment(file_block_id,
                                                                                             file_directory);
    } else {
        dir_inode_buffer->direct_block[file_block_index] = write_block_buffer_to_segment(file_block_id, file_directory);
        if (file_block_index != last_block_index)
            dir_inode_buffer->direct_block[last_block_index] = write_block_buffer_to_segment(last_block_id,
                                                                                             last_directory);
    }

    // write the inode buffer to the disk
    write_inode_buffer_to_segment(dir_inode_id, dir_inode_buffer);

    free(file_directory);
    if (file_directory != last_directory)
        free(last_directory);

    free(dir_inode_buffer);

    return true;
}

bool rm_file_in_dir(uint8_t parent_dir_inode_id, uint8_t file_inode_id) {
    rm_entry_in_dir(parent_dir_inode_id, file_inode_id);
    write_segment_to_file(file);

    rm_file_content(file_inode_id);
    write_inode_map_to_file(file);
    write_block_bitmap_to_file(file);

    return true;
}

bool rm_dir_in_dir(uint8_t parent_dir_inode_id, uint8_t dir_inode_id) {
    int16_t dir_data_block_id = get_block_id(dir_inode_id);

    INode *dir_inode_buffer = malloc(BLOCK_SIZE);
    get_block_buffer(dir_data_block_id, dir_inode_buffer, file);

    bool is_dir_empty = dir_inode_buffer->direct_block[0] == INVALID_BLOCK_ID;
    free(dir_inode_buffer);

    if (!is_dir_empty)
        return false;

    rm_entry_in_dir(parent_dir_inode_id, dir_inode_id);
    write_segment_to_file(file);

    rm_dir_content(dir_inode_id);
    write_inode_map_to_file(file);
    write_block_bitmap_to_file(file);

    return true;
}

/* New an inode, update the inode_map and write it to segment */
uint8_t create_file_inode(uint32_t flag) {
    uint8_t inode_id = get_available_inode_id();

    INode *inode_buffer = create_new_block();
    inode_buffer->flag = flag;

    write_inode_buffer_to_segment(inode_id, inode_buffer);

    return inode_id;
}

/* Add the file to directory entry, help function called by add_file()
 @Directory *directory: pointer to one block of directory
 @file_inode_id: #inode wait to insert entry
 @filename: filename wait to insert entry
 @return: whether insert succesufully
 */
bool add_file_to_dir(Directory *directory, uint8_t file_inode_id, char *filename) {
    for (int i = 0; i < NUM_ENTRY; i++) {
        uint8_t inode_id = directory->entries[i].inode_id;
        if (0 != inode_id)
            continue;

        directory->entries[i].inode_id = file_inode_id;
        strcpy(directory->entries[i].filename, filename);
        return true;
    }
    return false;
}

/* Add the file to directory entry
 @dir_inode_id： the #inode of the dirctory which the file should insert to
 @file_inode_id：#inode wait to insert entry
 @filename：filename wait to insert entry
 */
void add_file(uint8_t dir_inode_id, uint8_t file_inode_id, char *filename) {
    uint16_t old_inode_block_id = get_block_id(dir_inode_id);

    INode *inode_buffer = malloc(BLOCK_SIZE);
    get_block_buffer(old_inode_block_id, inode_buffer, file);

    Directory *directory = NULL;
    for (int i = 0; i < 10; i++) {
        int dir_data_block_id = inode_buffer->direct_block[i];
        if (INVALID_INODE_ID == dir_data_block_id) {
            directory = create_new_block();
        } else {
            directory = malloc(BLOCK_SIZE);
            get_block_buffer(dir_data_block_id, directory, file);
        }

        if (add_file_to_dir(directory, file_inode_id, filename)) {
            // update data block
            dir_data_block_id = write_block_buffer_to_segment(dir_data_block_id, directory);

            // update inode block
            inode_buffer->direct_block[i] = dir_data_block_id;
            write_inode_buffer_to_segment(dir_inode_id, inode_buffer);

            free(directory);
            break;
        }

        free(directory);
    }
    free(inode_buffer);
}

/* Create file
 @dir_inode_id: #inode of directrory
 @filename: the name of file need to be created
 @flag: 0:dir  1:flat file
 return #inode of the new file
 */
uint16_t create_file(uint8_t dir_inode_id, char *filename, uint32_t flag) {
    uint8_t file_inode_id = create_file_inode(flag);

    add_file(dir_inode_id, file_inode_id, filename);

    return file_inode_id;
}

int mkdir(char *pathname) {
    int level = 0;
    char *path = divide_pathname(pathname, &level);

    uint8_t curr_inode_id = ROOT_INODE_ID;
    char filename[FILENAME_SIZE];
    bool is_dir = true;
    int res = 0;
    for (int i = 0; i < level; i++) {
        strncpy(filename, path + i * FILENAME_SIZE, FILENAME_SIZE);
        uint8_t temp_inode_id = locate_inode_via_filename(curr_inode_id, filename, &is_dir);
        if (temp_inode_id == 0) {
            if (level - 1 == i) {
                create_file(curr_inode_id, filename, 0);
                write_segment_to_file(file);
            } else {
                res = -1;
                printf("Opps, directory [%s] does not exist.\n", filename);
            }

            break;
        } else {
            if (!is_dir) {
                res = -1;
                printf("Opps, [%s] should be a directory.\n", filename);
                break;
            } else if (level - 1 == i) {
                res = -2;
                printf("Opps, the directory [%s] already exist\n", pathname);
                break;
            }

            curr_inode_id = temp_inode_id;
        }
    }

    free(path);

    return res;
}

bool rm_dir(char *pathname) {
    int level = 0;
    char *path = divide_pathname(pathname, &level);

    uint8_t curr_inode_id = ROOT_INODE_ID;
    char filename[FILENAME_SIZE];
    bool success = true, is_dir = true;
    for (int i = 0; i < level; i++) {
        strncpy(filename, path + i * FILENAME_SIZE, FILENAME_SIZE);
        uint8_t temp_inode_id = locate_inode_via_filename(curr_inode_id, filename, &is_dir);
        if (temp_inode_id == 0) {
            printf("Opps, directory [%s] does not exist.\n", filename);
            success = false;
            break;
        } else {
            if (!is_dir) {
                printf("Opps, [%s] should be a directory.\n", filename);
                success = false;
                break;
            } else if (level - 1 == i) {
                success = rm_dir_in_dir(curr_inode_id, temp_inode_id);
                break;
            } else {
                curr_inode_id = temp_inode_id;
            }
        }
    }

    free(path);

    return success;
}

/* User API
 @ pathname: full path of file to be opened
 @ model: open model, write or read
 @ flag: dir or flat file
 @ return: the lfile pointer
 */
LFile *open_file(char *pathname, char *model) {
    if (0 != strcmp(model, "w") && 0 != strcmp(model, "r")){
        return NULL;
    }
    int level = 0;
    char *path = divide_pathname(pathname, &level);

    uint8_t curr_inode_id = ROOT_INODE_ID;
    char filename[FILENAME_SIZE];
    bool found_file = true, is_dir = true;
    for (int i = 0; i < level; i++) {
        strncpy(filename, path + i * FILENAME_SIZE, FILENAME_SIZE);
        uint8_t temp_inode_id = locate_inode_via_filename(curr_inode_id, filename, &is_dir);
        if (temp_inode_id == 0) {
            if (strcmp(model, "w") == 0){
                if (level - 1 == i) {
                    curr_inode_id = create_file(curr_inode_id, filename, 1);
                } else {
                    curr_inode_id = create_file(curr_inode_id, filename, 0);
                }

                write_segment_to_file(file);
            } else {
                printf("Opps, no such file.\n");
                found_file = false;
                break;
            }
        } else {
            if (i != level - 1 && !is_dir) {
                printf("Opps, %s should be a directory.\n", filename);
                found_file = false;
                break;
            }
            curr_inode_id = temp_inode_id;
        }
    }

    free(path);

    if (!found_file){
        return NULL;
    }

    LFile *lfile = malloc(sizeof(LFile));
    lfile->inode_id = curr_inode_id;
    return lfile;
}

bool rm_file(char *pathname) {
    int level = 0;
    char *path = divide_pathname(pathname, &level);

    uint8_t curr_inode_id = ROOT_INODE_ID;
    char filename[FILENAME_SIZE];
    bool success = true, is_dir = true;
    for (int i = 0; i < level; i++) {
        strncpy(filename, path + i * FILENAME_SIZE, FILENAME_SIZE);
        uint8_t temp_inode_id = locate_inode_via_filename(curr_inode_id, filename, &is_dir);
        if (temp_inode_id == 0) {
            if (is_dir)
                printf("Opps, directory [%s] does not exist.\n", filename);
            else
                printf("Opps, file [%s] does not exist.\n", filename);
            success = false;
            break;
        } else {
            if (level - 1 == i) {
                if (is_dir) {
                    printf("Opps, [%s] should be a file.\n", filename);
                    success = false;
                } else {
                    success = rm_file_in_dir(curr_inode_id, temp_inode_id);
                }
            } else {
                if (!is_dir) {
                    printf("Opps, [%s] should be a directory.\n", filename);
                    success = false;
                    break;
                }
            }

            curr_inode_id = temp_inode_id;
        }
    }

    free(path);

    return success;
}

/* User API read the data from vdisk
 @ ptr: the pointer of data
 @ size: size of data
 @ nmemb: number of data
 @ lfile: lfile pointer
 */
size_t read_file(void *ptr, size_t size, size_t nmemb, LFile *lfile) {
    uint8_t inode_id = lfile->inode_id;

    INode *inode_buffer = malloc(BLOCK_SIZE);
    get_block_buffer(get_block_id(inode_id), inode_buffer, file);

    size_t total_length = size * nmemb;
    size_t file_length = inode_buffer->file_size;

    size_t remaining_length = total_length > file_length ? file_length : total_length;
    size_t offset = 0;

    void *buffer = malloc(BLOCK_SIZE);
    int i = 0;
    while (remaining_length > 0) {
        int length = remaining_length > BLOCK_SIZE ? BLOCK_SIZE : remaining_length;

        if (i < 10)
            get_block_buffer(inode_buffer->direct_block[i], buffer, file);

        memcpy(ptr + offset, buffer, length);

        if (remaining_length <= BLOCK_SIZE)
            break;

        remaining_length -= BLOCK_SIZE;
        offset += BLOCK_SIZE;
        i++;
    }
    free(buffer);

    size_t res = (inode_buffer->file_size - size * nmemb > 0 ? size * nmemb : inode_buffer->file_size);

    free(inode_buffer);

    return res;
}

/* User API write the data to vdisk
 @ ptr: the pointer of data
 @ size: size of data
 @ nmemb: number of data
 @ lfile: lfile pointer
 */
void write_file(void *ptr, size_t size, size_t nmemb, LFile *lfile) {
    uint8_t inode_id = lfile->inode_id;
    uint16_t old_inode_block_id = get_block_id(inode_id);

    INode *inode_buffer = malloc(BLOCK_SIZE);
    get_block_buffer(old_inode_block_id, inode_buffer, file);

    size_t total_length = size * nmemb;

    size_t remaining_length = total_length;
    size_t offset = 0;

    // write the file content
    void *buffer = malloc(BLOCK_SIZE);
    int i = 0;
    while (true) {
        int length = remaining_length > BLOCK_SIZE ? BLOCK_SIZE : remaining_length;

        memcpy(buffer, ptr + offset, length);

        inode_buffer->direct_block[i] = write_block_buffer_to_segment(INVALID_BLOCK_ID, buffer);

        if (remaining_length <= BLOCK_SIZE)
            break;

        remaining_length -= BLOCK_SIZE;
        offset += BLOCK_SIZE;
        i++;
    }
    free(buffer);

    // write the file inode
    inode_buffer->file_size = total_length;
    write_inode_buffer_to_segment(inode_id, inode_buffer);

    free(inode_buffer);

    write_segment_to_file(file);
}

void close_file(LFile *lfile) {
    free(lfile);
}
