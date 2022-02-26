/*
Jacob Drew
4760 Project 2
slave.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define BUF_SIZE 1024
#define SHM_KEY 0x5050

struct shmseg
{
    int cnt;
    int complete;
    char buf[BUF_SIZE];
};

int main(int argc, char *argv[])
{

    FILE *outputFile;
    char* logName = "logfile.txt";
    outputFile = freopen(logName, "w+", stdout);
    //define memory segment and pointer to message struct
    int shmid;
    struct shmseg *shmp;
    shmid = shmget(SHM_KEY, 1024, 0777 | IPC_CREAT);
    if (shmid == -1)    //if creating shared memory is unsuccessful throw error
    {
        perror("Shared memory");
        return 1;
    }

    // Attach to the segment to get a pointer to it.
    shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *)-1)
    {
        perror("Shared memory attach");
        return 1;
    }


    //detatch the shared memory
    printf("Reading Process: Reading Done, Detaching Shared Memory\n");
    if (shmdt(shmp) == -1)
    {
        perror("shmdt");
        return 1;
    }

    
    printf("hello");
    fclose(outputFile);
    return 0;
}