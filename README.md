# Project Title
The Little Log File System

## Design
A little loose version log file system.

### Structures

- Block: 
    The minimum operation chunk in memory, each of which is 512 bytes in size. The maximum number of blocks on vdisk is 4096.
- Segment: 
    The minimum writing operation chunk in Log part on vdisk, each of which contains 16 blocks. Maximum 255 user accessible segments can fit in vdisk as the first segment is reserved.
- Inode: 
    Container of metadata of file, each of which will fit into one block to make life easier. The maximum of inodes is 256, since the inode id is represented by a unsigned 8-bit integer.

- Vdisk: a 2MB virtual disk
    - Block 0: SuperBlock
    - Block 1: Bitmap of Block, to track free and used blocks
    - Block 2: Inode Map, to track inode number and corresponding block number
    ... Reserved blocks number 3-15 ...
    - Block 16: Initially the Inode of Root directory whose Inode Id is set to 1
    ... growing Log part

```
| SuperBlock | Bitmap of Block | Inode Map | ... 13 blocks | Segment 0 | Segments 1 | ...max up to 254 |
|   Block 0  |     Block 1     |  Block 2  |Reserved Blocks|               Growing Log ...             |
```
### Main functionality & policies
- Initialize vdisk
    - **INITLLFS()** creates a virtual disk which is saved in file **/disk/vdisk**, and initializes the first reserved segment with the initial value.
- Mount vdisk
    - **load_disk()** loads the information from existent **/disk/vdisk** file, including inode and block information.
- Open file / create file
    - **LFile \*open_file(char \*pathname, char \*model)** opens a file for read or write.
    - Write mode "w", opens a exist file for write, or create the file if it's not exist.
    - Read mode "r", opens a exist file for read.
    - Both operations will print out error if the directory information is incorrect.
    - The file need to be opened first then do the following operations.
- Write file
    - **void write_file(void \*ptr, size_t size, size_t nmemb, LFile \*lfile)** writes **size * nmemb** byte(s) from buffer **ptr** to file **lfile** opened by **open_file**.
- Read file
    - **size_t read_file(void \*ptr, size_t size, size_t nmemb, LFile \*lfile)** reads up to the **size * nmemb** byte(s) to the buffer **ptr** from file **lfile** opened by **open_file**, the actual number of bytes will be returned.
- Delete file
    - **bool rm_file(char \*pathname)** deletes the file, all the relative information and structures on the vdisk will also be released.
- Make Directory
    - **int mkdir(char \*pathname)** creates the specific directory, it will only succeed when all the ancestor directories exist, otherwise reject and print error message.
- Delete directory 
    - **bool rm_dir(char \*pathname)** deletes the directory only if it is empty, which means no files or sub-directories in this directory, otherwise reject and print error message.
- When to write memory to vdisk (consistency)
    - Right after vdisk initialization
    - When segment buffer is full, write whole segment buffer to vdisk.
    - Each time when an operation listed above is finished, the data will be flushed to the disk.
- When to free outdated blocks
    - When data on the block needs to be update, the new data blocks will be occupied as well as the old ones will be freed.
    - When the file or directory is deleted, both of the data block and the inode block will be freed.
- When do Fsck 
    - When the file **/disk/vdisk** is load from the disk, the following two checks are performed.
        - if the specific block is occupied more than indicated times, meaning zero time if the block bitmap shows it's vacant, or once if occupied.
        - if the specific inode does not connect to the root directory, or has more than two parent directories.
- Other assumptions
    - File or directory is less than 10 blocks in size.
    - Append content to an existing file is not allowed.

### Files in ZIP
- struct.h: some typedef structures, i.e SuperBlock, Inode, Bitmap, Directory Entry etc.
- File.c: Main implementation of LLFS, i.e INITLLFS, open file, read file, mkdir, write file etc.
- File.h: Definition of the signatures of the functions.
- block.c: block related functions, i.e bitmap of blocks, create blocks etc.
- block.h: Definition of the signatures of the functions
- inode.c: inode related functions, i.e inode map and create inodes etc.
- inode.h: Definition of the signatures of the functions
- segment.c: segment related functions, i.e segment map and write segment to vdisk etc.
- segment.h: Definition of the signatures of the functions
- test01.c: Testing code for all the operation on files and directories.
- test02.c: Testing code for creation and initialization of virtual disk.
- task.sh: bash script to run the commands
- Makefile
- README

## Running the tests 
```
$ make clean
$ make test02
$ ./test02
$ make test01
$ ./test01
```
test02: aim to test two functionalities of vdisk.
    
    
```
INITLLFS();
printf("test: INITLLFS successfully \n");
    
int a = load_disk();
if (a == -1) {
   printf("test: Disk are failed mounted.\n");
   return -1;
}
printf("test: Disk are successfully mounted.\n");
```
test01: aim to test open/cretate/write/read files and make/remove directory or subdirectory. 
```
file = open_file("/a.txt", "r");
    printf("test: open_file works fine\n");
    
    file = open_file("/a.txt", "w");
    if (file == NULL){
        printf("test: open_file /a.txt in writing mode failed.\n");
        return -1;
    } else {
        printf("test: open_file /a.txt works fine.\n");
    }
    
    
    printf("Test writing of files in the root directory is done.\n");

    char* buffer = malloc(1024);
    printf("test: buffer allcoate successfully. \n");

    for (int i = 0; i < 100; i++){
        strcpy(buffer + i*5,  "12345");
    }
    printf("test: buffer assign successfully. \n");
    
    write_file(buffer, 5, 100, file);
    printf("test: write_file works fine\n");
    
    char* buffer2 = malloc(1024);
    memcpy(buffer2, buffer, 1024);
    
    read_file(buffer,1024,1,file);
    if (strcmp(buffer, buffer2) != 0){
        printf("test: read_file failed.\n");
        return -1;
    } else {
        printf("test: read_file works fine\n");
        printf("file a.txt content is: %s\n",buffer);
    }
    
    close_file(file);
    printf("test: close_file works fine\n");

```