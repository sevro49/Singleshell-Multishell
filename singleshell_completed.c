/**
 * header 
 * @author Emirhan Yıldırım
 * @brief added run function and writing dates
 * to sharedlogfile on start and exit
 * @author Emre Güler
 * @brief added process id and parent process id
 * when program runs. added shell commands to
 * sharedlogfile when you use command
 * @author Mustafa Emir Akgün
 * @brief added singleshell process creation with xterm 
 * in multishell and added data saving to the shelllog file
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

#define INBUF_SIZE 256

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

int run(char *);
int printProcessId();
int printParentProcessId();

char *addr = NULL;
int fd = -1;

int initmem() {
    fd = shm_open(MY_SHARED_FILE_NAME, O_RDWR | O_APPEND | O_CREAT , S_IRUSR | S_IWUSR);
    if (fd < 0){
        perror("singleshell.c:fd:line31");
        exit(1);
    }

    addr = mmap(NULL, MY_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL){
        perror("singleshell.c:mmap:");
        close(fd);
        exit(1);
    }

    return 0;
}
/**
 * todo: you can start with myshellv3.c from lecture notes
 */
int main(int argc, char *argv[]) {
    time_t timer = time(NULL);
    int nbyte;
    char command[256], *currenttime = ctime(&timer);
    pid_t pid = getpid();
    pid_t ppid = getppid();

    initmem();

    if(write(fd, currenttime, strlen(currenttime)) == -1) {
        perror("write");
        exit(0);
    }
    printProcessId();
    printParentProcessId();


    while(1) {
        printf("$");
        fflush(stdout);

        if((nbyte = read(0, command, 256)) <= 0)
            perror("Read error");
        else
            command[nbyte-1] = '\0';

        if(strncmp(command, "exit", 4) == 0) {
            printProcessId();
            if(write(fd, command, strlen(command)) == -1) { /* adding commands to addr */
                perror("write");
                exit(0);
            }
            
            timer = time(NULL);
            currenttime = ctime(&timer);
            if(write(fd, "\n", 1) == -1) {
                perror("write");
                exit(0);
            }
            if(write(fd, currenttime, strlen(currenttime)) == -1) {
                perror("write");
                exit(0);
            }
            exit(1);
        }
        
        run(command);
    }

    /* Unmap the shared memory */
    munmap(addr, 1024);
    /* Close the shared memory file */
    close(fd);

    return 0;
}

/** 
 * Runs the given command.
 * 
 * @param command: Command to be executed
 */
int run(char *command) {
    int i, child;
    char *args[256];

    if(strcmp(command, "addr") == 0) {
        printf("addr = %s\n", addr);
        return 1;
    }


    /* Execute command */
    child = fork();
    if (child == 0) { /* Child Process */
        printProcessId();
        if(write(fd, command, strlen(command)) == -1) { /* adding commands to addr */
            perror("write");
            exit(0);
        }
        if(write(fd, "\n", 1) == -1) {
            perror("write");
            exit(0);
        }

        /* Tokenize */
        if ((args[0] = strtok(command, " ")) == NULL)
            return 0;
        i = 1;
        while ((args[i] = strtok(NULL, " ")) != NULL) {
            i++;
        }

        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }
    else if (child > 0) { /* Parent Process */
        wait(NULL);
    }
    else {
        perror("fork");
        exit(1);
    }
}

/**
 * Prints Process ID to fd
 */
int printProcessId(){
    pid_t pid = getpid();
    dprintf(fd, "Process ID: %d | ", pid);
    return 0;
}

/**
 * Prints Parent Process ID to fd
 */
int printParentProcessId(){
    pid_t ppid = getppid(); 
    dprintf(fd, "Parent Process ID: %d\n", ppid);
    return 0;
}
