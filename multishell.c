/**
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

#define MAX_SHELL 10
#define DEFAULT_NSHELL 2
char *addr = NULL; /*mmap addres*/
int fd = -1;       /*fd for shared file object*/

int initmem()
{
    fd = shm_open(MY_SHARED_FILE_NAME,
                  O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0){
        perror("multishell.c:open file:");
        exit(1);
    }
    if (ftruncate(fd, 1024) == -1){
        perror("ftruncate");
        exit(1);
    }

    addr = mmap(NULL, MY_FILE_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL){
        perror("mmap:");
        exit(1);
    }
    return 0;
}

/**
 * todo, you can create multiple function, variables, etc..
 * */
int main(int argc, char **argv)
{

    initmem();

    /*unlink mmap*/
    munmap(addr, 1024);
    /* close the shared memory file*/
    close(fd);

    return 0;
}
