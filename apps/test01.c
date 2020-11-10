#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "File.h"

/* Test cases:
 a. reading and writing of files in the root directory
 b. creation of sub-directories
 c. reading and writing of files in any directory
 d. deletion of files and directories
 */

int main(){
//a. reading and writing of files in the root directory
    printf("Starting to test reading and writing of files in the root directory.\n");
	
    INITLLFS();
    LFile * file;
    
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
    
    printf("Test reading of files in the root directory is done.\n\n");

//b. creation of sub-directories
    printf("Starting to test creation of sub-directories.\n");
    
    if (mkdir("/one/two") == 0){
        printf("test: Logic of invaild create multi levels of dirs failed.\n");
        return -1;
    } else {
        printf("test: Logic of invaild create multi levels of dirs works fine.\n");
    }
    
    if (mkdir("/one") != 0) {
        printf("test: Creation of second level sub-directories failed.");
        return -1;
    } else {
        printf("test: Creation of second level sub-directories successfully.\n");
    }
    
    if (mkdir("/one/two") != 0) {
        printf("test: Creation of third level sub-directories failed.\n");
        return -1;
    } else {
        printf("test: Creation of third level sub-directories successfully.\n");
    }

    if (mkdir("/one") == 0) {
        printf("test: Exist directory should not be created again.");
        return -1;
    } else {
        printf("test: Skip the exist directory.\n");
    }

    if (mkdir("/one/two") == 0) {
        printf("test: Exist directory should not be created again.\n");
        return -1;
    } else {
        printf("test: Skip the exist directory.\n");
    }
    
    printf("Test creation of sub-directories is done.\n\n");

    
// c. reading and writing of files in any directory
    
    printf("Starting to reading and writing of files in sub directory.\n");
    
    file = open_file("/one/two/b.txt", "w");
    if (file == NULL){
        printf("test: open_file /one/two/b.txt in writing mode failed.\n");
        return -1;
    } else {
        printf("test: open_file /one/two/b.txt works fine.\n");
    }
    
    printf("Test writing of files in sub directory is done.\n");
    
    buffer = malloc(1024);
    printf("test: buffer allcoate successfully. \n");
    
    for (int i = 0; i < 100; i++){
        strcpy(buffer + i*5,  "56789");
    }
    printf("test: buffer assign successfully. \n");
    
    write_file(buffer, 5, 100, file);
    printf("test: write_file works fine\n");
    
    buffer2 = malloc(1024);
    memcpy(buffer2, buffer, 1024);
    
    read_file(buffer,1024,1,file);
    if (strcmp(buffer, buffer2) != 0){
        printf("test: read_file failed.\n");
        return -1;
    } else {
        printf("test: read_file works fine\n");
        printf("file /one/two/b.txt content is: %s\n",buffer);
    }
    
    close_file(file);
    printf("test: close_file works fine\n");
    
    printf("Test reading of files in sub directory is done.\n\n");
    
//d. deletion of files and directories
     printf("Test for deletion of files.\n");
    
    if (rm_file("/one/two/b.txt") == false){
        printf("test: Deletion of file failed.\n");
        return -1;
    } else {
        printf("test: Deletion of file successfully.\n");
    }
    
    printf("Test for deletion of files is done.\n\n");
    
    
    printf("Test for deletion of directories.\n");
    
    if (rm_dir("/one/two/three") == true){
        printf("test: Deletion of non-exist directory failed. \n");
        return -1;
    } else {
        printf("test: Deletion of non-exist directory fine.\n");
    }
    
    if (rm_dir("/one/two") == false){
        printf("test: Deletion of sub directory failed. \n");
        return -1;
    } else {
        printf("test: Deletion of sub directory successfully.\n");
    }
    
    printf("Test for deletion of directories is done.\n");
    
}
