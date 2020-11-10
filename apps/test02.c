#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "File.h"


int main(){
    printf("Starting to created virtual disks.\n");
    
    INITLLFS();
    printf("test: INITLLFS successfully \n");
    
    int a = load_disk();
    if (a == -1) {
        printf("test: Disk are failed mounted.\n");
        return -1;
    }
    printf("test: Disk are successfully mounted.\n");
    
    printf("vdisk setting done.\n");
    
}
