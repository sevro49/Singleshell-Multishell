#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

#define MAX_SHELL 10
#define DEFAULT_NSHELL 2
char *addr = NULL; /*mmap addres*/
int fd = -1;       /*fd for shared file object*/

int initmem() {
    fd = shm_open(MY_SHARED_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0) {
        perror("multishell.c:open file:");
        exit(1);
    }
/*     if (ftruncate(fd, 1024) == -1) {
        perror("ftruncate");
        exit(1);
    } */

    addr = mmap(NULL, MY_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL) {
        perror("mmap:");
        exit(1);
    }
    return 0;
}

/**
 * todo, you can create multiple function, variables, etc..
 * */
int main(int argc, char **argv) {
    initmem();
    pid_t pid[MAX_SHELL];
    int count = atoi(argv[1]); /* it is the number of 
    shells you want to open*/
    if(count > 10)
        count = 10;
    
    if(argc == 2){
        for (int i = 0; i < count; i++) {
            pid[i] = fork();
            if (pid[i] == -1) {
                perror("fork");
                exit(1);
            } else if (pid[i] == 0) {/*creates singleshell processes with xterm  */
                char *args[] = {"xterm", "./singleshell", NULL};
                if (execvp(args[0], args) == -1) {
                    perror("execvp");
                    exit(1);
                }
            }
        }
    } else {
        perror("argc");
        exit(1);
    }

    int status;

    for (int i = 0; i < count; i++) {
        int rd = waitpid(pid[i], &status, 0);
    }

    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    int year = timeinfo->tm_year + 1900;
    int month = timeinfo->tm_mon + 1;
    int day = timeinfo->tm_mday;
    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;

    char filename[40];
    /*names the shelllog file*/
    sprintf(filename, "shelllog-%.2d-%.2d-%d_%.2d-%.2d.txt", day, month, year, hour, minute);
    /*opens shelllog file*/
    int fd1 = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd1 < 0) {
        perror("open");
        exit(1);
    }

    struct stat fd_stat;
    if(fstat(fd, &fd_stat) == -1) {
        perror("fstat");
    }
    int len = fd_stat.st_size < 1024 ? fd_stat.st_size : MY_FILE_SIZE;

    if (write(fd1, addr, len) < 0) {/*saving data into the shelllog file */
        perror("write");
        exit(1);
    }

    close(fd1);


    /*unlink mmap*/
    munmap(addr, 1024);
    /* close the shared memory file*/
    close(fd);

    return 0;
}
